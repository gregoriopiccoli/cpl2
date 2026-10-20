# Tasks

## 1. Modulo condiviso `cli.c`

- [x] 1.1 Creare `cli.c` con `cliOpzione`, `cliScansione`, i due tipi `CLI_OPZIONE` e
  `CLI_VALORE` e il ciclo di scansione secondo D2, D3, D4 e D9: riconoscimento del nome lungo
  e del nome corto, opzioni ovunque, `--` che termina, `-` da solo escluso, valore preso dal
  `=` o dall'argomento successivo, annotazione dell'opzione sconosciuta con il suo eventuale
  valore. Il modulo non alloca e non libera: l'array dei posizionali e la tabella li fornisce
  il chiamante. Verifica: `cc -c -o /dev/null cli.c` compila senza diagnosiche.

- [x] 1.2 Aggiungere in `cli.c` il ciclo che stampa le opzioni riconosciute passando per i campi
  `lungo`, `corto` e `tipo` della tabella (D8), cosi' che l'elenco dell'uso non sia scritto a
  mano. Verifica: `cc -c -o /dev/null cli.c` compila senza diagnosiche e il ciclo stampa una
  riga per ogni riga della tabella.

- [x] 1.3 Elencare `cli.c` fra le dipendenze dei target `compiler` e `interpreter` nel
  `Makefile`, senza aggiungere un target nuovo e senza un passo di link. Verifica: dopo aver
  toccato `cli.c`, `make` ricompila entrambi i binari.

- [x] 1.4 Scrivere `cli_test.c`, file a se' stante con il proprio `main()` e non citato dal
  Makefile, sul pattern di `gc_test.cpp`. Deve coprire le tre forme (corta, lunga, con valore),
  `-b=on`, `--out=x`, `-o x`, il terminatore `--`, `-` da solo, opzione sconosciuta e valore
  mancante su un'opzione che ne vuole uno. Verifica: `cc -g -O2 cli_test.c -o cli_test && ./cli_test`
  stampa tutte le verifiche come superate e termina con 0.

- [x] 1.5 Compilare lo stesso `cli_test.c` come C++ per provare che `cli.c` e' valido nei due
  linguaggi, che e' il vincolo di D1. Verifica: `c++ -g -O2 -x c++ cli_test.c -o cli_test_cpp && ./cli_test_cpp`
  termina con 0 e con lo stesso esito di 1.4.

## 2. Compilatore

- [x] 2.1 In `parser.c`, includere `cli.c`, dichiarare una tabella vuota (D6) e sostituire il
  ciclo su `argv[1..argc-1]` con la scansione e il ciclo sui posizionali. Il ramo `argc<2` che
  legge da stdin e scrive su stdout resta intatto. Verifica: `compiler fib.cpl` produce
  `fib.pcd` e `compiler < fib.cpl > fib.pcd` produce lo stesso p-code di prima.

- [x] 2.2 Trattare il posizionale `-` come lo standard input, riusando il ramo gia' esistente
  invece di aprire un file di nome `-` (D5). Verifica: `cat fib.cpl | compiler -` produce lo
  stesso p-code di `compiler < fib.cpl`.

