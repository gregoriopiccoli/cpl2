# Design

## Context

Vedi `proposal.md` per la motivazione e `specs/compiler-cli/spec.md` per il contratto. Qui i
fatti del codice che condizionano l'approcpo.

`parser.c` e' l'unica translation unit del compilatore: include `pcodes.c`, `emit.c` e il
`cpl2.c` generato da lemon. Non c'e' alcuna dipendenza esterna e non c'e' un parser di
argomenti: la command line si riduce a un ciclo su `argv[1..argc-1]`.

Oggi `main()` (righe 435-470) fa tre cose per ogni argomento, nell'ordine:

1. `pcodePath(argv[i], pcdPath)` deriva il nome del `.pcd` dall'argomento;
2. `srcFile = argv[i]` e `fopen(argv[i], "r")`: se il `FILE*` e' `NULL`, `diag("impossibile
   aprire il sorgente: %s\n", strerror(errno))`, `errors++`, `continue`;
3. `fopen(pcdPath, "w")`, `emit_out = fOut`, `errors += parse(fIn)`, chiusura degli stream.

Le tre cose hanno oggi la **stessa** sorgente, `argv[i]`. La change le separa: esiste un
percorso risolto, e da quello derivano le tre.

`pcodePath()` (righe 426-433) implementa gia' la regola «il punto si cerca solo nel nome di
base»: calcola `slash = strrchr(src,'/')`, poi `base`, poi `dot = strrchr(base,'.')`, e
tronca da `dot`. Il nome di base e' quindi gia' isolato: la condizione «non c'e' estensione» e'
esattamente `dot == NULL` su quel valore.

`diag()` (righe 19-26) stampa `"<srcFile>: "` quando `srcFile != NULL`. `srcFile` e' un
`const char*` globale usato da tre chiamate: `tokenize()` per `stringa non terminata` e
`numero terminato male`, e l'azione `%syntax_error` in `cpl2.gram`. Quindi cambiare il valore
di `srcFile` copre tutte le diagnostiche, senza toccare nessuna delle tre.

`errno` va maneggiato con attenzione: il ramo di apertura e' l'unico che usa `strerror(errno)`,
e un secondo `fopen` fra la rilevazione del primo errore e la lettura di `errno` lo
sovrascriverebbe.

Il puntatore restituito dalla risoluzione viene usato per `fopen`, per `srcFile` e per
`pcodePath()`, quindi deve restare valido **per tutta l'iterazione**: il candidato `.cpl` non
puo' stare in un buffer di pilota che muore all'uscita dell'helper.

Le convenzioni di dimensione del file sono `2000` (`char line[2000]`, `char token[2000]`,
`char pcdPath[2000]` in `main()`), senza costante condivisa.

## Goals / Non-Goals

**Goals:**
- `compiler fib` funziona quando esiste `fib.cpl`, e non cambia nulla per le invocazioni che
  gia' funzionano o che gia' falliscono.
- Una sola nozione di «percorso del sorgente» in `main()`, dalla quale derivare apertura,
  diagnostiche e nome del `.pcd`.
- La regola «il punto si cerca solo nel nome di base» scritta una volta sola in `parser.c`.
- Il ramo `argc < 2` (stdin) byte per byte invariato.

**Non-Goals:**
- Nessuna modifica a `parse()`, a `pcodePath()` come contratto di output, a `emit.c`, ad
  `asm.c`, a `pcodes.*`, a `cpl2.gram`, al formato del `.pcd` o all'header `#cpl20.0.1`.
  Quindi nessuna rigenerazione di `cpl2.c`/`cpl2.h`.
- Nessun ripiego `.cpl` per un nome che ha gia' un'estensione: fuori perimetro della change,
  resta aperto in `Open Questions`.
- Nessun flag, nessuna opzione, nessuna distinzione fra opzioni e percorsi: `-h` resta un
  percorso. Stessa scelta di `compiler-cli` e `interpreter-cli`.
