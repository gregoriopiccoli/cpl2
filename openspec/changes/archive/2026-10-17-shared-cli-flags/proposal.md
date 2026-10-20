# Proposal

## Why

`-bench` e' l'unica opzione che esiste, ed e' cablata a mano in `main()` con un controllo di
forma (`argc==3 && argv[1]=="-bench"`). Con un secondo flag quella forma non scala: i casi di
`argc` crescono a una combinazione per ogni opzione, e un flag che prende un valore aggiunge una
categoria nuova. I due binari hanno inoltre due convenzioni opposte nate in due change diverse:
`compiler` tratta ogni argomento come sorgente e non ha opzioni, `interpreter` accetta due forme
esatte e considera ogni altro nome con `-` un percorso. Ogni opzione futura andrebbe quindi
riscritta due volte, in due stili diversi, con il rischio che i due elenchi divergano.

Serve un unico modo di dichiarare e riconoscere le opzioni, condiviso dai due strumenti e capace
di gestire le tre forme che servono: nome lungo, nome corto, valore.

## What Changes

- **BREAKING** — Le opzioni non sono piu' riconosciute solo in prima posizione e solo come stringa
  esatta: sono riconosciute in qualunque posizione della command line, e `--` termina
  l'interpretazione delle opzioni.
- **BREAKING** — Un argomento che inizia con `-` ed e' diverso da `-` semplice non e' piu' un
  percorso: e' un'opzione. Se non corrisponde a nessuna opzione dichiarata, lo strumento lo
  segnala come opzione sconosciuta. Un percorso che inizia con `-` si indica con `./` o dopo `--`.
- **BREAKING** — `-bench` non e' piu' l'unica forma del flag di misura: `--bench` e `-b` sono
  equivalenti. `-bench` continua a essere accettata (vedi Assunzioni).
- Un modulo nuovo `cli.c` contiene il ciclo di scansione condiviso dai due strumenti: riconosce
  opzioni lunghe (`--bench`), corte (`-b`), opzioni con valore nella stessa stringa
  (`--jit=on`, `-jit=on`) e `-` come argomento letterale, e separa le opzioni dai posizionali.
- Ogni strumento dichiara le proprie opzioni in una tabella locale; `cli.c` non conosce il
  vocabolario di nessuno dei due e non viene modificato quando uno dei due aggiunge un flag.
- Le istruzioni d'uso dell'interprete elencano le opzioni a partire dalla stessa tabella, cosi' un
  flag nuovo compare nell'uso senza che il testo venga riscritto a mano.
- Il compilatore smette di trattare ogni argomento come sorgente: accetta opzioni, onora `--`, e
  segnale un'opzione sconosciuta senza tradurre nulla.
- Un file di verifica a se' stante `cli_test.c`, con il proprio `main()`, esercita il ciclo sulle
  tre forme delle opzioni (corta, lunga, con valore) e sul terminatore `--`, following the same
  pattern di `gc_test.cpp`: non entra nel Makefile e si compila a parte.

### Fuori perimetro

- **Il codice di uscita di `-bench` e' sbagliato rispetto alla spec oggi**: `benchPcd()` finisce
  con `return r?1:0` mentre `runPcd()` usa `return r?0:1` (`interpreter.cpp:1755` e
  `interpreter.cpp:1744`), quindi `./interpreter -bench primo.pcd` esce con 1 e
  `./interpreter -bench non_esiste` esce con 0, l'opposto di quanto promette
  `interpreter-cli`. Non e' un problema di scansione degli argomenti e resta un difetto: questa
  change non lo corregge e non lo tocca, perche' mescolerebbe al delta della sintassi degli argomenti
  un requisito il cui valore corretto non sta in discussione. Va in una change separata.
- **Il corrompimento dell'heap in `-bench`**: `benchPcd()` riusa un solo `pcodeProgram` e il GC
  raccoglie fra un'iterazione e l'altra; `secondo.pcd` termina con `corrupted size vs. prev_size`
  e SIGABRT. Anche questo e' runtime/ownership, non sintassi degli argomenti, e resta fuori.
- **`-h` / `--help` non vengono introdotti.** `-h` diventa semplicemente un'opzione sconosciuta,
  perche' aggiungere un flag per l'aiuto non serve a costruire il meccanismo. Aggiungerlo dopo e'
  una riga di tabella.
- **Nessuna operazione nuova per il compilatore.** La tabella del compilatore resta vuota: la
  change realizza il meccanismo, non un flag di output. `-o`, `--asm` o simili sono change a
  se' stante, con il loro delta di derivazione del nome del file p-code.
- Non si toccano la grammatica, `cpl2.c`/`cpl2.h`, `emit.c`, il formato del `.pcd`, l'header
  `#cpl20.0.1`, ne' `asm`.

### Assunzioni

- **`-bench` resta accettata** oltre a `--bench` e `-b`. `AGENTS.md` la documenta e diversi
  scenari di `interpreter-cli` la nominano: toglierla romperebbe un contratto gia' scritto senza
  guadagno. Sono tre grafie della stessa opzione, non tre opzioni.
- **Nessun valore di opzione viene interpretato dal modulo condiviso.** `cli.c` consegna la
  stringa grezza e lascia che ogni strumento la converta: `-n 100` e `-n=100` producono la stessa
  stringa, e la validazione del contenuto resta dove sono le opzioni che la usano.

## Capabilities

### New Capabilities

Nessuna. La condivisione fra i due strumenti e' un fatto di implementazione, non un comportamento
osservabile: il comportamento resta descritto dalla command line di ciascuno strumento.

### Modified Capabilities

- `interpreter-cli`: la selezione del file da eseguire smette di essere una validazione per forma
  esatta e passa al riconoscimento delle opzioni; gli scenari che oggi dicono che `-h`,
  `--bench` e `-bench` in posizione non riconosciuta sono percorsi o errori cambiano tutti.
- `compiler-cli`: la selezione dei sorgenti smette di trattare ogni argomento come sorgente e
  distingue opzioni, posizionali e terminatore `--`.

## Impact

- **`cli.c` nuovo** (~100 righe): solo costrutti C e C++, nessuna dipendenza, nessuna libreria
  esterna, nessun tipo del linguaggio. incluso dal `#include` nelle due unita' di traduzione,
  come gia' avviene per `pcodes.c`, `emit.c` e `gc.cpp`.
- **`parser.c`**: `main()` smette di iterare su `argv[1..argc-1]` come se fossero tutti sorgenti;
  riceve dal modulo condiviso la lista dei posizionali. `openSource()`, `pcodePath()`,
  `baseDot()`, `parse()` e `diag()` non cambiano.
- **`interpreter.cpp`**: `main()` perde il controllo `argc==3 && argv[1]=="-bench"` e usa la
  tabella delle opzioni. `pcodePathFor()`, `runPcd()` e `benchPcd()` non cambiano.
- **`Makefile`**: `compiler` e `interpreter` dipendono da `cli.c` per la ricompilazione.
  Nessun nuovo target di build: `cli.c` non viene compilato da solo.
- **`AGENTS.md`**: la sezione che descrive `-bench` come unico flag in prima posizione va
  riallineata, insieme alla regola "`-h` e' un percorso".
- **Nessuna dipendenza nuova, nessun formato nuovo, nessuna migrazione di dati**: il `.pcd` resta
  quello che `loadPcd()` sa leggere.