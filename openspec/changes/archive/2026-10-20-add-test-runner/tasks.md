# Tasks

Vedi `specs/test-suite/spec.md` per il contratto e `design.md` per le decisioni (D1-D8). La
verifica e' manuale in tutto il file: non c'e' ancora un runner automatico, e' proprio questa
change a introdurlo, quindi ogni task si verifica eseguendo `runtest` o i due binari e guardando
stream, file e codice di uscita.

## 1. `runtest.c` e il target di build

- [x] 1.1 Creare `runtest.c` con i due parametri di campagna (`dirTest`, default `test`) e la
  struttura del rapporto (contatori dei superati e dei falliti, nome del test per i fallimenti).
  Commenti e messaggi in italiano, come il resto del progetto. Verifica: `cc -g -O2 runtest.c -o
  runtest` compila senza errori ne' warning e `./runtest` su una directory senza coppie stampa il
  rapporto con zero superati e zero falliti ed esce con 0.
- [x] 1.2 Applicare D8: leggere la directory dei test dalla forma condivisa di `cli.c` (incluso,
  tabella delle opzioni vuota nello stile di `parser.c`, al piu' un posizionale, `test` come
  default). Verifica: `./runtest` usa `test`, `./runtest tmp/prove` usa quella directory, e
  `./runtest a b` segnala su stderr che accetta al piu' una directory ed esce con 1.
- [x] 1.3 Aggiungere al `Makefile` il target `runtest` (`cc runtest.c -o runtest`) e inserirlo in
  `all`. Verifica: `make` costruisce `interpreter`, `compiler`, `asm` e `runtest` e la seconda
  esecuzione non ricompila nulla.

## 2. Raccolta dei test e directory di appoggio