- Non si introduce una lista di estensioni prova (`cpl`, `txt`, ...): la richiesta e' `.cpl` e
  una lista cambierebbe il contratto di `./compiler sorg.txt`, che resta verbatim.
- Non si distingue `ENOENT` da `ENOTDIR` e non si rileva il caso «argomento e' una
  directory», che oggi non e' un errore esplicito (vedi `Risks`).

## Decisions

### D1. La risoluzione sta in `main()`, non in `parse()`
`parse(FILE* f)` riceve uno stream gia' aperto e non sa nulla del nome: e' il punto sbagliato
per una decisione che dipende dal percorso su disco. La risoluzione va nel ramo multi-file di
`main()`, dove oggi si decide anche `pcdPath` e `srcFile`.

Un helper che apre e dice quale percorso ha aperto:

```c
// apre il sorgente indicato in command line. Se il percorso digitato non si
// apre perche' non esiste e il nome di base non ha estensione, si prova anche
// accodando ".cpl". *resolved riceve il percorso effettivamente aperto, che
// resta valido almeno fino alla chiamata successiva: in modalita' file e' la
// sorgente di srcFile e di pcodePath().
FILE* openSource(const char* arg, const char** resolved);
```

Perche' un helper e non inline in `main()`: `main()` sta per crescere di un blocco `if`
annidato dentro un ciclo gia' a tre blocchi, e la regola (prova, ricorda `errno`, prova
`.cpl` solo se il nome di base non ha punto, scegli il messaggio) merita un nome e un commento
propri.

Alternativa scartata: risolvere il percorso in una funzione che **non** apre, tipo
`const char* resolve(const char* arg)` che fa uno `stat` e restituisce `arg` o `arg + ".cpl"`.
Piu' pura da leggere, ma introduce un TOCTOU fra lo `stat` e il `fopen`: in mezzo ci puo'
essere un `rm`, e in piu' serve `stat.h`/`struct stat` per una cosa che `fopen` ti dice
gratis.

Alternativa scartata: normalizzare tutti gli argomenti in un array di percorsi risolti prima
del ciclo. Un solo criterio e' fallito per uno solo argomento: non vale una passata in piu'.

### D2. Il percorso digitato ha la precedenza, e il ripiego e' sul solo `ENOENT`
Due decisioni collegate, che insieme danno la precedenza richiesta:

- se `fopen(arg)` riesce, `arg` vince e il ripiego non viene neppure provato;
- se `fopen(arg)` fallisce, il ripiego su `arg + ".cpl"` viene tentato **solo** se
  `errno == ENOENT`, cioe' solo se `arg` non esiste.

Perche' `ENOENT` e non «qualunque fallimento»: se `fib` esiste ma non e' leggibile, passare
silenziosamente a `fib.cpl` significherebbe tradurre un file che l'utente non ha nominato. Il
comportamento onesto e' dire che `fib` non e' apribile, con il suo `strerror`. Il prezzo e'
una comparison su `errno`, che e' gia' li' nel ramo che chiama `strerror`.

Alternativa scartata: ripiegare su qualunque errore di apertura. Piu' semplice da scrivere e
piu' «utile» quando `fib` e' un file rotto, ma trasforma un errore esplicito in una
sostituzione silenziosa. Il contratto dell'interprete non e' un precedente: `pcodePathFor()`
accoda `.pcd` incondizionatamente perche' li' non ci sono due file candidati, solo uno.

Nota: con `ENOENT` come condizione, l'unico caso in cui il ripiego fallisce e' «`fib.cpl`
esiste ma non si apre». In quel caso il messaggio deve comunque citare i due percorsi, perche'
l'utente ha scritto `fib` e deve poter capire che cosa e' stato cercato.

### D3. `errno` del primo tentativo va salvato prima del secondo
Nel ramo di apertura, se il primo `fopen` fallisce, `errno` va copiato in una variabile locale
prima di tentare `.cpl`, altrimenti `strerror(errno)` nel messaggio finale riporta l'errore
del secondo tentativo — che e' quello giusto solo quando anche il secondo fallisce per
assenza. La copia costa una riga ed e' l'unico modo di non riportare il motivo sbagliato.

