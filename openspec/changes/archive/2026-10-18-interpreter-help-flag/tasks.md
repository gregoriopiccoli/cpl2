# Tasks

Vedi `specs/interpreter-cli/spec.md` per il contratto e `design.md` per le decisioni (D1-D9).
Verifica manuale in tutto il file, secondo D6: non c'e' un runner di test (`AGENTS.md`), ogni
scenario si verifica eseguendo `./interpreter ...` e guardando stream e codice di uscita. Usare
`primo.pcd` e non `fib.pcd`: il bench di `fib(34)` costa 30-60s.

## 1. Grafie delle opzioni in `interpreter.cpp`

- [x] 1.1 Applicare D7: nella riga dell'opzione di misura in `tabOpzioni` mettere `NULL` nel campo
  `alias` (era `"bench"`), lasciando `lungo="bench"` e `corto='b'`, e riscrivere il commento `D7`
  sopra la tabella perche' non nomini piu' tre grafie e spieghi un alias che non esiste piu'. Il
  testo della descrizione della riga resta invariato. Verifica: `make interpreter` compila senza
  errori e il commento sopra `tabOpzioni` non contiene piu' la parola "alias".
- [x] 1.2 Applicare D2: cambiare `uso()` in `uso(FILE* out)`, sostituire i tre `cerr << ... <<`
  con `fprintf(out,...)` e passare `out` a `cliOpzioni(tabOpzioni,2,out)`. Le due righe finali
  sulla derivazione `.pcd` e sul terminatore `--` restano identiche, cambiano solo lo stream
  (D5). Verifica: `make interpreter` compila e il corpo di `uso()` non contiene piu' alcun
  riferimento a `cerr`.
- [x] 1.3 Applicare D3: dichiarare `static int helpOpzione=0;` accanto a `benchOpzione` e
  aggiungere a `tabOpzioni` la riga `{"help",'h',NULL,"stampa queste istruzioni d'uso",CLI_OPZIONE,&helpOpzione}`,
  passando la dimensione da 1 a 2 in `tabOpzioni[2]` e in ogni chiamata a `cliScansiona()` e
  `cliOpzioni()`. Verifica: `grep -n 'tabOpzioni' interpreter.cpp` non mostra piu' nessun `1`
  letterale come dimensione della tabella, e `make interpreter` compila.
- [x] 1.4 Applicare D1: spezzare l'unico `if (sc.errore!=CLI_ERRORE_NESSUNO||nPercorsi!=1)` in
  tre controlli in sequenza — (a) `sc.errore!=CLI_ERRORE_NESSUNO` con lo stesso `switch` sui
  `CLI_ERRORE_*` invariato, `uso(stderr)` e `return 1`; (b) `helpOpzione` con `uso(stdout)` e
  `return 0`; (c) `nPercorsi!=1` con `uso(stderr)` e `return 1` — azzerando `helpOpzione=0` dove
  oggi viene azzerato `benchOpzione=0`. Verifica: `make interpreter` compila e
  `./interpreter primo.pcd` continua a stampare l'output del programma e il rapporto del GC,
  terminando con 0.
- [x] 1.5 Applicare D4: verificare che il ramo dell'help stia prima di `pcodePathFor()` e non
  tocchi nessun ramo che carichi un file. Verifica: `./interpreter -h primo.pcd` stampa l'help
  ed esce 0 senza la riga `--- final status,locked objs ---` e senza alcun output di programma,
  e non crea nessun file (`ls *.pcd` invariato rispetto a prima).

## 2. Commenti di `cli.c`

- [x] 2.1 Applicare D8: correggere il commento dentro `cliTrova()` che usa `-bench` come esempio
  di alias, e il commento nella testata di `cli.c` che descrive l'alias come una parola intera
  con un solo trattino, senza toccare nessuna riga di codice. Verifica:
  `git diff cli.c` mostra solo righe di commento, e `grep -n -- '-bench' cli.c` non trova piu'
  nulla.
- [x] 2.2 Applicare D8: verificare che i due esempi nuovi non siano nomi di flag di questo
  progetto, cosi' il commento non torna a mentire alla prossima modifica. Verifica: nessuna delle
  parole usate come esempio compare in `tabOpzioni` in `interpreter.cpp`.
- [x] 2.3 Verificare che `cli_test.c` non abbia bisogno di modifiche: la sua tabella dichiara
  ancora un flag con alias, quindi continua a provare la forma `alias` del modulo. Verifica:
  `cc -g -O2 cli_test.c -o cli_test && ./cli_test` termina senza errori e `git diff --stat
  cli_test.c` non produce output.

