# Design

## Context

Vedi `proposal.md` per la motivazione. Qui i fatti del codice che condizionano l'approccio.

Il progetto e' piatto e senza dipendenze: `parser.c` e' l'unica unita' di traduzione del
compilatore e `#include` in sequenza `pcodes.c`, `emit.c`, `cpl2.c`; `interpreter.cpp`
`#include` `gc.cpp`, `pcodes.h`/`pcodes.c` e `nanobench.h`. Nessun file `.c` viene compilato
da solo e il Makefile non ha un passo di link separato. Questo pattern e' gia' il modo del
progetto di condividere codice fra i due binari, e `cli.c` lo segue.

I due `main()` attuali, abbreviati:

```c
// parser.c
int main(int argc, char** argv){
  if (argc<2) { srcFile=NULL; emit_out=stdout; return parse(stdin); }
  int errors=0;
  for (int i=1;i<argc;i++) {
    // ... ogni argv[i] e' un sorgente, nessuna opzione
  }
  return errors;
}
```

```cpp
// interpreter.cpp
int main(int argc, char** argv){
  const char* BENCH="-bench";
  bool bench=false; string path;
  if (argc==3 && strcmp(argv[1],BENCH)==0){ bench=true; path=argv[2]; }
  else if (argc==2 && strcmp(argv[1],BENCH)!=0){ path=argv[1]; }
  else { cerr << "uso: ..." << endl; ...; return 1; }
  int rc=bench?benchPcd(pcodePathFor(path)):runPcd(pcodePathFor(path));
  ...
}
```

Il vincolo tecnico che pesa di piu' sulla scelta dell'interfaccia: **`interpreter.cpp` e' C++,
`parser.c` e' C.** Un modulo condiviso deve quindi essere valido nei due linguaggi senza
condizionali: niente conversioni implicite da `void*`, niente `malloc` restituito come altro
tipo, niente parole chiave che cambiano significato. Le stringhe che attraversano il confine
restano `const char*`; ogni struttura che contiene puntatori viene allocata dallo chiamante e
passata per puntatore, cosi' il modulo non deve mai chiamare `new` o `free`.

Il terzo confine: `parser.c` e `interpreter.cpp` non sono collegati fra loro, quindi un modulo
"condiviso" non puo' condividere dati, solo funzioni. Ogni strumento deve avere la sua tabella
di opzioni.

`gc_test.cpp` gia' esiste come file di verifica a se' stante con il proprio `main()`, non
citato dal Makefile, costruito a mano con `c++ -g -O2 gc_test.cpp -o gc_test && ./gc_test`.
`cli_test.c` replica quel pattern, non un'infrastruttura nuova.

## Goals / Non-Goals

**Goals:**
- Un solo ciclo di scansione degli argomenti, in `cli.c`, usato da entrambi i binari.
- L'interfaccia coperta da C e C++, e senza alcuna dipendenza esterna.
- Il vocabolario delle opzioni (quali nomi esistono, cosa fanno) resta di ciascuno strumento,
  in una tabella dichiarata li' dove viene usata.
- Le tre forme servite dal progetto sono distinte dal tipo dichiarato nella tabella: con nome
  lungo, con nome corto, con valore.
- Il testo delle istruzioni d'uso non viene scritto due volte: l'elenco delle opzioni arriva
  dalla stessa tabella che la scansione usa.

**Non-Goals:**
- Nessuna dipendenza nuova. In particolare `getopt_long` resta scartato (D1): e' POSIX, la
  riordinazione delle opzioni (`permute`) e' un'estensione GNU, e sostituirebbe con una
  libreria di sistema un ciclo di poche righe che il progetto puo' scrivere e leggere da solo.
  Perde anche il vantaggio di tenere il comportamento degli argomenti nel sorgente, dove vale
  la convenzione del progetto di commentare in italiano.
- Nessuna opzione nuova al compilatore. La sua tabella parte vuota (D6).
- Non si aggiunge `-h` / `--help`: `-h` resta un'opzione sconosciuta (D7).
- Non si toccano la logica di misura, il rapporto del GC, la derivazione del percorso, la
  risoluzione implicita di `.cpl`, il formato del `.pcd`, la grammatica ne' `asm`.
- Non si correggono i due difetti gia' segnalati in `proposal.md` (codice di uscita invertito
  in `-bench`, corruzione dell'heap in `-bench`): sono runtime, non sintassi degli argomenti.

## Decisions

### D1. `cli.c` incluso nelle due unita' di traduzione, non linkato

`cli.c` si include con `#include "cli.c"` in `parser.c` accanto a `emit.c`, e in
`interpreter.cpp` accanto a `gc.cpp`.

