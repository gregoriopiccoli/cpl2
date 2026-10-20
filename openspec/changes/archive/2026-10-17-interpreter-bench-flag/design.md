# Design

## Context

Vedi `proposal.md` per la motivazione. Qui i fatti del codice che condizionano l'approccio.

Il perimetro da cambiare e' tutto in `main()` e in `benchPcd()`:

```cpp
int main(int argc, char** argv){
  if (argc!=2){
    cerr << "uso: interpreter <file.pcd>" << endl;
    cerr << "  un nome senza estensione viene completato con .pcd" << endl;
    return 1;
  }
  int rc=runPcd(pcodePathFor(argv[1]));
  cout << "--- status on exit ---\n";
  stdGC().status();
  ...
  return rc;
}
```

`benchPcd()` esiste gia' ed e' gia' la funzione giusta, ma restituisce la costante 0 e
ignora il valore di ritorno di `runPcd()`:

```cpp
int benchPcd(const string& fn){
  ankerl::nanobench::Bench().run("esegue "+fn, [&] {
    runPcd(fn);
  });
  return 0;
}
```

Fatti che rendono l'implementazione piccola:

- `pcodePathFor()` (D2 della change precedente) e' gia' pronto e non va toccato: e' la
  derivazione "nessuna estensione nel nome di base -> accodo `.pcd`", valida in entrambe le
  modalita'.
- `pcodeProgram::loadPcd()` restituisce gia' `1` se il p-code e' caricabile e `0` altrimenti,
  e stampa gia' su stderr il messaggio di ogni motivo di fallimento (`not a compiled cpl2
  file ...`, `<fn> has errors!`, e il ramo del file non apribile aggiunto da D4). Non serve
  alcuna diagnostica nuova.
- `runPcd()` e' gia' `int` e gia' restituisce `0`/`1` secondo l'esito del caricamento (D3
  della change precedente): il codice di uscita della modalita' normale e' gia' corretto.
- `pcodeProgram` ha distruttore (`~pcodeProgram(){clear();}`) che libera i `pcode*` caricati:
  un oggetto locale usato e abbandonato non lascia niente indietro.
- `interp::interp()` inizializza `sp=-1`, quindi `assert(intp.sp==-1)` in `runPcd()` resta
  soddisfatto anche quando il caricamento fallisce e `intp.run()` non viene chiamata. Il
  probe di D3 puo' quindi riusare quella forma senza toccare `runPcd()`.

## Goals / Non-Goals

**Goals:**
- `main()` distingue due sole forme valide: `<percorso>` e `-bench <percorso>`; tutto il
  resto e' il ramo d'uso esistente.
- Il flag e' `-bench` esatto, in prima posizione. Nessun altro nome con `-` diventa opzione.
- In modalita' bench il codice di uscita dipende dal caricamento, come in quella normale,
  e un file non caricabile non produce una tabella di misura.
- Il corpo di `runPcd()`, di `pcodePathFor()` e di `loadPcd()` non cambia.

**Non-Goals:**
- Nessuna modifica alla logica di misura: stessa `nanobench::Bench()`, stesse iterazioni,
  stessa etichetta `esegue <file>`, nessuna taratura.
- Nessuna nuova diagnostica e nessun nuovo testo d'errore oltre la riga `-bench` nell'uso.
- Nessuna modifica al rapporto del GC: resta stampato su stdout dopo l'esecuzione, in
  entrambe le modalita'.
- Non si tocca `compiler`, `./asm`, la grammatica, il formato del `.pcd` ne' l'header
  `#cpl20.0.1`: nessuna rigenerazione di `cpl2.c`/`cpl2.h`.
- Non si rendono raggiungibili dalla command line i bench in C++ (`bench_cc()`,
  `bench_fib_cc()`, `test_cc()`, `test_fib_cc()`): non hanno un p-code da indicare e
  accodarli al flag significherebbe inventare un secondo contratto di selezione.

## Decisions

### D1. `-bench` e' una stringa esatta, confrontata solo in prima posizione
Il riconoscimento e' `strcmp(argv[1], "-bench")==0` con `argc==3`. Nessuna ricerca del flag
fra gli argomenti, nessun prefisso, nessun doppio trattino.

Perche' confronto esatto e non "inizia con": e' l'unico nome che il progetto deve ricordare
e non ci sono altri candidati (`-b` e `--bench` restano percorsi). Un confronto per prefisso
farebbe diventare opzione qualunque file il cui nome inizi con quel testo, cioe' romperebbe
la regola documentata in `interpreter-cli` per un vantaggio che qui non esiste.

Perche' solo in prima posizione e non ovunque: una sola forma canonica da scrivere e da
ricordare, e la validazione degli argomenti resta una verifica di forma (`argc` atteso +
posizione fissa) invece che un conteggio di elementi residui. Il costo e' che
`./interpreter fib.pcd -bench` e' un errore invece di una forma equivalente: accettato,
perche' l'errore e' esplicito e nessuno scenario della spec promette quella forma.

