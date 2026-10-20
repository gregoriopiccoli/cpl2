# Proposal

## Why

La verifica del progetto e' oggi **manuale**: `AGENTS.md` lo dice esplicitamente ("No runner.
Verification is manual: `./compiler test/x.cpl` then `./asm < test/x.pcd` and diff the listing
against the expected block"), e `todo.md` ha fra i DA FARE "5. test test test test". Chi tocca
`interpreter.cpp` o la grammatica deve rifare a mano, ogni volta, la stessa sequenza
(compilare, eseguire, leggere l'output a occhio) su ogni fixture di `test/`, e non ha modo di
accorgersi in un colpo solo se una modifica ha rotto un programma che prima funzionava.

Una parte di quella verifica e' gia' meccanica: `test/parm_default.cpl` ha accanto
`test/parm_default.out`, cioe' l'output atteso scritto a mano dal redattore del fixture. Mancano
solo il ciclo che confronta i due file e il codice di uscita che dice "tutto bene" oppure "questo
test e' fallito". E' l'anello mancante fra la convenzione gia' adottata in `test/` e la
verifica a mano.

## What Changes

- Un nuovo eseguibile **`runtest`** (target `runtest` nel `Makefile`, accanto a `interpreter`,
  `compiler` e `asm`) che prende come argomento **una directory** (default `test`), vi cerca ogni
  coppia `<nome>.cpl` + `<nome>.out`, e per ognuna:
  1. compila il `.cpl` invocando **il binario `compiler`** (non una copia della logica di
     compilazione), come si farebbe a mano;
  2. esegue il `.pcd` prodotto invocando **il binario `interpreter`**;
  3. confronta l'output catturato con il contenuto del `.out`: **uguali** -> test passato,
     **diversi** -> test fallito.
- Il confronto e' **byte per byte**, senza normalizzazioni, senza trimming e senza opzioni che lo
  ammorbidiscano. La cattura dell'output e' quella osservabile: lo standard output dell'interprete
  comprende l'output del programma, l'eventuale tabella di nanobench e il rapporto del GC stampato
  all'uscita (vedi `AGENTS.md`, sezione sull'esecuzione), quindi il `.out` e' una fotografia
  esatta di quello standard output.
- Ogni test e' indipendente: un test fallito **non ferma** gli altri. Alla fine `runtest` stampa
  il conteggio dei test passati e falliti e **termina con 1 se almeno un test e' fallito, 0 se
  tutti sono passati**, cosi' l'esito e' leggibile da uno script.
- Un `.cpl` senza `.out` **non e' un test** e non produce fallimenti: i due file sono la
  convenzione che definisce un test, e i `.cpl` senza `.out` (per esempio `test/test_expr1.cpl`,
  `test/test_stm1.cpl`, `test/test_var1.cpl`) restano quello che sono, uno scratchpad.
- Un `.out` **senza** `.cpl` e' segnalato come incoerenza: viene nominato e conta come fallimento,
  perche' e' un output atteso che nessuno verifica piu'.
- Le fixture che oggi abortiscono con `not impl!` (46 dei 105 p-code sono implementati; le classi,
  `import`, `try`, gli altri tipi e i p-code `and`/`or` no — vedi `AGENTS.md`, "Don't assume a
  construct runs") **non** entrano nei test: senza un `.out` accanto non vengono raccolte. Il
  sistema confronta output di esecuzioni che riescono, non introduce un modo di aspettarsi un
  abort.
- La **directory di appoggio** in cui `runtest` scrive i `.pcd` prodotti e cattura l'output e'
  una directory temporanea rimossa alla fine (pulita anche in caso di fallimento), cosi' un giro
  di test non lascia build output dentro `test/` e non intacca il `.pcd` che l'utente ha
  eventualmente gia' li': i test sono ripetibili e non hanno ordine.
- **Fuori perimetro**: un formato di file di test piu' ricco (esito atteso, argomenti, stdin,
  timeout, test di sola compilazione). L'input e' la sola coppia `.cpl`/`.out`; se un domani
  servisse di piu', sara' una change a parte.
