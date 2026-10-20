# Proposal

## Why

`./interpreter` non ha una command line: `main()` non ha parametri e chiama direttamente
`runPcd("primo.pcd")` (nell'albero di lavoro) o i bench su `primo.pcd`, `fib.pcd` e
`bench_fib_cc()` (in `HEAD`). L'unico modo per eseguire un p-code proprio e' **modificare e
ricompilare `interpreter.cpp`**: non esiste nessun modo di dire al programma *quale* file
eseguire. E' il punto 2 di `todo.md`, esplicitamente lasciato fuori perimetro dalla change
`compiler-cli-input-files` ("restano aperto").

La situazione e' ora matura per chiuderlo: il compilatore da quella change scrive un `.pcd`
accanto a ogni sorgente (`fib.cpl` -> `fib.pcd`), quindi esiste un file su disco da
indicare, ma non c'e' il modo di eseguirlo. Il compile e l'esecuzione di un programma cpl2
restano legati da un passaggio che si puo' solo fare a mano.

## What Changes

- **BREAKING** `interpreter` accetta **un solo argomento**: il p-code da eseguire.
  `./interpreter fib.pcd` carica ed esegue il programma, esattamente come oggi fa
  `runPcd()` chiamata da `main()`.
- **BREAKING** `./interpreter` **senza argomenti** non esegue piu' nulla: stampa le
  istruzioni d'uso su stderr e termina con codice di uscita 1. Il blocco hardcoded di oggi
  (`primo.pcd`, i bench su `fib.pcd`, `bench_fib_cc()`) esce da `main()`.
- **BREAKING** `./interpreter a.pcd b.pcd` (piu' di un argomento) e' un errore: viene
  segnalato su stderr e il codice di uscita e' 1, senza eseguire nulla.
- Il nome del file da caricare, quando **non ha estensione**, viene completato con `.pcd`:
  `./interpreter fib` carica `fib.pcd`. Un nome che ha gia' un'estensione viene usato
  **verbatim**: `./interpreter fib.pcd` carica `fib.pcd`, `./interpreter fib.cpl` tenta di
  caricare `fib.cpl` e segnala che non e' un p-code. L'estensione si cerca **solo nel nome
  di base**, quindi `dir.d/x` resta `dir.d/x.pcd` e non `dir.pcd/d.x.pcd`.
- Un file che non si puo' aprire, che non e' un p-code valido o che ha errori di compilazione
  **produce una diagnostica su stderr e codice di uscita 1**, senza eseguire il programma.
  Oggi un file inesistente viene ignorato in silenzio: `loadPcd()` restituisce 0 e
  `runPcd()` salta `intp.run()` senza dire niente, e il processo esce con 0.
- Il codice di uscita e' 0 quando il programma arriva in fondo.
- Fuori perimetro: nessun flag. Non c'e' `-h`/`--help`, non c'e' `-` come sinonimo di
  stdin, non c'e' un modo di chiedere le statistiche del GC da riga di comando. Vale la
  stessa scelta della `compiler-cli`: le opzioni sono una questione separata.
- Fuori perimetro: il fallimento a runtime del programma interpretato. `not impl!` e gli
  altri errori di dominio fanno lanciare `domain_error` senza che nessuno la catturi, quindi
  il processo termina con `abort()`. Non e' un comportamento introdotto qui e non viene
  normalizzato.

## Capabilities

### New Capabilities
- `interpreter-cli`: contratto della command line dell'interprete cpl2: selezione del file
  da eseguire e sue regole di naming, comportamento in assenza di argomenti o con argomenti
  in piu', diagnostica e codice di uscita quando il file non e' caricabile.

### Modified Capabilities
- Nessuna: `compiler-cli` descrive il contratto di `./compiler`, che questa change non
  tocca.

## Impact

- `interpreter.cpp`: `main()` acquisisce la firma `(int argc, char** argv)` e la logica di
  selezione del file; sparisce il blocco di bench hardcoded. Serve un piccolo helper per
  derivare il percorso da caricare (la regola "senza estensione -> aggiungi `.pcd`" e' la
  regola opposta di `pcodePath()` in `parser.c`, che invece sostituisce).
- `interpreter.cpp`: `pcodeProgram::loadPcd()` oggi non distingue "file inesistente" da
  "p-code non valido": nel primo caso non stampa nulla. Serve una diagnostica, e la
  decisione su dove metterla (in `loadPcd()` o in `runPcd()`/`main()`) e' di `design.md`.
- `interpreter.cpp`: `runPcd()` (il nome `test()` e' stato rinominato in `runPcd()`
  nell'albero di lavoro, **non ancora committato**) e' gia' la funzione giusta: nessuna
  modifica prevista al suo corpo. Le funzioni `bench*()` restano nel sorgente, semplicemente
  non piu' chiamate da `main()`: non sono un'API e non si cancellano.
- `AGENTS.md`: la sezione "Running a programma" descrive `./interpreter` come programma
  senza argomenti che esegue i bench e dice che si deve modificare `main()` per eseguire un
  file proprio. Entrambe le affermazioni diventano false e vanno riscritte.
- Il formato del `.pcd`, l'header `#cpl20.0.1` e `loadPcd` come parser non cambiano: questa
  change tocca solo il perimetro del processo, non il contenuto del file.
- Il compilatore e `./asm` non sono toccati. La grammatica non e' toccata, quindi nessuna
  rigenerazione di `cpl2.c`/`cpl2.h`.