### D4. `srcFile` e `pcodePath()` prendono il percorso risolto
`srcFile = *resolved` invece di `argv[i]`, e `pcodePath()` viene chiamata con `*resolved`.

Per `srcFile` e' un requisito di `specs/compiler-cli/spec.md`: il prefisso deve essere «il
percorso del file realmente tradotto», e `diag()` centralizza gia' tutte le diagnostiche, quindi
una sola assegnazione copre scanner, `%syntax_error` e apertura.

Per `pcodePath()` il risultato e' **identico**: lo stem di `fib` e di `fib.cpl` e' lo stesso,
quindi `fib.pcd` in entrambi i casi, e la regola «estensione sostituita nella stessa
directory» non cambia. Si cambia perche' `main()` abbia un solo termine per «il percorso del
sorgente» invece di due che coincidono solo per caso: se `pcodePath()` cambiasse domani
domani, il chiamare con l'argomento grezzo resterebbe indietro.

Nota: `pcodePath()` va chiamata **dopo** la risoluzione, non prima. Oggi sta prima dell'apertura
e il suo risultato viene buttato se il sorgente non si apre; spostarla dopo la rende dipendente
dal percorso risolto senza cambiare null'altro, perche' sul ramo di errore si fa `continue`
come adesso.

### D5. Un helper unico per «il punto e' nel nome di base»
La regola serve a due posti in `parser.c`: `pcodePath()` (tronca) e la risoluzione (accoda
`.cpl` solo se non c'e' punto). Si estrae:

```c
// restituisce l'ultimo '.' presente nel solo nome di base del percorso, NULL se
// il nome di base non ne contiene: la directory non viene considerata, quindi
// "dir.d/x" non ha estensione. Usata sia da pcodePath() sia dalla risoluzione del
// sorgente in command line, cosi' la regola sta in un posto solo.
const char* baseDot(const char* path);
```

`pcodePath()` la usa al posto dei suoi tre calcoli (`slash`, `base`, `dot`), che diventano
`dot = baseDot(src)` e `stem = dot!=NULL ? dot-src : strlen(src)`. Il comportamento e' lo stesso:
`baseDot` restituisce il `.` nel nome di base, quindi `dot-src` e' l'indice giusto anche con
una directory che contiene punti.

Alternativa scartata: duplicare i tre calcoli nella risoluzione. Sono cinque righe, ma sono
esattamente la regola che `pcodePath()` ed `interpreter.cpp` hanno gia' in due copie ciascuno;
una terza copia in quello stesso file e' il punto in cui le copie iniziano a divergere.

Nota: `interpreter.cpp` ha la sua copia (`pcodePathFor()`), e resta cosi': i due programmi non
compilano insieme, sono in linguaggi diversi e hanno regole opposte (sostituzione contro
accodamento). Condividere richiederebbe un header, che non e' giustificato da cinque righe.

### D6. Il buffer del candidato e' statico, e la sua durata e' vincolata
Il percorso candidate (`arg` + `".cpl"`) e' una stringa nuova: `arg` punta dentro `argv` e non
puo' essere esteso. Il buffer non puo' stare in `main()` e non puo' stare in pilota
nell'helper, perche' `*resolved` viene usato come `srcFile` e come argomento di `pcodePath()`
**dopo** la ritornata, e deve valere fino alla fine dell'iterazione. Quindi e' `static` dentro
l'helper:

```c
static char cplPath[2000];   // stessa dimensione di pcdPath e line
```

`static` e' sicuro perche' il programma e' single-threaded e l'helper viene chiamato una volta
per argomento: al turno successivo il buffer viene riscritto, e a quel punto `srcFile` e
`pcdPath` sono gia' stati consumati. Il vincolo di durata va scritto nel commento
dell'helper, perche' e' la parte non ovvia.

Dimensione `2000` per coerenza con `line`, `token` e `pcdPath`: un percorso piu' lungo viene
troncato, esattamente come accade gia' per `pcodePath()`. Non e' una regressione e non vale la
pe' introdurre `PATH_MAX` solo qui. La copia usa `snprintf`, cosi' il troncamento non puo'
scrivere oltre il buffer.

