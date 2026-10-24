# Design

## Context

Vedi `proposal.md` per la motivazione. Lo stato attuale in `interpreter.cpp`:
- `interp` ha `pcodeProgram& prg;` come membro. In `procObj::call` viene fatto `const pcodeProgram& retPrg=interpreter.prg; ... interpreter.prg=prg; ... interpreter.run(); ... interpreter.prg=retPrg;`. Poiché `prg` è un riferimento membro, assegnare ad esso copia il contenuto dell'oggetto referenziato (assegnamento per riferimento implicito al puntatore del membro) piuttosto che rilegare il riferimento: l'oggetto `pcodeProgram` del chiamante viene sovrascritto con i contenuti di quello del modulo, causando esecuzione di codice errato e segfault al ritorno.
- `procObj` memorizza `pcodeProgram& prg` legato al programma in cui è stata definita la procedura (valido).
- `main()` usa `theSys.execModule(percorsi[0])` per il percorso normale invece di derivare con `pcodePathFor` e usare `runPcd` come da documentazione AGENTS.
- I test `q`, `q1`, `q2` esistono nella root ma non sono registrati in `test/`.

## Goals / Non-Goals

**Goals:**
- Rendere il programma corrente nell'interprete puntabile/riagganciabile correttamente (salvare e ripristinare un puntatore).
- Garantire ripristino di contesto e PC dopo chiamata a procedura/funzione di modulo.
- Allineare CLI a `pcodePathFor` + `runPcd` per esecuzione normale.
- Aggiungere `test/q*.cpl` e `.out` per i test indicati e farli passare.

**Non-Goals:**
- Cambiare il formato `.pcd` o i p-code esistenti.
- Modifiche al runner `runtest`.
- Cambiare meccanismo di cache moduli (`sys`) se non strettamente necessario.

## Decisions

### 1. Sostituire il riferimento `prg` con un puntatore
Cambiare `pcodeProgram& prg;` in `interp` a `pcodeProgram* prg;`. Nel costruttore `prg{&p}`. Aggiornare tutti gli accessi: `prg[pc]` → `(*prg)[pc]`, `prg.getLabelPos` → `prg->getLabelPos`, `prg.size()` → `prg->size()`. Questo permette di salvare `pcodeProgram* retPrg = interpreter.prg;`, impostare `interpreter.prg = &prg;` (dove `prg` è quello di `procObj`), e ripristinare con `interpreter.prg = retPrg;` senza copiare oggetti.

Alternative considerate: usare `std::reference_wrapper` (non riabilitabile) o copiare il programma. Copiare è inefficiente e rischioso; puntatore è semplice e corretto.

### 2. Correggere `procObj::call`
Mantenere salvataggio di `retPc`, `retPrg` (puntatore), `retCtx`. Impostare `interpreter.context = vars; interpreter.prg = &prg;` (program della procedura). Dopo `run()`, ripristinare tutto. Verificare che `procObj` continui a tenere riferimento/relazione corretta al proprio `pcodeProgram` (il suo membro `pcodeProgram& prg` può restare o diventare riferimento al programma del modulo; resta valido se il programma del modulo vive nella cache).

### 3. Aggiornare accessi a `prg` in tutto `interpreter.cpp`
Sistemare `pcodeProgram::get/operatore[]` già usano `prg` interno (non relativo all'interp). Negli `pcode*::exec` cercare `interpreter.prg.` → `interpreter.prg->`. Nei punti in computed-goto mantenere coerenza.

### 4. Allineare CLI
In `main()`, per il percorso normale (non help/test), derivare il percorso con `pcodePathFor(percorsi[0])` e chiamare `rc=runPcd(...)`. Per `-b/--bench` usare `benchPcd(pcodePathFor(percorsi[0]))`. Mantenere il comportamento di `execModule` solo se necessario, ma preferire il percorso documentato (runPcd) per compatibilità con i test.

### 5. Aggiungere test q/q1/q2
Copiare/creare `test/q.cpl`, `test/q1.cpl`, `test/q2.cpl` con gli stessi contenuti trovati in root (o equivalenti) e aggiungere `.out` corrispondenti. Per `q`: output atteso deve includere tutte le stampe fino alla riga finale. `q1` può essere usato come modulo (test completo con output o come modulo helper). Assicurare che il runner li trovi.

## Risks / Trade-offs

- [Cambiare tipo membro da riferimento a puntatore] → Modifica estesa ma meccanica; verificare tutte le inizializzazioni.
- [CLI cambia da execModule a runPcd] → Potrebbe influire sul comportamento, ma allineato ad AGENTS; i test dei moduli esistenti devono restare verdi.
- [Test fixture dipendenti da ordine] → Già accade per test moduli; mantenere nomi coerenti.
