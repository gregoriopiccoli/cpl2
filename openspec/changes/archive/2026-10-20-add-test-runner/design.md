# Design

## Context

Vedi `proposal.md` per la motivazione, e `specs/test-suite/spec.md` per il contratto. Lo stato
attuale che condiziona l'approccio e' questo.

`test/` contiene oggi **una sola** coppia completa, `parm_default.cpl` + `parm_default.out`, e
l'output catturato dell'interprete su `test/parm_default.pcd` e' gia' **identico byte per byte** al
`.out` (verificato: `./interpreter test/parm_default.pcd > out.txt` e `diff out.txt
test/parm_default.out` non stampa nulla, con `rc=0`). La convenzione quindi non e' un'ipotesi da
costruire: e' gia' rispettata dalla fixture che esiste, e il primo test della suite nasce verde.

Gli altri `.cpl` di `test/` (`simple_class`, `simple_func`, `simple_import`, `test_expr1`,
`test_stm1`, `test_var1`) non hanno un `.out` e non devono averlo per forza: secondo `AGENTS.md`
diversi fixture eseguibili abortiscono con `not impl!` (46 p-code su 105 sono implementati), e
`simple_class.cpl` verifica il **blocco p-code atteso scritto dentro il sorgente**, non l'output.
La raccolta dei test non deve quindi toccarli: e' la loro mancanza di `.out` a tenerli fuori.

`cli.c` e' l'unico modulo condiviso fra i binari e ha gia' una scansione delle opzioni
(`cliInizia`/`cliScansiona`/`cliOpzioni`) costruita su una `cliOpzione` con `lungo`, `corto`,
`alias`, `descrizione`, `tipo` e `dest`. E' un modulo **incluso**, non compilato a parte: `parser.c`
lo `#include`, e `cli_test.c` e' l'unico posto in cui viene compilato da solo.

`interpreter.cpp` ha gia' un ramo `--test`/`-t` (`testOpzione`) che oggi chiama
`test_cc_translation()`: una funzione che esegue a mano, dentro il binario dell'interprete, una
sequenza fissa di `runPcd()`/`benchPcd()`. Non e' il sistema di test di questa change: e'
un esercizio di debug, non legge nessuna directory, non confronta nulla con un `.out`, misura
dentro nanobench e **accende l'opzione `-b`**, quindi non puo' servire al confronto byte per byte.

`Makefile` non ha ne' un target `test` ne' un target `clean` (lo dice `AGENTS.md`), e i binari
`compiler`/`interpreter`/`asm` sono costruiti ognuno da una riga. `runtest` deve seguire la stessa
forma. Nel repository non c'e' nessuna libreria esterna, nessun `gitignore`: ogni nuovo file
prodotto va tenuto presente nell'elenco dei build output non tracciati.

Vincoli da `AGENTS.md`: nessun CI e nessun runner (questa change ne introduce uno), nessun
formattatore, commenti e note in italiano, nessun `git add -A`, nessuna rigenerazione della
grammatica, e il `.pcd` resta quello che `loadPcd()` sa leggere.

## Goals / Non-Goals

**Goals:**
- Un eseguibile `runtest` costruito con una sola riga di `make`, sul modello di `asm`.
- Un sistema di test che **usa i binari come li usa l'utente**, senza duplicare ne' la
  compilazione ne' l'esecuzione: se il contratto di `compiler` o `interpreter` cambia, i test lo
  seguono da soli.
- Confronto byte per byte, con un codice di uscita leggibile da uno script: 0 tutto verde, 1
  almeno un fallimento.
- Nessun file lasciato in `test/`: un giro di test non cambia la directory in cui lavora.
- La convenzione `.cpl` + `.out` come **unica** definizione di "test": nessun elenco da
  mantenere, nessun file di configurazione della suite.