Alternativa scartata: cercare `-bench` in qualunque posizione e richiedere che resti un solo
percorso. Piu' permissiva, ma rende "accetta un solo argomento" una frase falsa per il
contesto e aggiunge un caso al messaggio d'uso. Se un domani servisse, e' una modifica
piccola e locale a `main()`.

### D2. `main()`：due forme valide, il resto nel ramo d'uso di oggi
La selezione avviene prima di qualunque altra cosa, cosi' il resto di `main()` resta identico:

- `argc==3` e `argv[1]=="-bench"` -> `benchPcd(pcodePathFor(argv[2]))`;
- `argc==2` e `argv[1]!="-bench"` -> `runPcd(pcodePathFor(argv[1]))`;
- ogni altro `argc` (0, 1, 2 con `-bench`, 3 senza flag, 4) -> le due righe d'uso su stderr e
  `return 1`.

Perche' la derivazione resti in `main()` e non dentro `benchPcd()`: `benchPcd()` chiama
`runPcd()`, che non deriva niente e prende un percorso gia' pronto; derivare una volta sola
evita che i due punti possano divergere e fa si' che l'etichetta della tabella mostri il
percorso realmente caricato (`fib.pcd`), non quello digitato (`fib`).

Nota sul caso `argc==2` con `argv[1]=="-bench"`: non e' un ramo dedicato, e' la caduta
nella terza regola. Il risultato richiesto (uso su stderr, codice 1, niente misurato)
arriva senza codice aggiuntivo, e senza una riga di logica che dica "manca il percorso".

### D3. `benchPcd()` verifica il caricamento una volta e poi misura
`benchPcd()` smette di restituire la costante 0: prima di entrare in nanobench carica il file
in un `pcodeProgram` locale e, se il caricamento fallisce, restituisce 1 senza misurare. Se
riesce, chiama `runPcd()` come oggi e restituisce 0.

Perche' un probe e non catturare il ritorno di `runPcd()` dentro la lambda (`rc=runPcd(fn)`
nella lambda, `return rc` dopo): il ritorno di `runPcd()` dice "caricabile", ma il ciclo
nanobench continuerebbe a girare su un programma che non esiste, stampando una tabella di
tempi senza significato (`~3 ns per iterazione`) con un codice di uscita 1. Uno scenario della
spec richiede esattamente l'opposto: nessuna tabella. Il probe costa un caricamento in piu'
sulla strada felice, un `loadPcd()` su un file di poche decine di righe, contro N esecuzioni
del programma: trascurabile rispetto a 30-60s di misura.

Perche' il probe dentro `benchPcd()` e non in `main()`: la domanda "c'e' qualcosa da
misurare" appartiene a chi misura. Se stesse in `main()`, `benchPcd()` continuerebbe a
restituire 0 e `main()` dovrebbe sapere della misura per non richiamarla al ritorno — due
punti che devono restare allineati sul risultato.

La diagnostica non viene duplicata: `loadPcd()` stampa il motivo del fallimento e restituisce
0, il probe restituisce 1 subito, quindi `runPcd()` non viene mai chiamata su un file non
caricabile e il messaggio compare una volta sola. Non serve un parametro "silenzioso" in
`loadPcd()`.

Alternativa scartata: lasciare `benchPcd()` intatta e fare il probe in `main()`. Sposta una
riga e lascia la funzione che misura a mentire sul codice di uscita.

### D4. Il testo d'uso nomina il flag, senza diventare un manuale
Le due righe attuali diventano tre: `uso: interpreter [-bench] <file.pcd>`, la riga che spiega
che `-bench` misura invece di eseguire, e la riga sulla derivazione `.pcd`. La spec fissa solo
che l'uso va su stderr e che il codice e' 1, quindi il testo esatto resta libero.

Nota: il messaggio di "accetta un solo argomento" resta quello di oggi anche quando l'errore
e' un `-bench` senza percorso o un `-bench` troppo indietro. Il testo resta vero (l'unico
argomento ammesso come percorso e' uno) e cambiare i messaggi per caso richiederebbe una
tabella di testi che non serve a niente: chi ha sbagliato la forma riceve comunque l'uso.

### D5. Il rapporto del GC e la tabella restano su stdout, in entrambe le modalita'
Nessun ramo di `main()` cambia: il rapporto del GC viene stampato dopo che `benchPcd()` o
`runPcd()` hanno restituito, e `return rc` restituisce il codice di uscita della modalita'.

Perche' non sopprimere il rapporto del GC in modalita' bench: e' strumentazione di sviluppo
esistente, stampata anche oggi quando `main()` chiama `bench()`, e toglierla a una sola
modalita' renderebbe il confronto fra una esecuzione e la sua misura piu' scomodo, non meno.
Un eventuale intervento sul rapporto del GC resta una modifica separata, valida per tutte le
modalita'.