## 3. Documentazione

- [x] 3.1 Aggiornare `AGENTS.md`, sezione "Running a programma — one path, plus options": "The
  only flag is the benchmark one, written `-bench`, `-b` or `--bench`" diventa due opzioni con due
  grafie ciascuna (`-b`/`--bench` e `-h`/`--help`), con le due uscite dell'help (`./interpreter
  --help` su stdout con codice 0, uso su stderr con codice 1 quando la command line e' sbagliata) e
  l'esempio `./interpreter -bench mio.pcd` corretto in `./interpreter -b mio.pcd`. Verifica:
  `grep -n -- '-bench\|--help\| -h' AGENTS.md` non elenca piu' `-bench` come grafia accettata e
  nomina `-h`/`--help` come opzione riconosciuta.
- [x] 3.2 Correggere nella stessa sezione la frase `./interpreter -h` says so rather than opening
  a file named `-h`, che dopo questa change e' falsa, e aggiungere che `-h` con un percorso stampa
  l'help e ignora il percorso; allineare anche la sezione "Wiring" (`interpreter.cpp` has
  `--bench`/`-b`/`-bench`) e il paragrafo sul difetto di `benchPcd()` che nomina `-bench` nella
  descrizione della sintassi dei suoi due comandi. Verifica:
  `grep -n 'named .-h\|/ -bench\|/-bench' AGENTS.md` non trova piu' frasi che descrivono `-h` come
  opzione sconosciuta o `-bench` come grafia valida.
- [x] 3.3 Verificare che `todo.md` resti da parte: i DA FARE sono 3, 4 e 5 e nessuno riguarda la
  command line. Verifica: `git diff --stat todo.md` non produce output.

## 4. Verifica degli scenari della spec

- [x] 4.1 Requisito "Opzione di help `-h` e `--help`": `./interpreter -h` e `./interpreter
  --help` stampano l'uso su stdout ed escono con 0; `./interpreter -h primo.pcd` e
  `./interpreter primo.pcd -h` stampano l'help ed escono con 0 senza eseguire `primo.pcd`;
  `./interpreter -h -b primo.pcd` stampa l'help ed esce 0 senza tabella nanobench;
  `./interpreter -h --help` non segnala errori. Verifica: i sei comandi hanno codice 0 e nessuno
  stampa l'output di `primo.pcd` ne' una tabella nanobench.
- [x] 4.2 Stesso requisito, bordo e precedenza: `./interpreter -h=x` segnala su stderr che `-h`
  non accetta un valore, con uso su stderr e codice 1, e non stampa nulla su stdout;
  `./interpreter -h -z primo.pcd` segnala `-z` come opzione sconosciuta su stderr, con uso e
  codice 1, senza help su stdout (D1); `./interpreter -- -h` tenta di caricare `-h.pcd` e ne
  segnala l'assenza su stderr. Verifica: i tre comandi hanno codice 1, i primi due distinguono il
  messaggio dell'errore dall'help, e `./interpreter -- -h` nomina `-h.pcd` nel messaggio.
- [x] 4.3 Requisiti sulla misura: `./interpreter -bench primo.pcd` segnala `-bench` come opzione
  sconosciuta su stderr, con uso e codice 1, e non stampa alcuna tabella; `./interpreter -b
  primo.pcd` e `./interpreter --bench primo.pcd` misurano ed escono con 0; `./interpreter -b`
  e `./interpreter -b primo.pcd fib.pcd` escono con 1 senza misurare nulla; `./interpreter
  primo.pcd -b` misura `primo.pcd`; `./interpreter ./-b` carica `./-b.pcd`. Verifica: i sette
  comandi hanno il codice di uscita atteso e nessuno dei due casi sconosciuti produce una tabella
  nanobench. **Correzione in sede di verifica:** i tre comandi che misurano producono la tabella
  su stdout (verificato) ma escono con **1**, non 0, per il difetto noto di `benchPcd()` descritto
  sotto in "Workflow follow-up" e fuori perimetro: il codice di uscita di questa change e'
  confrontato solo con la tabella e con i messaggi, non con l'inversione preesistente.
- [x] 4.4 Requisito modificato "Le istruzioni d'uso elencano le opzioni dichiarate":
  `./interpreter --help 2>/dev/null` contiene per intero l'uso, con le righe `--help, -h` e
  `--bench, -b` e senza `-bench` come terza grafia, piu' la riga sulla derivazione `.pcd` e quella
  sul terminatore `--`; `./interpreter 2>/dev/null` non stampa nulla; `./interpreter -z primo.pcd
  2>/dev/null` non stampa nulla; `./interpreter primo.pcd fib.pcd` segnala la condizione su stderr
  ed esce con 1. Verifica: sui tre casi di errore lo standard output non contiene le istruzioni
  d'uso ne' la segnalazione (l'unica cosa che ci finisce e' il rapporto del GC stampato da `~GC()`
  all'uscita del processo, che c'era gia' prima di questa change e non e' della sua portata),
  mentre il primo contiene tutte e quattro le righe attese e non la terza grafia.
- [x] 4.5 Regressione sui requisiti non toccati: `./interpreter primo.pcd` esce con 0 ed esegue
  il programma; `./interpreter` esce con 1 senza eseguire nulla; `./interpreter -b non_esiste` e
  `./interpreter -b primo.cpl` riportano su stderr le rispettive diagnostiche e finiscono con 1
  senza tabella; `./interpreter -- --bench` carica il file di nome `--bench.pcd`. Verifica: i
  cinque comandi hanno il codice di uscita atteso e i messaggi distinguono "non apribile" da
  "non e' un p-code". **Correzione in sede di verifica:** i due casi non caricabili escono con
  **0**, non 1, per la stessa inversione di `benchPcd()` documentata sotto in "Workflow
  follow-up": le diagnostiche sono corrette e distinte e nessuna tabella viene prodotta, che e'
  cio' che la spec richiede; il codice di uscita resta il difetto preesistente e fuori perimetro.
  Il diff di questa change non tocca `benchPcd()` ne' `runPcd()`.
- [x] 4.6 Controllo finale del diff: rileggere `git diff` e verificare che tocchi solo
  `interpreter.cpp`, `cli.c` (commenti), `cli_test.c` (niente) e `AGENTS.md`, che `parser.c`,
  `emit.c`, `pcodes.c` e `asm.c` siano immutati e che nessun file della grammatica (`cpl2.gram`,
  `cpl2.c`, `cpl2.h`) sia stato toccato; rileggere i tre corpi di scenario citati in D9 nella spec
  risultante dall'archivio e controllare che dicano `-b` e `--bench`. Verifica:
  `git diff --stat` elenca i tre file attesi e i `.pcd` prodotti dalla verifica restano non
  tracciati (build output, vedi `AGENTS.md`: mai `git add -A`). **Correzione in sede di verifica:**
  `cli.c` e `cli_test.c` sono file **non tracciati** in questo repository (arrivano dalla change
  `shared-cli-flags`, mai committata), quindi `git diff --stat` elenca solo `AGENTS.md` e
  `interpreter.cpp`: i tre file attesi diventano due tracciati piu' `cli.c`, controllato a mano
  (nessuna riga di codice cambiata, due blocchi di commento, zero occorrenze di `-bench`, e
  `make` + `cli_test` verdi).

## Workflow follow-up

- Archiviare la change dopo che i task sono completati e il diff e' stato rivisto. Il delta su
  `interpreter-cli` modifica cinque requisiti della spec principale ("Selezione del file da
  eseguire", "Le istruzioni d'uso elencano le opzioni dichiarate", "Riconoscimento delle
  opzioni", "Misura di un p-code con il flag `-bench`", "Esito del caricamento in modalita'
  bench") e aggiunge il requisito sull'opzione di help; l'archivio sostituisce i cinque blocchi e
  accoda il nuovo.
- Dopo l'archivio restano tre titoli che nominano `-bench` e che OpenSpec non permette di
  rinominare dentro un blocco `MODIFIED` (vedi D9 in `design.md`): due scenari di "Selezione del
  file da eseguire" e uno di "Le istruzioni d'uso elencano le opzioni dichiarate", piu' il titolo
  del requisito sulla misura. Se disturbano, si sistema in una change a parte che rimuove e
  ricrea quei requisiti per intero.
- Difetto noto, fuori perimetro: `benchPcd()` termina con `return r?1:0` mentre `runPcd()` usa
  `return r?0:1`, quindi la misura esce con 1 su file caricabile e con 0 su file non caricabile.
  I task 4.3 e 4.5 lo devono sapere per non leggere l'inversione come una regressione di questa
  change: la correzione e' una change separata, come dice `AGENTS.md`.
- Dopo l'archivio, valutare con l'utente se `-h`/`--help` sul compilatore merita una change
  propria: `compiler-cli` continua a descrivere `./compiler -h fib.cpl` come opzione sconosciuta
  e la sua tabella resta intenzionalmente vuota.