Perche' e non un vero oggetto linkato: il progetto non ha un passo di link separato e non ha
mai avuto codice condiviso linkato fra i due binari. Un `cli.o` richiederebbe `-I.` e un
`-c` piu' link esplicito nelle due regole del Makefile, piu' una riga per ogni utente futuro
del modulo. Il `#include` costa zero modifiche alla build ed e' coerente con `pcodes.c`, che e'
gia' incluso in entrambe le unita'.

Alternativa scartata: `getopt_long`. Vedi i Non-Goals.

### D2. L'interfaccia separa i posizionali dalle opzioni e non conosce il vocabolario

`cli.c` espone una tabella e un ciclo; la tabella la scrive ogni strumento.

```c
// come un'opzione e' dichiarata
typedef struct {
  const char* lungo;   // "bench", NULL se l'opzione e' solo corta
  char        corto;   // 'b', 0 se l'opzione e' solo lunga
  int         tipo;    // CLI_OPZIONE oppure CLI_VALORE
  void*       dest;    // int* per un flag, const char** per un valore
} cliOpzione;

// stato della scansione, inizializzato dal chiamante
typedef struct {
  int argc;
  char** argv;
  int indice;                  // argv[indice] e' il prossimo da guardare
  int nOpzioni;                // opzioni riconosciute fin qui
  int nPosizionali;            // posizionali raccolti fin qui
  const char* sconosciutaNome; // NULL se non ce ne sono
  const char* sconosciutaValore;
  int         sconosciutaVuolValore;
} cliScansione;
```

L'array dei posizionali e la tabella li fornisce il chiamante, perche' il modulo non deve
allocare: gli argomenti sono gia' in memoria e basta copiare i puntatori.

La separazione e' la ragione per cui il modulo resta utile a due strumenti che non hanno
opzioni in comune: `cli.c` sa cos'e' un'opzione con valore, non cosa sia `-bench`.

Alternativa scartata: un vocabolario unico di flag cpl2 condiviso. Sarebbe piu' breve da
scrivere, ma accoppierebbe i due binari: un flag che ha senso solo per il compilatore
finirebbe nella tabella dell'interprete, e i due elenchi ricomincerebbero a divergere come
oggi.

### D3. Le opzioni si riconoscono ovunque, `--` le termina

Il ciclo, in pseudo-codice:

```
i = 1
mentre i < argc:
    a = argv[i]
    se a == "--":        appendere argv[i+1 .. argc-1] come posizionali; i = argc; continua
    se a[0] == '-' e a[1] != '\0':
        cerca in tabella:
            lungo = confronto esatto con a+2, se a inizia con "--"
            corto = a[1],                    se a inizia con un solo "-"
        se non trovata:  annotare come sconosciuta; i++; continua
        se trovata e vuole valore:
            se c'e' un '=' nel token:  valore = tutto dopo '='
            altrimenti:               valore = argv[i+1]; i++
        *dest = valore, oppure *dest = 1 per un flag
    altrimenti:          appendere a come posizionale
    i++
```

Tre scelte dentro il ciclo.