**Non-Goals:**
- Un file di test con esito o argomenti dichiarati (vedi `proposal.md`: fuori perimetro).
- Verificare i listing p-code: il confronto con il blocco atteso dentro il sorgente resta la
  verifica manuale descritta da `AGENTS.md`, con `./asm < x.pcd`.
- Trattare le eccezioni `not impl!` come esito accettabile: un test senza `.out` non viene
  raccolto, e un test con `.out` che abortisce e' semplicemente un test fallito.
- La modalita' di misura: `runtest` esegue in modalita' normale, quindi non incontra mai il
  difetto di `benchPcd()` (codice di uscita invertito), che resta fuori perimetro.
- Sostituire o rimuovere il ramo `--test`/`-t` dell'interprete: convive con `runtest` e resta
  com'e' (vedi D7).
- Il comportamento del compilatore con piu' sorgenti, l'header `.pcd`, la grammatica,
  `emit.c`, `pcodes.c`: non toccati.

## Decisions

### D1: `runtest` e' un binario separato, non un flag dell'interprete

Il sistema di test e' un eseguibile nuovo, `runtest`, con il suo `runtest.c`, il suo target nel
`Makefile` e la sua riga di `cc`.

Alternativa considerata: estendere il ramo `-t` gia' presente in `interpreter.cpp`, che oggi
chiama `test_cc_translation()`. Rifiutata per tre motivi indipendenti. Primo: l'interprete e'
l'oggetto sotto test, e un self-test dentro l'interprete non puo' misurare un'interprete che non
parte (una regressione che rompe `main()` rende impossibile anche il test); un binario separato
che invoca `./interpreter` come farebbe l'utente non ha questo problema. Secondo: `-t` misura con
nanobench e accende `-b`, quindi il suo output **non e'** quello di una singola esecuzione e non
puo' essere confrontato con un `.out` (vedi il requisito "Nessuna interazione con la modalita' di
misura"). Terzo: il ramo `-t` non accetta una directory ne' legge `.out`, e farlo crescere dentro
`interpreter.cpp` significherebbe mettere la logica del test runner in un file che non e' suo.

Il costo e' un quarto binario da costruire e un file in piu' nell'elenco dei build output non
tracciati (`AGENTS.md`). Accettato: e' la stessa forma che ha gia' `asm`.

### D2: il test compila ed esegue invocando i due binari, non rifacendo il loro lavoro

Per ogni test `runtest` lancia `./compiler <file>.cpl` e poi `./interpreter <pcd>`.

Alternativa considerata: includere `parser.c`/`pcodes.c` e chiamare `parse()` direttamente, o
includere `interpreter.cpp`. Rifiutata: `parser.c` ha un `main()` e uno stato globale orientato
alla singola invocazione, `interpreter.cpp` e' C++ e include `gc.cpp`, e in entrambi i casi il
test verificherebbe una copia del comportamento che non e' quella che l'utente esegue. In
particolare il test **deve** passare dalla stessa command line documentata (selezione dei
sorgenti, scrittura del `.pcd` accanto al sorgente, derivazione del percorso da caricare), perche'
e' li' che vivono requisiti come quelli di `compiler-cli` e `interpreter-cli`.

Il costo e' un processo in piu' per test e una dipendenza dall'ambiente (`./compiler` e
`./interpreter` devono esistere e devono essere quelli appena costruiti). Accettato: sono la
stessa cosa, e il messaggio di errore quando mancano lo dira'.

### D3: confronto byte per byte, senza normalizzazioni

Il confronto fra l'output catturato e il `.out` e' un confronto di byte, senza trim, senza
normalizzazione di fine riga e senza tolleranze.

Alternativa considerata: ignorare gli spazi bianchi finali o le righe vuote in coda, per non far
fallire un test su una differenza "cosmetica". Rifiutata: e' la differenza fra un test che dice
la verita' e uno che nasconde una regressione di formattazione. `test/parm_default.out` e' gia'
byte-identico all'output reale, quindi la convenzione non richiede tolleranze; e ogni tolleranza
sarebbe una regola in piu' da spiegare e da ricordare.