- **Fuori perimetro**: usare `runtest` per verificare le fixture con il blocco p-code atteso
  **dentro** il sorgente (`test/simple_class.cpl` e' l'esempio piu' chiaro, e il loro confronto e'
  quello che `AGENTS.md` descrive con `./asm < x.pcd`). `runtest` esegue e confronta output; non
  guarda il listing e non legge quei commenti.
- **Fuori perimetro**: il difetto noto di `benchPcd()` (esce con l'esito inverso di `runPcd()`).
  `runtest` esegue in modalita' normale e non misura, quindi la misura non entra nel confronto;
  la correzione resta una change separata, come dice `AGENTS.md`.
- **Fuori perimetro**: i p-code non implementati. `runtest` non li dichiara passati e non li
  dichiara falliti: non li vede proprio, finche' non hanno un `.out`.

## Capabilities

### New Capabilities

- `test-suite`: il sistema di test: quali file in una directory formano un test, come il test
  viene eseguito (compilazione + esecuzione + confronto con l'output atteso), che cosa significa
  passare e fallire, come viene riportato l'esito e con quale codice di uscita termina `runtest`.

### Modified Capabilities

- Nessuna. Le tre capacita' esistenti (`compiler-cli`, `interpreter-cli`,
  `interpreter-pcodes`) descrivono il comportamento dei due strumenti, che questa change non
  tocca: `runtest` li **usa** come sono, con la loro command line documentata, e non cambia una
  riga del loro contratto.

## Impact

- **`runtest.c` nuovo**: un solo file, in C, costruito con `cc` come `parser.c` e `asm.c`. Il
  lavoro si riduce a scorrere la directory, costruire i nomi dei file, lanciare `compiler` e
  `interpreter` via `system()`/`popen()` (o `fork`/`exec`), catturare l'output e confrontare due
  file. Nessuna logica del compilatore o dell'interprete viene duplicata.
- **`Makefile`**: target `runtest` (`cc runtest.c -o runtest`) aggiunto a `all` e al `.PHONY`
  se esiste. Nessuna nuova dipendenza di build, nessun target `test`/`clean` (nel Makefile non ci
  sono nemmeno oggi, vedi `AGENTS.md`).
- **`test/`**: nessun fixture esistente viene modificato o rimosso. L'unica coppia completa oggi
  e' `parm_default.cpl` + `parm_default.out`, che diventa il primo test verde; essere verde e'
  compito dell'implementazione, non di questa change. Se servisse almeno un secondo test per
  dimostrare il caso "fallito", il fixture nuovo e' materiale della fase di implementazione, non
  un requisito di questo piano.
- **`AGENTS.md`**: la sezione "Tests" va riscritta da "No runner. Verification is manual" a "il
  runner e' `runtest`", con la convenzione `.cpl`/`.out`, il comando e il significato del codice
  di uscita; la verifica del blocco p-code atteso dentro il sorgente resta manuale e va detta.
  Anche la sezione "Build" va allineata (`make` produce quattro binari, non tre) e l'elenco dei
  build output non tracciati guadagna `runtest`.
- **`todo.md`**: il punto "5. test test test test" e' il tema di questa change; si aggiorna in
  fase di implementazione (spostarlo fra i FATTO o ridurlo a cio' che resta fuori perimetro).
- **Nessuna dipendenza nuova, nessun formato nuovo**: il `.pcd`, l'header `#cpl20.0.1`, `emit.c`,
  `pcodes.c` e la grammatica non sono toccati; non serve rigenerare `cpl2.c`/`cpl2.h` e non c'e'
  nessun `.pcd` da rifare. La command line dei due strumenti resta quella documentata: `runtest`
  non aggiunge flag a `compiler` ne' a `interpreter`.
- **Tempi**: un giro su `test/` esegue i programmi in modalita' normale, non misurata, quindi non
  paga i 30-60s del bench di `fib(34)`. Il costo e' quello della compilazione piu' quella
  dell'esecuzione di ogni fixture coinvolta.