**Perche' ovunque e non solo in prima posizione.** Riconoscere un'opzione in coda alla riga
costa una condizione, non un ramo separato. La forma che ne risulta ("prima separare, poi
validare") non e' implementabile con un controllo di `argc`, quindi con piu' di un flag
l'interprete dovrebbe comunque passare a una scansione. Tenere la posizione fissa avrebbe
significato un ciclo che si rifiuta di guardare il resto della riga.

**Perche' un terminatore `--`.** Senza, non esiste modo di caricare un file il cui nome
comincia con `-`. Con, il problema e' risolto e resta un caso solo da documentare.

**Perche' `-` da solo non e' un'opzione.** La condizione su `a[1]` lo esclude dal
riconoscimento, perche' `-` non nomina nessun carattere di opzione. Cosa significhi non e'
deciso qui: per il compilatore e' lo standard input (D5), per l'interprete resta un percorso.

### D4. Un valore mancante su un'opzione che vuole un valore e' un errore esplicito

Se un'opzione vuole un valore, il valore e' quello che segue il `=` nella stessa stringa
(`-o=x`, `--out=x`) oppure l'argomento immediatamente successivo. Se l'argomento successivo
non esiste, il modulo riporta l'opzione come sconosciuta insieme al motivo, invece di lasciare
il chiamante con un valore mancante.

Alternativa scartata: consegnare "flag presente, valore assente" e lasciare che ogni strumento
se ne accorga. Sposta il controllo in due posti diversi e rende l'errore dipendente da quale
strumento e'.

### D5. `-` da solo indica lo standard input per il compilatore

Il ciclo consegna `-` come posizionale. Nel `main()` del compilatore un posizionale il cui
contenuto e' `-` viene tradotto leggendo da stdin e scrivendo su stdout, cioe' il ramo che gia'
esiste per `argc<2`.

Perche' e non lasciare `-` come nome di file: `-` non e' un percorso che qualcuno abbia voluto
aprire, e in ogni altro strumento significa "leggere dallo standard input". Il risultato e' che
il ramo stdin del compilatore ha due forme invece che una, senza una modalita' nuova.

### D6. La tabella del compilatore parte vuota, ed e' un risultato

`parser.c` dichiara una tabella vuota e usa comunque il ciclo. Il comportamento osservabile
del compilatore cambia lo stesso, perche' e' cambiato cosa arriva a `main()`: gli argomenti
sono gia' divisi fra opzioni e sorgenti.

Non si aggiunge un flag al compilatore perche' il primo flag vero che gli serve (`-o`) cambia
la derivazione del nome del file p-code e ha gia' un requisito in `compiler-cli` da modificare
con cura. Metterlo qui allungherebbe il delta di una capability che questa change vuole toccare
il meno possibile.

La tabella vuota non e' pero' una difesa: se il primo flag che arrivera' e' `-o`, la riga c'e'
gia' e il resto del lavoro e' dichiararlo li'.

### D7. `-h` e `--help` non vengono introdotti; `-bench` resta

Tre decisioni sulle grafie, perche' i nomi che gli utenti scrivono sono l'unica parte di una
command line che non si puo' cambiare dopo.

**`-h` non e' un flag.** Dopo D3 `-h` e' un'opzione sconosciuta, quindi la diagnostica passa da
"non posso aprire `-h.pcd`" a "opzione `-h` sconosciuta", sempre con codice di uscita 1.
L'opzione per l'aiuto si aggiunge quando serve, come una riga di tabella e una riga di testo
nell'uso, che la tabella genera gia' (D8).

**`--bench` e `-b` sono le forme nuove, `-bench` resta.** `AGENTS.md` documenta `-bench` e
diversi scenari di `interpreter-cli` la nominano, quindi toglierla romperebbe un contratto
scritto. Le tre grafie sono la stessa opzione: una riga di tabella con `lungo="bench"` e
`corto='b'`, piu' il confronto esatto per `-bench`, che il modulo non puo' fare da solo perche'
non e' un nome lungo ne' un nome corto ma una parola intera preceduta da un solo trattino.

**Nessun clustering di opzioni corte.** `-vv` non si puo' scrivere finche' `-v` non esiste. Il
ciclo confronta `a[1]` e basta: mettere `-vv` vuol dire che `a[1]` e' un'opizione e `a[2..]`
ne sono altre, e va scritto per bene quando ce n'e' bisogno.

### D8. Il testo dell'uso viene dalla tabella, non a mano

`cli.c` espone anche un ciclo che stampa le opzioni riconosciute, usando i campi `lungo`,
`corto` e `tipo` della tabella. Il `main()` dell'interprete chiama quel ciclo al posto delle
righe scritte a mano.

Perche': oggi il testo dell'uso e' una seconda copia dell'elenco delle opzioni, e le due copie
hanno gia' un motivo per divergere (il testo non dice che `-h` non e' un flag). Con la tabella
come unica fonte, aggiungere un flag allinea codice, uso e specifica in un passo solo.

Alternativa scartata: scrivere l'uso a mano e ricordarsi di aggiornarlo. Sono due verita' su
una cosa che l'utente legge proprio quando ha sbagliato.

### D9. Il valore di un'opzione resta grezzo

Quando un'opzione vuole un valore, il modulo consegna la stringa e basta: non interpreta numeri,
non distingue `on` da `off`, non valida niente. Ogni strumento converte e valida nel proprio
codice.

Perche': la validazione di un valore dipende da cosa lo strumento fa con quel valore, e metterla
nel modulo significa che il modulo deve conoscere i tipi di tutti i valori di tutti gli
strumenti. Il modulo puo' pero' dire che il valore manca (D4).

Nessuno strumento dichiara oggi un'opzione con valore, quindi questa parte del ciclo e' scritta
e verificata da `cli_test.c` e non dai binari. E' un costo consapevole: il ciclo copre una
forma che non e' ancora in uso, cosi' il primo flag con valore che il progetto si dara' non
dovra' aspettare che il ciclo impari a leggerlo.

## Risks / Trade-offs

- **Gli scenari esistenti sull'opzione di misura cambiano.** `interpreter-cli` ha uno scenario
  che dice che `primo.pcd -bench` e' un errore e uno che dice che `--bench` non e' un'opzione:
  entrambi sono modificati nel delta.
  → Accettato: e' il costo di avere un meccanismo di opzioni vero. Lo scenario "accetta un solo
  argomento" resta vero per i percorsi.

- **Un file il cui nome comincia con `-` non e' piu' compilabile senza `./` o `--`.** Vale per
  `compiler fib.cpl -x` e per `./interpreter -h`.
  → Accettato: e' la convenzione di ogni altro strumento, e `--` mette a disposizione la via
  d'uscita. Le istruzioni d'uso dell'interprete citano `--`.

