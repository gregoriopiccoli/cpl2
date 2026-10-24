# Proposal

## Why

Quando una procedura o una funzione definita in un modulo importato viene chiamata, il ripristino dello stato dell'interprete dopo la chiamata è errato. `procObj::call` salva e ripristina `interpreter.prg` per riferimento con un assegnamento al membro di riferimento, copiando il programma invece di reindirizzarlo; questo corrompe il programma del chiamante (soprattutto in esecuzione commutata/puntata tra moduli) e lascia l'interprete in uno stato non corretto alla fine della chiamata. Inoltre la CLI principale usa un percorso di esecuzione diverso da quello documentato per i moduli e non applica coerentemente `pcodePathFor`, rendendo difficile eseguire correttamente i test dei moduli. I test forniti (`q`, `q2`) mostrano segfault o mancata stampa della riga finale per questo motivo.

## What Changes

- Correggere il ripristino del programma nell'interprete: rendere `interp.prg` un puntatore al `pcodeProgram` corrente invece di un riferimento, aggiornare tutte le sue occorrenze (`(*prg)[pc]`, `prg->...`) e salvare/ripristinare il puntatore in `procObj::call`. Assicurare che `procObj` mantenga un riferimento/puntatore al programma della procedura corretto.
- Correggere `pcodeCall`/esecuzione in modo che il contesto (`context`) venga ripristinato correttamente e il PC resti valido dopo il ritorno da una funzione/procedura di modulo.
- Allineare l'interfaccia di esecuzione CLI: usare `pcodePathFor` per i percorsi senza estensione e `runPcd` per l'esecuzione normale (coerente con il comportamento documentato in AGENTS.md), in modo che `./interpreter q`, `./interpreter q.pcd`, ecc. funzionino come previsto.
- Aggiungere supporto/registrazione dei test `q`, `q1`, `q2` come test completi in `test/` con output atteso per `q` e `q2` (e `q1` opzionale se usato come modulo). Verificare che `q` completi senza segfault e stampi tutte le righe previste, e che `q2` stampi anche la sua riga finale.

## Capabilities

### New Capabilities
- `interpreter-call-restore`: corretta restaurazione di programma, contesto e contatore di programma dopo una chiamata a funzione/procedura definita in un modulo importato; garantisce che l'esecuzione continui nel programma corretto del chiamante.

### Modified Capabilities
- `interpreter-cli`: allineamento del percorso di esecuzione e dell'uso di `pcodePathFor`/`runPcd` per la CLI normale.

## Impact

- `interpreter.cpp`: modifica della struttura `interp` (membro `prg` da riferimento a puntatore), aggiornamento degli accessi a `prg`, modifica di `procObj::call` per salvare/ripristinare puntatore al programma, possibili aggiustamenti in `makePCode`/esecuzione, modifica di `main()` per usare `pcodePathFor` e `runPcd`.
- `test/`: aggiunta di `q.cpl`, `q1.cpl`, `q2.cpl` e relativi `.out` per i test forniti.
- Nessuna modifica a `pcodes.h`, `pcodes.c`, `cpl2.gram`, `asm.c`, `parser.c`, runner `runtest` (regole esistenti).
- Comportamento runtime corretto per chiamate tra moduli senza segfault.