- [x] 2.1 Raccogliere le coppie `.cpl` + `.out` della directory (requisito "Selezione dei test in
  una directory"): un `.cpl` senza `.out` non e' un test e non entra nel rapporto; nessuna
  ricorsione nelle sottodirectory. Verifica: `./runtest` su `test` conta **un** test
  (`parm_default`), e `test_expr1.cpl`, `test_stm1.cpl` e `test_var1.cpl` non compaiono nel
  rapporto.
- [x] 2.2 Rilevare e segnalare l'`.out` senza `.cpl` (requisito "Output atteso senza sorgente"):
  il file viene nominato e conta come fallimento. Verifica: creare `test/orfano.out` in una
  directory di prova, eseguire `./runtest` e controllare che `orfano.out` compaia fra i falliti e
  che l'uscita sia 1; rimuovere il file di prova.
- [x] 2.3 Applicare D5: creare la directory di appoggio con `mkdtemp()` (schema in `TMPDIR`), e
  alla fine del giro rimuoverla con tutto il suo contenuto **anche in caso di fallimenti**.
  Verifica: dopo `./runtest` la directory `test` non contiene file nuovi (`ls test` invariato) e in
  `/tmp` non resta nessuna directory `runtest-*`.

## 3. Compilazione ed esecuzione di un test

- [x] 3.1 Applicare D2 e D5: per ogni test copiare il `.cpl` nella directory di appoggio e
  invocare `./compiler <appoggio>/<nome>.cpl`, cosi' il `.pcd` nasce nella directory di appoggio e
  mai in `test`. Verifica: `./runtest` su `test` non modifica `test/parm_default.pcd` (confrontare
  con `sum`/`md5sum` prima e dopo) e la compilazione di `parm_default` riesce.
- [x] 3.2 Eseguire con `./interpreter <appoggio>/<nome>` catturando il **solo standard output**
  (D4), in modalita' normale e senza opzione di misura (requisito "Nessuna interazione con la
  modalita' di misura"). Verifica: il contenuto catturato per `parm_default` non contiene la
  tabella di nanobench e coincide con l'output di una singola esecuzione a mano
  (`./interpreter test/parm_default.pcd`, che oggi e' gia' identico a `test/parm_default.out`).
- [x] 3.3 Distinguere i due modi di fallire prima del confronto (requisito "Isolamento ed esito di
  ogni test"): compilazione non riuscita e esecuzione con codice di uscita diverso da 0 rendono il
  test fallito senza fermare il giro. Verifica: su una directory di prova con un `.cpl`
  volutamente rotto e un `.cpl` con `not impl!` (per esempio una copia di `test/simple_class.cpl`)
  piu' i loro `.out`, il rapporto nomina entrambi come falliti, il conteggio e' corretto e gli
  altri test vengono eseguiti comunque.

## 4. Confronto ed esito

- [x] 4.1 Applicare D3: confrontare l'output catturato con il `.out` **byte per byte**, senza
  normalizzazioni, e mostrare la differenza nel rapporto quando il test fallisce. Verifica: il
  test `parm_default` e' superato (i due file sono identici: `diff` a mano non stampa nulla), e un
  `.out` in cui si toglie un carattere o si aggiunge una riga vuota in coda rende il test fallito
  con la differenza visibile nel rapporto.
- [x] 4.2 Applicare D6: contare i superati e i falliti, nominare ogni fallito su stderr, e uscire
  con **0 se e solo se non ci sono fallimenti**. Verifica: `./runtest` con tutti i test verdi esce
  con 0; `./runtest` con un test rosso esce con 1; `./runtest > /dev/null` mantiene il codice di
  uscita.

## 5. Verifica degli scenari della spec

- [x] 5.1 Requisiti su selezione, directory e directory vuota: `./runtest` usa `test`,
  `./runtest tmp/prove` usa quella directory, e `./runtest` su una directory senza coppie esce con
  0. Verifica: i tre comandi hanno l'esito atteso e nessuno esegue i test dell'altra directory.
- [x] 5.2 Requisiti su compilazione, esecuzione e cattura: il `.pcd` eseguito e' quello appena
  prodotto dal compilatore e non un `.pcd` preesistente. Verifica: compilare a mano una variante
  diversa di `parm_default` in modo che il `.pcd` preesistente nella directory dei test abbia un
  output diverso dal `.cpl` (rimuovere il `.out`, cambiare un `?`, ricompilare, ripristinare il
  `.cpl` e il `.out`): `./runtest` deve comunque superare il test, perche' usa il `.pcd` che
  costruisce lui.
- [x] 5.3 Requisiti su isolamento e ripetibilita': due giri consecutivi danno lo stesso esito, il
  secondo non trova file lasciati dal primo, e la directory dei test resta invariata. Verifica:
  `./runtest && ./runtest` ha lo stesso rapporto nei due giri e `ls test` e' invariato prima e
  dopo.
- [x] 5.4 Requisito "Nessuna interazione con la modalita' di misura" e regressione sui binari:
  `./runtest` non tocca `interpreter` ne' `compiler`. Verifica: `git diff --stat` non elenca
  `interpreter.cpp`, `parser.c`, `emit.c`, `pcodes.c`, `asm.c` ne' i file della grammatica
  (`cpl2.gram`, `cpl2.c`, `cpl2.h`).

## 6. Documentazione

- [x] 6.1 Riscrivere la sezione "Tests" di `AGENTS.md`: il runner e' `runtest`, la convenzione e'
  `.cpl` + `.out` con lo stesso nome di base, il confronto e' byte per byte sullo standard output,
  il codice di uscita e' 0 tutto verde / 1 almeno un fallimento. Verifica: la sezione non dice piu'
  "No runner. Verification is manual" e il comando documentato (`make && ./runtest`) funziona come
  scritto.
- [x] 6.2 Correggere la frase di `AGENTS.md` secondo cui `main()` stampa **sempre** il rapporto del
  GC dopo l'esecuzione in entrambe le modalita': il rapporto e' dietro `--gc`/`-g` e senza
  quell'opzione lo standard output e' solo quello del programma (design D4). Allineare la stessa
  affermazione nella sezione "Running a program" e, se ripete l'errore, nella descrizione della
  misura. Verifica: `grep -n 'GC report' AGENTS.md` non trova piu' frasi che promettono il rapporto
  senza l'opzione.
- [x] 6.3 Allineare la sezione "Build" di `AGENTS.md`: `make` produce **quattro** binari (`runtest`
  compreso) e l'elenco dei build output non tracciati guadagna `runtest`. Verifica: la sezione
  elenca il target `runtest` e `git status` non propone di tracciare il binario.
- [x] 6.4 Dire in `AGENTS.md` a cosa serve ciascuno dei due "sistemi di test" (design D7): le
  fixture di `test/` si eseguono con `runtest`, il ramo `--test`/`-t` dell'interprete e'
  un esercizio di debug con una sequenza fissa di prove, e il confronto con il blocco p-code atteso
  dentro il sorgente resta manuale con `./asm < x.pcd`. Verifica: la sezione "Tests" nomina
  entrambe le cose e non lascia credere che `--test` sia la suite.
- [x] 6.5 Aggiornare `todo.md`: il punto "5. test test test test" e' il tema di questa change.
  Verifica: il punto non compare piu' fra i DA FARE nella forma attuale e cio' che resta fuori
  perimetro (formato di test piu' ricco, confronto dei listing p-code) e' nominato, oppure il punto
  e' spostato fra i FATTO.

## Workflow follow-up

- Dopo la revisione: `openspec archive add-test-runner` per consolidare il requisito in
  `openspec/specs/test-suite/spec.md`.
- Dopo l'archivio, valutare con l'utente se serve un secondo fixture completo in `test/` con il suo
  `.out` (per esempio un programma con parametri di default piu' articolato di quelli gia' coperti
  da `parm_default`, oppure il primo caso di un p-code che oggi non ha nessun test): il piano non
  lo richiede, ma una suite con un solo test non dimostra il caso "fallito" in modo naturale, e
  oggi il rosso si verifica solo con una directory di prova.
- Resta fuori perimetro, e vale la pena nominarlo quando si archivia: il difetto noto di
  `benchPcd()` (esce con l'esito inverso di `runPcd()`), i p-code non implementati (`not impl!`) e
  il confronto dei listing p-code con `./asm < x.pcd`, che non entra in `runtest`.
- Valutare se il ramo `--test`/`-t` di `interpreter.cpp` vada eliminato o fuso con `runtest` in una
  change dedicata: oggi convive e la sua descrizione in `--help` va tenuta distinta dalla suite.