- **Un'opzione sconosciuta ferma il compilatore, mentre un sorgente irraggiungibile no.**
  L'asimmetria e' voluta ed e' nella specifica, ma e' surprising: `compiler a.cpl -x b.cpl` non
  produce `a.pcd`, mentre `compiler a.cpl mancante.cpl b.cpl` lo produce.
  → Accettato: un'opzione sconosciuta riguarda tutta l'invocazione, e continuare significherebbe
  tradurre un sottoinsieme di quello che l'utente ha chiesto senza dirglielo.

- **La tabella del compilatore e' vuota, quindi il ramo del valore con non viene esercitato dai
  binari.**
  → Compensato da `cli_test.c`, che e' l'unico posto dove `-o=x` viene provato. Se quel file non
  viene scritto o non viene eseguito, quel ramo resta senza verifica.

- **`cli.c` incluso in due unita' di traduzione viene compilato due volte.** Due copie di
  funzioni in due binari diversi, non due copie nello stesso binario.
  → Irrilevante: sono programmi separati e il modulo e' di poche decine di righe.

- **I due `main()` divergono ancora nella gestione degli errori.** L'interprete mette tutto su
  stderr e restituisce 1; il compilatore restituisce il numero degli errori. La scansione e'
  comune, il "cosa fare dopo" no.
  → Accettato: e' gia' cosi' e i due codici di uscita sono entrambi in specifica. Unificarli
  sarebbe una change separata, non un effetto collaterale di questa.

- **`-bench` resta, quindi l'interprete ha tre grafie per un flag e il delta le elenca tutte.**
  → Accettato: la compatibilita' costa una riga in tabella e uno scenario in piu'.

## Migration Plan

Nessuna migrazione di dati ne' di formato: il `.pcd` prodotto dal compilatore e' gia' quello
che `loadPcd()` sa leggere, e la sua struttura non cambia.

Cambia solo come i due strumenti leggono i propri argomenti.

Per l'interprete:

- `./interpreter fib.pcd` continua a eseguire una volta;
- `./interpreter -bench fib.pcd` continua a misurare;
- `./interpreter --bench fib.pcd` e `./interpreter -b fib.pcd` diventano equivalenti alla forma
  precedente e sono forme nuove;
- `./interpreter fib.pcd -bench` passa da errore a misura;
- `./interpreter -h` passa da "non posso aprire `-h.pcd`" a "opzione `-h` sconosciuta", con
  codice di uscita 1 in entrambi i casi;
- `./interpreter -h fib.pcd` passa da "misuro `-h.pcd`" a "opzione sconosciuta".

Per il compilatore:

- `compiler`, `compiler fib.cpl`, `compiler fib primo test/a` e `compiler < fib.cpl > fib.pcd`
  restano identici;
- `compiler -x fib.cpl` passa da "non posso aprire `-x`" a "opzione `-x` sconosciuta", e non
  produce p-code;
- `compiler -` e `compiler -- nome.cpl` sono forme nuove.

Rollback: ripristinare `parser.c`, `interpreter.cpp`, `cli.c` e `Makefile`, piu' `AGENTS.md` e
`todo.md` se sono stati riallineati. Nessun dato persistito e' coinvolto e nessun formato
cambia, quindi il rollback e' il ripristino dei sorgenti. Se la change non e' ancora stata
archiviata, il rollback completo include la rinuncia alla change stessa, perche' la specifica
continua a descrivere lo stato precedente finche' il delta non viene sincronizzato.

## Open Questions

- **Il nome lungo canonico di un flag coincide con la forma breve?** Oggi `--bench` e `-b`
  derivano dallo stesso nome. Quando arrivera' il primo flag del compilatore si vedra' se `-o`
  e `--output` vogliono dire la stessa cosa o se sono due nomi distinti che la tabella sa gia'
  esprimere con due righe. La tabella non richiede una risposta adesso.

- **L'uso del compilatore resta vuoto?** Oggi `compiler` non stampa istruzioni d'uso e non ha un
  ramo dedicato: `main()` va subito in `argc<2`. Con la tabella vuota non c'e' niente da
  elencare, ma quando il primo flag arriva sara' naturale aggiungere un ramo d'uso. Se quel
  ramo vada su stderr con codice 1 come per l'interprete, o su stdout come un `--help`, e' una
  decisione che si prende con il primo flag, non prima.

- **`todo.md` deve essere riallineato?** Riporta al punto 1 "specificare i file da compilare in
  cli" e al punto 2 "specificare i file da eseguire in cli", entrambi gia' chiusi. Come per la
  change precedente, se riallinearlo e' un task, resta una domanda dell'autore del file su quali
  punti chiudere.