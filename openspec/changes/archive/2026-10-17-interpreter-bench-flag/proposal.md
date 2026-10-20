# Proposal

## Why

Misurare un p-code oggi costa un ciclo di edit-compile: `main()` chiama `runPcd()`, e per
misurare un file diverso l'unica via e' commentare la chiamata, scrivere
`bench("fib.pcd")` al suo posto e rilanciare `make interpreter` (~5s), ogni volta. La change
`interpreter-cli-input-file` ha reso `runPcd()` raggiungibile dalla riga di comando ma ha
lasciato `benchPcd()` irraggiungibile, scegliendo esplicitamente di non introdurre flag
(D5): il banco di misura dello sviluppatore e' quindi peggio di prima, perche' il file da
misurare si puo' scegliere senza toccare il sorgente solo nel caso normale, non in quello che
serve misurare.

`./interpreter -bench fib.pcd` chiude il cerchio: stessa selezione del file di
`./interpreter fib.pcd`, con la misura nanobench al posto della singola esecuzione.

## What Changes

- **BREAKING** `./interpreter -bench <file>` misura il file invece di eseguirlo una volta:
  il percorso viene derivato con la stessa regola di oggi (`pcodePathFor()`, nessuna
  estensione -> accodo `.pcd`), caricato e poi eseguito dentro il ciclo nanobench da
  `benchPcd()`. Il flag sta **prima** del percorso ed e' l'unico riconosciuto.
- **BREAKING** La regola "un argomento che inizia con `-` e' un percorso, non un'opzione"
  della spec `interpreter-cli` ha un'eccezione: `-bench`. Tutti gli altri argomenti che
  iniziano con `-` restano percorsi, quindi `./interpreter -h` continua a tentare di aprire
  il file di nome `-h`, e anche `--bench` (doppio trattino) e' un percorso, non il flag.
- `./interpreter -bench` senza il percorso e' un errore: come senza argomenti, l'interprete
  stampa le istruzioni d'uso su stderr e termina con codice 1, senza misurare nulla.
- `./interpreter -bench primo.pcd fib.pcd` (flag piu' di un percorso) resta un errore, come
  oggi con piu' di un argomento: uso su stderr, codice 1.
- **BREAKING** In modalita' bench il codice di uscita non e' piu' sempre 0: il file viene
  caricato una volta prima di misurare e il suo esito decide l'uscita, 0 se caricabile e 1
  altrimenti, con le stesse diagnostiche su stderr e le stesse regole di derivazione del
  percorso della modalita' normale. Un file che non si puo' caricare non produce nessuna
  tabella nanobench: la misura non avrebbe significato.
- La tabella nanobench e il rapporto del GC stampato dopo l'esecuzione restano quelli di oggi,
  su stdout, anche in modalita' bench.
- Fuori perimetro: `-h`/`--help`, `-` come sinonimo di stdin, piu' flag combinabili, e
  qualunque altra opzione. `-bench` e' l'unica e si introduce perche' il bisogno e' concreto;
  le altre restano una questione separata, come gia' deciso in `compiler-cli`.
- Fuori perimetro: i bench in C++ (`bench_cc()`, `bench_fib_cc()`, `test_cc()`,
  `test_fib_cc()`), che non hanno un p-code da indicare. Restano nel sorgente e irraggiungibili
  dalla riga di comando, come prima di questa change.
- Fuori perimetro: taratura di nanobench (epoche, durata, `run(...)->iterations(...)`) e
  differenziazione fra programmi diversi. `./interpreter -bench fib.pcd` e
  `./interpreter -bench primo.pcd` danno la stessa tabella di oggi, con il nome del file.

## Capabilities

### New Capabilities
- Nessuna.

### Modified Capabilities
- `interpreter-cli`: il requisito "Selezione del file da eseguire" non ammette piu' un solo
  argomento e nega l'esistenza di opzioni; diventa ammettere il solo flag `-bench` prima del
  percorso, mantenendo il trattamento da percorso per ogni altro argomento con `-`. Nuovo
  requisito per la modalita' bench: cosa misura, come deriva il codice di uscita e cosa
  succede senza percorso.

## Impact

- `interpreter.cpp`: `main()` smette di assumere `argc==2` e distingue la forma
  `-bench <percorso>`; `benchPcd()` smette di restituire la costante 0 e propaga
  l'esito del caricamento. Le funzioni `benchPcd()`, `bench_cc()`, `bench_fib_cc()`,
  `test_cc()`, `test_fib_cc()` e `runPcd()` restano tutte: nessuna viene cancellata, e
  `benchPcd()` continua a richiamare `runPcd()` dentro la lambda nanobench.
- `interpreter.cpp`: `pcodePathFor()` non cambia, e non cambia nessuna diagnostica di
  `pcodeProgram::loadPcd()`: il messaggio "non apribile" che la modalita' bench produce
  e' lo stesso che produce gia' oggi la modalita' normale.
- `AGENTS.md`: la sezione "Running a programma" afferma che l'interprete "takes exactly one
  argument", che "there are no flags", che `-h` viene trattato come percorso e che i bench
  richiedono ancora di modificare `main()`. Le prime tre restano vere con l'eccezione di
  `-bench`, la quarta e' falsa: la sezione va riscritta.
- `todo.md`: porta ancora "2. specificare i file da eseguire in cli" fra i DA FARE, ma la
  change `interpreter-cli-input-file` lo ha gia' chiuso. Il task di allineamento e' una
  decisione di questa change, non un prerequisito.
- Formato del `.pcd`, header `#cpl20.0.1`, `parser.c`, `emit.c`, `pcodes.c`, `asm.c` e la
  grammatica non sono toccati: nessuna rigenerazione di `cpl2.c`/`cpl2.h`, nessun `.pcd`
  da rifare.
- Tempi: `./interpreter -bench fib.pcd` costa i 30-60s di nanobench che il bench gia' costa,
  non di piu'. E' il motivo per cui non e' un problema che il flag non abbia una forma breve.