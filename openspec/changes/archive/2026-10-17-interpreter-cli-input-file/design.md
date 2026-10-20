# Design

## Context

Vedi `proposal.md` per la motivazione. Qui i fatti del codice che condizionano l'approccio.

`interpreter.cpp` e' l'unica translation unit dell'interprete: include `gc.cpp`,
`pcodes.h`/`pcodes.c`, e `nanobench.h` e' vendored. Non c'e' alcuna dipendenza esterna e non
c'e' un parser di argomenti: la command line si riduce a `argv[1]`.

`main()` oggi e' `int main()` senza parametri. Dopo il caricamento/esecuzione stampa un
rapporto sul GC (`stdGC().status()`, `collectall()`, `printLocked()`), tutto su `cout`:
e' strumentazione di sviluppo, non parte del contratto del programma.

Il perimetro di esecuzione esiste gia' ed e' pronto: `runPcd(const string& fn)` (chiamata
`test()` fino alla modifica non committata nell'albero di lavoro) fa esattamente
load + `interp::run()` + `assert(intp.sp==-1)`, restituendo `void`. `main()` la chiama con
una stringa letterale e le funzioni `bench*()` la richiamano dentro nanobench. Nessuna
modifica al suo corpo e' necessaria.

`pcodeProgram::loadPcd(string fn)` ha gia' la firma giusta: restituisce `1` se il p-code e'
caricabile e `0` altrimenti, e in tutti i casi lascia in `prg` un `pcodePCodeEnd(0)`, cosi'
che un programma non caricabile non venga mai eseguito. Ha pero' due difetti rispetto al
contratto che questa change fissa:

1. i due messaggi che emette, `not a compiled cpl2 file ...` e `<fn> has errors!`, vanno
   su `cout`, quindi finiscono mescolati all'output del programma e su stdout;
2. il caso **file non apribile** non produce alcun messaggio: `ifstream pcd(fn)` fallisce,
   `pcd.good()` e' falso e la funzione esce con `return 0` in silenzio.

Il terzo caso (intestazione diversa) e' l'unico che il chiamante puo' distinguere da solo,
perche' e' l'unico in cui `loadPcd` parla. Non serve un codice di ritorno piu' ricco di
quello che c'e' gia'.

Sul lato compilatore c'e' `pcodePath()` in `parser.c` che fa l'operazione **opposta**: dato
un sorgente, toglie l'estensione e accoda `.pcd`. La regola dell'interprete e' "se non c'e'
estensione, accoda `.pcd`", cioe' e' l'inversa. Le due funzioni non possono condividere
codice: stanno in due programmi diversi, in due linguaggi diversi, e con regole opposte.

L'albero di lavoro ha gia' modifiche non committate a `interpreter.cpp`: il rinominamento di
`test()` in `runPcd()` e i bench commentati. Questa change parte da li' e non deve scartarle.

## Goals / Non-Goals

**Goals:**
- `main()` con `argc`/`argv`: esattamente un argomento, il p-code da eseguire.
- Derivazione del percorso in un helper unico, leggibile, con la regola "solo nome di base".
- Ogni motivo di caricamento fallito produce un messaggio su stderr che nomina il file, e
  un codice di uscita non zero. Oggi un file in rapporto mancante e' silenzioso.
- Il corpo di `runPcd()` e la struttura di `loadPcd()` restano quelle di oggi: la change e'
  di perimetro, non di struttura.

**Non-Goals:**
- Nessun flag: niente `-h`/`--help`, niente `-` come sinonimo di stdin, niente opzioni che
  inizino con `-`. Un nome che inizia con `-` e' un percorso. Stessa scelta di
  `compiler-cli`: le opzioni sono una questione separata.
- Non si tocca il rapporto del GC: resta stampato su stdout dopo l'esecuzione, anche se
  mescola strumentazione di sviluppo con l'output del programma.
- Non si normalizza il fallimento a runtime: `not impl!` e gli altri `domain_error` non
  intercettati terminano il processo con `abort()`. Non e' introdotto qui e non e' un
  requisito di questa change.
- Non si cancellano `bench()`, `bench_cc()`, `bench_fib_cc()`, `test_cc()`, `test_fib_cc()`
  e `releaseSysObjs()`: smettono solo di essere chiamate da `main()`.
- Nessuna modifica a `parser.c`, `emit.c`, `asm.c`, `cpl2.gram`, al formato del `.pcd` o
  all'header `#cpl20.0.1`. Quindi nessuna rigenerazione di `cpl2.c`/`cpl2.h`.

## Decisions

### D1. `main(int argc, char** argv)` con esattamente un argomento
`argc == 2` e' l'unico caso che esegue. `argc < 2` e `argc > 2` stampano su stderr le
istruzioni d'uso — nella forma minima `uso: interpreter <file.pcd>`, piu' la regola del
completamento con `.pcd` — e restituiscono 1. Il testo dell'uso e' fissato dalla spec solo
per la parte osservabile (su stderr, nessuna esecuzione, codice 1), non parola per parola.

Perche' un solo argomento e non una lista: l'utente ha scelto il file singolo, e l'esecuzione
di piu' programmi nello stesso processo non e' un bisogno noto. Le funzioni `bench*()` mostrano
che eseguire piu' programmi in sequenza e' tecnicamente possibile (ognuna costruisce il
proprio `pcodeProgram` e il proprio `contextObj`), quindi la scelta e' di contratto, non di
limite tecnico.

Alternativa scartata: ignorare gli argomenti in eccesso ed eseguire il primo. Un typo
(`./interpreter fib.pcd primo.pcd`) diventerebbe silenzioso.

Alternativa scartata: `argc < 2` che esegue un default (per esempio `primo.pcd` come fa oggi
l'albero di lavoro). Un default silenzioso e' la forma peggiore: l'utente crede di aver
eseguito il proprio file e ha eseguito un altro.

### D2. Il percorso lo deriva un helper, con la regola opposta di `pcodePath()`
Un helper piccolo e dedicato in `interpreter.cpp`, del tipo:

```cpp
string pcodePathFor(const string& arg);   // nome senza estensione -> nome + ".pcd"
```

l'algoritmo e' tre righe: `base = arg dopo l'ultimo '/'`, `dot = ultimo '.' in base`, e se
`dot` non esiste si accoda `".pcd"`. La directory non viene toccata, quindi `dir.d/x` resta
`dir.d/x.pcd`.

Perche' un helper e non inline in `main()`: e' una regola con casi limite (directory con
punti, percorso senza directory, nome che inizia con un punto) che merita un nome proprio e
un commento, e resta verificabile da solo.

Perche' non riusare `pcodePath()` di `parser.c`: le regole sono opposte (sostituzione contro
accodamento condizionato) e i due programmi non hanno nulla in comune da compilare insieme
— sono due binari distinti, uno in C e uno in C++. Il fatto che le due funzioni si somiglino
e' una coincidenza, non un caso da coprire con un header condiviso.

Alternativa scartata: sostituire l'estensione come fa il compilatore, cosi' anche
`./interpreter fib.cpl` arriverebbe a `fib.pcd`. Sarebbe comodo ma rende ambiguo il
contratto: l'argomento sarebbe un sorgente, non un p-code, e `fib.txt` diventerebbe
silenziosamente `fib.pcd`. L'utente ha scelto il nome verbatim.

Nota: un nome di base che inizia con un punto (`.hidden`) viene trattato come avente
estensione e resta verbatim. Caso trascurabile, accettato.

### D3. `runPcd()` restituisce il codice di uscita invece di `void`
`void runPcd(const string&)` diventa `int runPcd(const string&)`: restituisce 0 se il
caricamento e' andato a buon fine (il programma e' stato eseguito), 1 se non lo e' stato.
`main()` diventa `return runPcd(pcodePathFor(argv[1]));`. Il corpo non cambia: si limita ad
aggiungere il `return` dopo l'`assert`.