Alternativa scartata: passare il buffer dal chiamante (`char cand[2000]` dentro il ciclo di
`main()`, gia' allineato a `pcdPath`). Ha il vantaggio di non avere durata implicita, ma
esporta un dettaglio di implementazione nella firma dell'helper e allunga la chiamata di un
parametro per un caso che non e' un problema pratico (vedi `Risks`).

### D7. Il testo del messaggio nomina i percorsi, ma non e' fissato dalla spec
La spec fissa **che** deve essere detto (i percorsi tentati, il fatto che il sorgente non si
apre), non le parole. Il messaggio esistente e' `impossibile aprire il sorgente: %s` con lo
`strerror`. La forma naturale e' Keep il verbo e passare a elencare i tentativi, per esempio
`impossibile aprire il sorgente: fib, fib.cpl: No such file or directory`. Il testo esatto si
sceglie in implementazione: se un test futuro lo fisserà, sara' una modifica di una riga, non
degli artifact.

Da non fare: due messaggi diversi per «non esiste nessuno dei due» e per «esiste ma non e'
leggibile». Il secondo arriva dal ramo `errno != ENOENT`, che mantiene il messaggio di oggi con
un solo percorso: e' la condizione normale (permessi), non un caso eccezionale da
rimaneggiare.

### D8. Nessun impatto fuori dal ramo multi-file
Il ramo `argc < 2` (stdin) non viene toccato: `srcFile = NULL`, `emit_out = stdout`,
`return parse(stdin)`. Non ha argomenti, quindi non ha percorsi da risolvere, e la specifica di
`Compatibilita' della modalita' standard input` richiede che produca lo stesso risultato e le
stesse diagnostiche di prima. Nessun ramo della risoluzione deve poter essere raggiunto da li'.

## Risks / Trade-offs

- **Un file senza estensione "ombra" il suo `.cpl`.** Se nella directory ci sono `fib` e
  `fib.cpl`, `compiler fib` traduce `fib`. E' il comportamento richiesto (precedenza del
  percorso digitato) ed e' quello di oggi, ma e' una divergenza rispetto a quello che un utente
  puo' aspettarsi vedendo due file con lo stesso nome.
  → La precedenza e' dichiarata nella spec con uno scenario dedicato, e il prefisso delle
  diagnostiche (D4) dice sempre quale file e' stato tradotto: se l'utente vede `fib: syntax
  error` quando credeva di aver compilato `fib.cpl`, glielo dice il messaggio, non deve
  indovinarlo.

- **Il percorso di output non cambia mai, quindi un errore di risoluzione non lascia traccia
  nella lista dei file prodotti.** `compiler fib` e `compiler fib.cpl` producono lo stesso
  `fib.pcd`; se la risoluzione sbaglia, l'utente vede un `fib.pcd` con il contenuto del file
  sbagliato.
  → Mitigato dalla precedenza dichiarata e dal prefisso. Una difesa ulteriore (un `-v` che
  nomini il sorgente) e' fuori perimetro: nessun flag.

- **`snprintf` su `cplPath[2000]` tronca un percorso lungo.** Un sorgente con un percorso
  oltre ~2000 caratteri non viene risolto e l'utente riceve un «non apribile» su un percorso
  troncato, che e' poco leggibile.
  → Lo stesso limite gia' esiste per `pcodePath()`/`pcdPath` e per `line`/`token`; e' un
  limite noto del file, non una regressione. Il troncamento avviene senza scrivere fuori dal
  buffer. Se un giorno il limite diventa un problema, si alza `2000` in un colpo solo o si
  passa a `PATH_MAX` per tutti i percorsi: e' una modifica separata e non coerente fino ad
  allora spostare solo questo.