## Risks / Trade-offs

- **Il file di nome `-bench` (o `-bench.pcd`) non e' piu' caricabile.** La change precedente
  aveva fissato in D6 che "un nome che inizia con `-` e' un percorso" e l'aveva scritto come
  scenario di spec; questa change lo modifica per un nome solo.
  → Accettato: il file `-bench` e' un nome improbabile, e il caso in cui diventa reale si
  riconosce subito (uso su stderr invece di "non posso aprire `-bench.pcd`") ed e' risolvibile
  con `./interpreter ./-bench`. Il testo d'uso di D4 mostra la forma corretta.

- **`./interpreter -bench fib.pcd` dura 30-60s e sembra un'interruzione.** Il bench di
  `fib(34)` e' gia' lento quando si misura a mano.
  → Accettato, e' il costo della misura. Nessun timeout ne' indicatore di attesa: la tabella
  di nanobench arriva alla fine come oggi.

- **`nanobench` stampa su stdout cosi' come il programma misurato.** In una misura lunga le
  righe del programma e quelle dei tempi vanno in un unico flusso.
  → Fuori perimetro per scelta (D5): e' il comportamento di oggi di `bench()`, che mescola
  gia' output e tempi. Se un domani desse fastidio, si puo' separare senza toccare gli scenari
  di questa spec, che riguardano solo la destinazione della tabella e delle diagnostiche.

- **Un programma che a runtime lancia `domain_error` (`not impl!`) fa abortire il processo
  anche in modalita' bench**, lasciando la tabella incompleta. Vale per 59 dei 105 p-code non
  implementati, quindi e' una probabilita' alta, non un'eccezione.
  → Non introdotto qui: `bench()` ha lo stesso comportamento da quando i bench sono hardcoded.
  Se un domani si vuole distinguere, e' la normalizzazione del fallimento a runtime, che resta
  fuori perimetro anche per la modalita' normale.

- **`assert(intp.sp==-1)` e' attivo** (`-g -O2` senza `-DNDEBUG`): in modalita' bench un
  programma che non ripulisce lo stack termina con `SIGABRT` alla prima iterazione, e con
  esso il bench, non solo l'esecuzione.
  → Stessa condizione della modalita' normale, gia' annotata in `AGENTS.md`. Nessuna modifica:
  il flag non cambia quando scatta.

- **Il probe carica il file una volta in piu' rispetto a `runPcd()`.** Doppio parsing del
  `.pcd` prima di misurare.
  → Irrilevante rispetto al tempo di esecuzione del programma, e D3 spiega perche' il probe
  serve.

## Migration Plan

Nessuna migrazione di dati ne' di formato: il `.pcd` prodotto dal compilatore e' gia' quello
che `loadPcd()` sa leggere e la sua struttura non cambia.

Cambia solo come si sceglie la modalita' dell'interprete:

- `./interpreter fib.pcd` continua a eseguire una volta, come prima della change;
- `./interpreter -bench fib.pcd` misura, e prima della change equivaleva a modificare
  `main()` e rilanciare `make`;
- le invocazioni che prima erano errori e restano errori: `./interpreter`,
  `./interpreter a.pcd b.pcd`, `./interpreter -h` (che continua a tentare di aprire il
  file `-h`);
- l'unica forma che cambia categoria e' `./interpreter -bench`, che prima era "non posso
  aprire `-bench.pcd`" e ora e' istruzioni d'uso con codice 1.

Rollback: ripristinare `interpreter.cpp` e `AGENTS.md` (e `todo.md`, se il task di
allineamento e' stato fatto). Nessun dato persistito e' coinvolto, nessun formato cambia.
Se la change non e' ancora stata archiviata, il rollback completo e' anche la rinuncia alla
change stessa, perche' la specifica di `interpreter-cli` continua a descrivere lo stato
precedente finche' il delta non viene sincronizzato.

## Open Questions

- **nanobench mostra una sola riga finale o i dettagli delle iterazioni?** Il bench di
  `fib(34)` dura 30-60s e la tabella attuale mostra tutte le epoch. Ridurla, o usare un
  `->title()` piu' esplicito, si decide nella configurazione della singola chiamata e non
  cambia nessuno scenario di questa spec.
- **L'etichetta della tabella resta `esegue <percorso>`?** Mostra gia' il percorso derivato,
  che e' quello giusto; distinguere `misura <percorso>` dal `run` normale e' una questione di
  testo, non di contratto.
- **`todo.md` deve essere riallineato?** Porta ancora al punto 2 "specificare i file da
  eseguire in cli", che la change `interpreter-cli-input-file` ha gia' chiuso. Se il riallineamento
  e' messo in questa change come task, resta una domanda se chiudere anche il punto 3 (che il
  file non elenca affatto) oppure limitarsi a rimuovere il 2 ormai fatto: e' una decisione
  dell'autore del file, non un vincolo tecnico, e non tocca gli artifact.