Perche' cambiare il tipo di ritorno e non duplicare il corpo in `main()`: `main()` deve
sapere se il caricamento e' riuscito per scegliere il codice di uscita, e l'unico posto in
cui quella informazione esiste e' `runPcd()`.

Alternativa scartata: `main()` che istanzia `pcodeProgram`, chiama `loadPcd()` e poi esegue,
lasciando `runPcd()` per `bench()`. Duplicherebbe quattro righe di `interp`/`contextObj` e
i due punti divergerebbero alla prima modifica.

Alternativa scartata: un valore di ritorno tipo enum (`LOADED`/`NOT_A_PCD`/`NOT_FOUND`/
`HAS_ERRORS`). Piu' informativo, ma `loadPcd()` non ha un solo chiamante (`runPcd()`) e
nessuno ha bisogno di distinguere i motivi: il messaggio li nomina gia' (D4).

`bench()` continua a chiamare `runPcd()` ignorando il valore di ritorno: il valore ignorato
in una lambda e' legale, e `bench()` non e' piu' chiamata da `main()` comunque.

### D4. La diagnostica di caricamento resta dentro `loadPcd()`, e va su stderr
`loadPcd()` conosce il motivo del fallimento, quindi e' il posto giusto per il messaggio:

- i due messaggi esistenti (`not a compiled cpl2 file ...`, `<fn> has errors!`) passano da
  `cout` a `cerr`;