Alternativa considerata: confrontare usando `diff` invocato come processo esterno `diff <out> <.out>`.
Rifiutata come **meccanismo di confronto** (l'esito e' affidato a un binario esterno che puo'
mancare, e leggere il suo codice di uscita confonde "diversi" con "diff e' esploso"), ma tenuta
come **modalita' diagnostica**: quando un test fallisce, mostrare le righe che differiscono e'
l'unica cosa che rende utile il fallimento. Il confronto che decide l'esito e' fatto da `runtest`,
in memoria.

### D4: l'output catturato e' lo standard output dell'esecuzione, senza filtri e senza flag

`runtest` cattura lo standard output di `./interpreter <pcd>` e lo confronta con il `.out`.

Il fatto misurato: in questo repository l'interprete **non** stampa il rapporto del GC a fine
esecuzione. Il rapporto e' dietro l'opzione `--gc`/`-g` (`interpreter.cpp`: `if (gcOpzione) { ...
}`, prima di `return rc`), e `gcOpzione` vale 0 quando l'opzione non e' dichiarata. Quindi
`./interpreter test/parm_default.pcd` produce **solo** l'output del programma — verificato sopra,
dove il confronto con `parm_default.out` non ha differenze. `AGENTS.md` dice il contrario in due
punti ("`main()` keeps printing the GC report on stdout afterwards in both modes", e il
riferimento al rapporto del GC nella sezione sulla misura): quelle frasi descrivono uno stato
che non c'e' piu' e vanno corrette in questa change, se no il `.out` sembra dover contenere
anche il rapporto.

**Decisione dipendente**: se l'utente vuole un rapporto del GC dentro il `.out`, la via e' usare
l'opzione gia' esistente, cioe' eseguire il giro con `--gc`, non aggiungere un quinto flag
`--test-gc`. Il `.out` resta la fotografia esatta di quello standard output.

Alternativa considerata: catturare con `2>&1`, mescolando gli errori all'output. Rifiutata: le
diagnostiche dei due strumenti **devono** restare su stderr (e' un requisito di `compiler-cli` e
di `interpreter-cli`), e mescolarle renderebbe il `.out` dipendente da messaggi d'errore che non
hanno niente a che vedere con l'output del programma. `runtest` cattura il solo stdout, che e'
anche la cosa che la spec chiama "output dell'esecuzione".

Alternativa considerata: mostrare l'output del programma mentre il test gira (`tee`). Rifiutata
come comportamento normale: il rapporto del giro diventa illeggibile su una fixture verbosa; il
contenuto catturato si mostra **quando il test fallisce** (vedi D6).

### D5: la directory di appoggio e' temporanea e viene rimossa

`runtest` compila **il `.cpl` della directory dei test** invocando `./compiler <dir>/<nome>.cpl`,
il che, per come e' fatto `parser.c` (`pcodePath()`), scrive `<dir>/<nome>.pcd` **accanto al
sorgente**. Per non lasciare quel file in `test/` — la spec lo vieta e `AGENTS.md` documenta che
i `.pcd` in `test/` sono build output — `runtest`:

1. crea una directory di appoggio temporanea;
2. copia il `.cpl` del test dentro la directory di appoggio, con lo stesso nome di base (e il
   `.out` accanto, se serve al rapporto);
3. esegue `./compiler <appoggio>/<nome>.cpl`, che produce `<appoggio>/<nome>.pcd` senz'altro;
4. esegue `./interpreter <appoggio>/<nome>` catturando lo standard output su un file della
   directory di appoggio;
5. confronta quel file con il `.out` **originale**, nella directory dei test;
6. alla fine del giro, rimuove la directory di appoggio con tutto il suo contenuto, **anche se
   ci sono stati fallimenti**.