- **`errno` non e' affidabile dopo un `fopen` su directory.** Un argomento che e' una
  directory si apre con successo su glibc (l'errore arriva alla prima lettura), quindi non
  cade in nessuno dei due rami e il compilatore entra in `parse()` su uno stream non
  leggibile. Comportamento di oggi, non introdotto qui.
  → Annotato in `Non-Goals` e non coperto dagli scenari: richiederebbe uno `stat` o un tentativo
  di lettura, che sono entrambi una modifica al contratto di apertura degli argomenti.

- **`fopen("fib", "r")` su un file senza permessi di lettura e la precedenza**: e' il caso D2.
  Il rischio e' il contrario di quello che sembra: senza il controllo su `errno` il
  comportamento sarebbe *piu' permissivo* (compila `fib.cpl`), non piu' restrittivo.

- **Il troncamento silenzioso del percorso puo' produrre un nome di percorso diverso ma
  valido**, se i 2000 caratteri tagliano nel mezzo di un percorso corto in una directory
  lunga. Il secondo `fopen` puo' allora aprire qualcosa di inesistente e fallire: nessun
  file sbagliato viene compilato, ma il messaggio cita un percorso troncato.
  → Accettato insieme al limite `2000` sopra; nessun percorso corretto viene perso.

## Migration Plan

Nessuna migrazione di dati e nessun formato nuovo: il `.pcd` prodotto e' identico a quello di
oggi e l'header `#cpl20.0.1` non cambia.

Cambia solo la superficie della command line, in modo additivo:

- `./compiler fib.cpl` si comporta esattamente come prima;
- `./compiler fib` dove `fib` esiste si comporta esattamente come prima;
- `./compiler fib` dove esiste solo `fib.cpl` passa a funzionare: e' l'unica invocazione il cui
  esito cambia, e passa da `codice 1` a `codice 0`;
- `./compiler fib` dove non esiste nessuno dei due resta un errore, ma il messaggio cita due
  percorsi invece di uno.

Nessuna invocazione che oggi funziona smette di funzionare, quindi non e' un cambio di
contratto per gli script esistenti.

Rollback: ripristinare `parser.c` e la sezione «Compiler» di `AGENTS.md`. Nessun dato
persistito e' coinvolto e nessun `.pcd` esistente cambia significato.

## Open Questions

- **Ripiego anche per un'estensione diversa da `.cpl`** (`compiler sorg.txt` -> prova
  `sorg.cpl`). E' la generalizzazione naturale di questa change, ma cambia un contratto gia'
  scritto nella spec (`compiler sorg.txt` traduce `sorg.txt` e scrive `sorg.pcd`) e richiede
  una lista di estensioni prova, che si porta dietro la decisione «`.cpl` e' l'unica
  estensione dei sorgenti?». Se la risposta e' no, la lista e' arbitraria. Vale una change
  separata con una sua sezione di delta.
- **Il percorso risolto dovrebbe finire anche nell'output della riga di comando?** Oggi in
  modalita' file non viene scritto nulla su stdout e la diagnostica resta l'unico canale.
  Dire `fib.cpl` su stdout a ogni sorgente romperebbe la specifica «il compilatore non scrive
  p-code su stdout» solo se non si distingue, quindi e' una domanda di contratto, non un
  dettaglio: nessuno scenario delle spec attuali lo nomina, quindi si puo' decidere dopo senza
  toccare gli artifact.
- **`todo.md`**: il punto 2 «specificare i file da eseguire in cli» e' stato chiuso dal commit
  `e616f62` (lato interprete) ma la voce non e' stata spuntata. Spuntarla e' una questione di
  igiene del file e non riguarda questa change; nessuno scenario la copre.
- **Il caso `ENOTDIR`** (`compiler fib/x` dove `fib` e' un file) cade nel ramo «non
  apribibile» senza tentare il ripiego, perche' `errno` e' `ENOTDIR` e non `ENOENT`. Se in
  futuro si vuole che anche questo tenga il ripiego, basta allargare la condizione a
  `ENOENT || ENOTDIR`: e' una modifica locale a D2 e non cambia nessuno scenario della spec
  perche' nessuno scenario nomina `ENOTDIR`.