- al ramo del file non apribile, che oggi e' silenzioso, se ne aggiunge uno che nomina il
  percorso tentato (gia' derivato con `.pcd` incluso), perche' altrimenti
  `./interpreter non_esiste` non dice che cosa abbia cercato.

Perche' non far risolvere il problema a `main()`: `main()` vede solo `0`, e per distinguere
i tre motivi dovrebbe riaprire il file o fidarsi di `errno`. Il messaggio verrebbe
riscritto in un secondo posto, con il rischio che i due punti si allontanino.

Il tipo di ritorno resta `int` (`1` caricato, `0` altrimenti): e' gia' il contratto che
`runPcd()` usa con `if (r) intp.run();`, e non c'e' un motivo per cambiarlo.

Nota: l'header `#cpl20.0.1` resta hardcoded qui e in `parser.c`. Questa change non lo tocca,
quindi il vincolo di bumparli insieme, da `AGENTS.md`, resta valido.

### D5. I bench escono da `main()`, ma le funzioni restano
`main()` perde il blocco con `runPcd("primo.pcd")`, `bench("primo.pcd")`,
`bench("fib.pcd")` e `bench_fib_cc()`. Le funzioni restano nel sorgente: sono il banco di
misura dello sviluppatore, e cancellarle costerebbe di ricostruirle la prossima volta che
serve misurare. Diventano raggiungibili solo modificando `main()`, ed e' esattamente il
lavoro che questa change toglie di mezzo per i programmi.

Perche' non un flag `--bench`: aggiungerebbe opzioni, che sono fuori perimetro (D1).

Conseguenza accettata e da registrare: `./interpreter` senza argomenti non e' piu' uno
smoke test. `AGENTS.md` lo dice esplicitamente e va aggiornato.

### D6. Nessuna distinzione fra opzioni e percorsi
Un argomento che inizia con `-` viene trattato come un percorso: `./interpreter -h` tenta di
aprire il file `-h` e segnala che non puo' aprirlo. E' la stessa scelta di `pcodePath()` nel
compilatore (che in `design.md` dichiara i flag esclusi) e mantiene i due strumenti
coerenti: nessuno dei due ha un concetto di opzione.

## Risks / Trade-offs

- **Perdita dello smoke test `./interpreter`.** Chi ci faceva affidamento deve ora scrivere
  `./interpreter primo.pcd`.
  → La riscrittura della sezione "Running a programma" di `AGENTS.md` e' un task della
  change, non un accessorio: senza, la documentazione mente.

- **Le funzioni `bench*()` diventano irraggiungibili dalla riga di comando** e il compilatore
  non e' costretto a segnalarle (`-O2` senza `-Wall`, e non sono `static`).
  → Accettato (D5). Se in futuro serve un modo per richiamarle, e' una questione di flag,
  non di questa change.

- **L'output del programma e il rapporto del GC finiscono entrambi su stdout.** Con
  `./interpreter primo.pcd > out.txt`, il file contiene l'output del programma *e* il
  rapporto del GC.
  → Fuori perimetro per scelta (D1, nessun flag): il rapporto del GC e' strumentazione
  esistente, rimuoverlo o spostarlo su stderr e' una modifica separata. Da sapere, perche'
  il behaviour di una redirezione futura dipende da questa decisione.

- **`assert(intp.sp==-1)` e' attivo**: il `Makefile` compila con `-g -O2` e senza `-DNDEBUG`,
  quindi un programma che lascia lo stack sporco termina con `SIGABRT` (codice 134), non con
  un codice di uscita pulito. Non e' introdotto qui, ma il contract "codice 0 se il
  programma arriva in fondo" vale solo per i programmi che lo fanno davvero.
  → Annotato in `Open Questions`.

- **Il nome derivato puo' non esistere.** `./interpreter fibi` cerca `fibi.pcd` e non lo
  trova: senza il completamento il messaggio sarebbe ambiguo, con il completamento e'
  esplicito.
  → Per questo il messaggio di D4 nomina il percorso **derivato**, non quello digitato.

- **Un file vuoto o troncato cade nella diagnostica "non e' un p-code compilato".** Non e'
  distinguibile da un file sbagliato e non serve distinguerlo.
  → Accettato: il messaggio e' comunque corretto.

- **L'albero di lavoro ha modifiche non committate a `interpreter.cpp`.**
  → I task partono da `git diff` esistente e non devono ripristinare `test()` ne'
  riaprire i bench commentati; il diff finale va verificato riga per riga.

## Migration Plan

Nessuna migrazione di dati ne' di formato: il `.pcd` prodotto dal compilatore e' gia'
quello che `loadPcd()` sa leggere, e la sua struttura non cambia.

Cambia solo il modo di invocare l'interprete:

- `./interpreter primo.pcd` esegue il programma come prima;
- `./interpreter` da solo non esegue piu' `primo.pcd`, `fib.pcd` e i bench: stampa
  l'uso ed esce con 1;
- i bench si misurano solo modificando `main()` e ricompilando, come prima.

Rollback: ripristinare `interpreter.cpp` e `AGENTS.md`. Nessun dato persistito e' coinvolto,
nessun formato cambia.

## Open Questions

- Il rapporto del GC stampato su stdout dopo ogni esecuzione dovrebbe andare su stderr o
  essere soppresso? Se in futuro si risolve, sara' una modifica alla strumentazione di
  sviluppo, non al contratto della command line: nessuno degli scenari di questa spec
  riguarda il rapporto del GC (riguardano l'output del programma e le diagnostiche di
  caricamento), quindi si puo' decidere dopo senza toccare gli artifact.
- `assert(intp.sp==-1)` con `NDEBUG` disattivato fa terminare con `SIGABRT` un programma che
  non ripulisce lo stack. Si vuole distinguere quel caso da un normale errore a runtime con
  un codice di uscita pulito? Non e' una decisione di questa change: richiede prima sapere
  quali sono gli errori runtime che si vogliono distinguere, e cambiare il requirement sul
  codice di uscita richiederebbe di rivedere la spec per quello.
- Un file che esiste ma non e' leggibile (permessi) cade nel ramo "non apribile" di D4, che
  non distingue `ENOENT` da `EACCES`. Il messaggio include `strerror(errno)` come fa
  `diag()` nel compilatore? E' un dettaglio del testo del messaggio, che la spec non
  vincola parola per parola: si puo' aggiustare in implementazione senza toccare gli
  artifact.