La copia e' cio' che tiene intatta la directory dei test: il `.pcd` che l'utente ha
eventualmente gia' in `test/` non viene toccato, e non serve nessuna opzione nuova per dire al
compilatore dove scrivere (che non esiste e non e' richiesta da questa change).

Alternativa considerata: compilare in place e cancellare il `.pcd` alla fine. Rifiutata: cancella
un file che non e' nostro se l'utente aveva gia' compilato quella fixture, e lascia la directory
sporca se il giro viene interrotto. La copia costa una copia per test e rende l'isolamento
strutturale invece che affidato alla pulizia.

Alternativa considerata: `chdir()` nella directory di appoggio e usare percorsi relativi.
Rifiutata: `chdir()` e' stato globale del processo e va tenuto d'occhio per ogni percorso (i
binari da invocare, il `.out` da leggere); costruire percorsi assoluti o relativi espliciti e'
piu' lineare e non ha stato nascosto.

Alternativa considerata: una directory fissa `.runtest/` nella radice del progetto, svuotata a
ogni giro. Rifiutata come prima scelta: `mkdtemp()` con schema `runtest-XXXXXX` (in `TMPDIR`,
tipicamente `/tmp`) non lascia niente nel repository nemmeno se il processo viene ucciso, e non
richiede di decidere dove metterla.

Alternativa considerata: catturare l'output in memoria senza file. Parzialmente accettata: il
confronto e' un confronto di byte fra due contenuti caricati, l'output catturato puo' restare in
memoria; il `.pcd` invece **deve** passare dal filesystem perche' e' li' che il compilatore lo
scrive e da li' che l'interprete lo legge.

### D6: esito del test e rapporto del giro

Ogni test ha tre esiti possibili nel rapporto:

- **superato**: la compilazione e' riuscita, l'esecuzione e' riuscita e l'output e' identico al
  `.out`;
- **fallito**: una qualunque delle tre cose non e' riuscita; il nome del test viene stampato e,
  se c'e' una differenza, si mostra la differenza (D3);
- **non eseguito**: non e' un test, quindi non entra nel rapporto (un `.cpl` senza `.out`).

Un `.out` senza `.cpl` e' segnalato come incoerenza e conta come fallimento (requisito
"Output atteso senza sorgente").

Il rapporto finale e' il conteggio dei superati e dei falliti su stderr, con i nomi dei falliti, e
il codice di uscita e' 0 se e solo se non ci sono fallimenti.

Alternativa considerata: fermarsi al primo test fallito (`set -e`). Rifiutata: il valore di una
suite sta nel sapere **quanti** e **quali** test sono rotti dopo una modifica, e fermarsi al primo
lo nasconde.

### D7: il ramo `--test`/`-t` dell'interprete resta dov'e'

`runtest` non lo tocca, non lo usa e non lo rimuove: `interpreter.cpp` e' fuori dal perimetro di
questa change e il suo `test_cc_translation()` resta un esercizio di debug dell'interprete.

Il costo e' un secondo "sistema di test" che vale la pena nominare (vedi Risks): chi legge
`--help` dell'interprete vede `--test` e potrebbe credere che sia la suite. La mitigazione e' nel
testo: la descrizione dell'opzione e i documenti devono dire che `--test` esegue una sequenza
fissa di prove dell'interprete, mentre le fixture di `test/` si eseguono con `runtest`.
L'eliminazione o la fusione dei due e' una change a parte.

### D8: la command line di `runtest`: una directory, e nient'altro

`runtest` accetta **al piu' un argomento**, la directory dei test, con `test` come default. E'
il caso d'uso reale (`./runtest` dalla radice) e la spec non chiede altro.

Lo strumento usa la forma condivisa di `cli.c` (`cliInizia`/`cliScansiona`/`cliOpzioni`) anche se
oggi non dichiara opzioni, per la stessa ragione per cui la tabella vuota del compilatore e' una
scelta e non un buco (`AGENTS.md`, sezione "Wiring"): il giorno in cui servisse un `--verbose` o
un `--gc`, la riga in tabella compare anche nell'uso senza scrivere una seconda volta l'elenco.
La tabella e' dichiarata nello stile di `compiler-cli`, vuota o quasi.