- [x] 2.3 Riconoscere l'opzione sconosciuta: `diag` che la nomina, nessuna traduzione e
  `return 1`, senza proseguire con i sorgenti restanti (vedi lo scenario "L'opzione sconosciuta
  interrompe gli altri sorgenti"). Verifica: `compiler -x fib.cpl` nomina `-x` su stderr, non
  produce `fib.pcd` ed esce con 1; `compiler a.cpl -x b.cpl` non produce ne' `a.pcd` ne' `b.pcd`.

- [x] 2.4 Verificare che gli scenari di `compiler-cli` non toccati da questa change reggano:
  piu' sorgenti tradotti nell'ordine, risoluzione implicita di `.cpl`, uscita pari al totale
  degli errori, e un sorgente non apribile che non interrompe gli altri. Verifica:
  `compiler fib.cpl primo.cpl test/simple_class.cpl` produce tre p-code distinti;
  `compiler fib` in una directory dove esiste solo `fib.cpl` scrive `fib.pcd`;
  `compiler a.cpl mancante.cpl b.cpl` produce `a.pcd` e `b.pcd` e termina con 1.

## 3. Interprete

- [x] 3.1 In `interpreter.cpp`, includere `cli.c` e dichiarare la tabella con `lungo="bench"`
  e `corto='b'`, piu' il confronto esatto per `-bench` che resta nel `main()` (D7). Verifica:
  `./interpreter -bench primo.pcd`, `./interpreter --bench primo.pcd` e `./interpreter -b primo.pcd`
  misurano tutti e tre lo stesso file.

- [x] 3.2 Validare il numero di percorsi sulla lista dei posizionali: zero percorsi oppure piu'
  di uno producono le istruzioni d'uso su stderr e codice di uscita 1, senza eseguire ne'
  misurare nulla. Verifica: `./interpreter` e `./interpreter primo.pcd fib.pcd` escono con 1 e
  non producono output del programma.

- [x] 3.3 Segnalare l'opzione sconosciuta nominandola, con le istruzioni d'uso su stderr e
  codice di uscita 1. Verifica: `./interpreter -h`, `./interpreter -z fib.pcd` e
  `./interpreter -h fib.pcd` escono tutti con 1 e nessuno di loro tenta di aprire un file.

- [x] 3.4 Onorare il terminatore `--` nella risoluzione del percorso (D3). Verifica:
  `./interpreter -- -h` tenta di caricare `-h.pcd`; `./interpreter ./-bench` carica
  `./-bench.pcd`.

- [x] 3.5 Sostituire le righe d'uso scritte a mano con il ciclo che legge la tabella (D8),
  mantenendo la riga sulla derivazione di `.pcd` e nominando il terminatore `--`. Verifica:
  l'uso stampato da `./interpreter` riporta `--bench`, `-b` e `-bench`, e su stderr e non su
  stdout.

- [x] 3.6 Verificare che la misura sia indifferente alla posizione dell'opzione e che la
  ripetizione non sia un errore. Verifica: `./interpreter primo.pcd -b` misura `primo.pcd`;
  `./interpreter --bench -b primo.pcd` misura una sola volta senza diagnostiche.

- [x] 3.7 Verificare che il rapporto del GC, la destinazione di output e diagnostica e i codici
  di uscita per file caricabile e non caricabile restino quelli di prima, e che il percorso
  derivato sia quello mostrato nella tabella di misura. Verifica: `./interpreter primo.pcd`
  esce con 0 e stampa il rapporto del GC su stdout; la tabella di `./interpreter -b primo.pcd`
  riporta il percorso derivato `primo.pcd`.

## 4. Documentazione

- [x] 4.1 Riallineare `AGENTS.md`: la sezione che descrive `-bench` come unico flag riconosciuto
  solo in prima posizione e come stringa esatta, e la regola per cui `-h` e un percorso vanno
  sostituite con le nuove regole di riconoscimento delle opzioni. Verifica: in `AGENTS.md`
  non resta piu' nessuna delle due affermazioni, e le forme `--bench` e `-b` sono nominate
  accanto a `-bench`.

- [x] 4.2 Riallineare `AGENTS.md` anche nella sezione "Wiring": `cli.c` e `cli_test.c` vanno
  elencati dove oggi si descrive l'inclusione di `pcodes.c`, `emit.c` e `gc.cpp`, con la nota
  che `cli.c` e' incluso nelle due unita' di traduzione e `cli_test.c` sta fuori dal Makefile
  come `gc_test.cpp`. Verifica: la sezione elenca `cli.c` accanto agli altri file condivisi e
  `cli_test.c` accanto a `gc_test.cpp`.

## 5. Verifica finale

- [x] 5.1 Ricostruire da capo e percorrere il ciclo completo compilatore-interprete sui
  sorgenti di prova, perche' la change tocca i due binari insieme e nessuna verifica dei gruppi
  precedenti li esercita in sequenza. Verifica: `make` da pulito compila `interpreter`,
  `compiler` e `asm` senza diagnosiche; poi `compiler primo.cpl && ./interpreter primo.pcd`
  produce l'output del programma ed esce con 0, e `./compiler test/simple_class.cpl` genera
  `test/simple_class.pcd` il cui listato `./asm < test/simple_class.pcd` resta identico a
  quello del fixture.

## Workflow follow-up

- Archiviare la change dopo che i task sono completati e la revisione e' soddisfatta.
- Verificare il risultato archiviato: `openspec/specs/interpreter-cli/spec.md` e
  `openspec/specs/compiler-cli/spec.md` devono descrivere il riconoscimento delle opzioni e
  piu' gli scenari sulla validazione per forma esatta.
- Aprire una change separata per i due difetti fuori perimetro indicati in `proposal.md`: il
  codice di uscita invertito di `benchPcd()` e la corruzione dell'heap nel ciclo di misura.