Il costo e' `~180 righe` di modulo incluso che su tre righe di codice utile sembrano sproporzionate.
Accettato, e coerente con `parser.c`.

Alternativa considerata: `runtest` senza `cli.c`, con un `argv[1]` letto a mano. Rifiutata:
riapre la strada a una validazione della command line "per forma esatta" che questo progetto ha
appena eliminato dalle altre due (change `shared-cli-flags`), e le regole di `cli.c` su opzioni e
`--` sono gia' scritte e provate.

## Risks / Trade-offs

- **`runtest` chiama processi esterni e non trova i binari** → i messaggi devono essere espliciti:
  se `./compiler` o `./interpreter` non si riesce a eseguirlo, il test e' fallito con quel motivo
  nel rapporto, non un "errore di sistema" muto. Mitigazione nel task di verifica: eseguire
  `runtest` con i binari presenti e assenti e controllare i due messaggi.
- **La directory di appoggio non viene rimossa** (processo interrotto) → `mkdtemp()` con schema in
  `TMPDIR` non lascia niente nel repository; un giro successivo usa una directory nuova. Il prezzo
  e' una directory orfana in `/tmp`, non un file in `test/`.
- **La copia del `.cpl` nella directory di appoggio non riproduce l'ambiente di origine** (per
  esempio un sorgente che apre un file con percorso relativo) → oggi nessuna fixture lo fa, e il
  contratto del linguaggio per l'apertura dei file non e' oggetto di questa change. Se un domani
  servisse, il test diventa il posto giusto per una variante esplicita.
- **Il `.out` di una fixture puo' contenere il rapporto del GC** → oggi no (l'opzione e' `--gc`),
  ma la frase di `AGENTS.md` che lo afferma va corretta o un `.out` futuro verrebbe scritto con
  un contenuto che l'esecuzione non produce. Mitigazione: task documentale nella prima sezione di
  `tasks.md`.
- **Due sistemi di test (`-t` dell'interprete e `runtest`) confondono chi legge `--help`** →
  mitigazione in D7: la descrizione dell'opzione e la sezione "Tests" di `AGENTS.md` dicono a cosa
  serve ciascuno. La fusione e' una change a parte, non un requisito di questa.
- **La convenzione `.cpl`/`.out` non registra l'esito atteso di un test che deve fallire** → la
  spec non lo chiede: un test e' una coppia che deve combaciare, e un caso "questo sorgente deve
  dare errore" e' fuori perimetro (proposal.md). La conseguenza e' che un `.cpl` che non compila
  non puo' essere un test verde; resta un `.cpl` senza `.out`.
- **Un `.out` che non combacia rende il giro rosso e blocca l'abitudine di eseguirlo** → e' il
  comportamento voluto (D3), ma il rapporto deve dire **quale** test e **come** differisce, se no
  il rosso e' solo rumore. Mitigazione: la differenza mostrata nel rapporto (D3) e il nome del
  test su stderr (D6).
- **`make` guadagna un quarto binario e i documenti lo nominano in tre punti** (`AGENTS.md` sezione
  "Build", elenco dei build output non tracciati, sezione "Tests") → sono un task della change e
  hanno una verifica a grep ciascuno, come nella change `interpreter-help-flag`.

## Open Questions

- Il **come** `runtest` esegue i processi figli — `popen()`/`system()` oppure `fork()`/`exec()`/
  `waitpid()` con `pipe()` — e' una scelta interna che il contratto osservabile non distingue
  (l'output catturato e il codice di uscita sono gli stessi). Va decisa in fase di
  implementazione, guardando quanta parte del lavoro e' "eseguire e leggere un file". Non cambia
  nessuna spec, nessuna decisione qui sopra e nessuna voce di `tasks.md`.
