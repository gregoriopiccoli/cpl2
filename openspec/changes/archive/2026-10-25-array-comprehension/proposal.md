# Proposal

## Why

La grammatica riconosce già le quattro forme dell'array comprehension
(`[e for each v in x]`, `[e for each v,k in x]`, ognuna con o senza `with <condizione>`), ma
tutte emettono soltanto `COMMENT "da fare array comprehension ..."`: il p-code contiene
l'espressione e la collezione una volta sola, senza ciclo, e il `COMMENT` a runtime finisce in
`pcodeNotImpl`. Ora che il `for each` funziona, il ciclo che serve alla comprehension esiste già
(`FOREACH_INIT`/`FOREACH_VAR`/`FOREACH_KEY`/`FOREACH_NEXT`), e `q.cpl` termina con tre esempi
(`? [x for each x in [1,2,3]]`, la stessa con `with x>1`, e l'assegnazione `a:=[...]`).

## What Changes

- L'array comprehension diventa un'**espressione** eseguibile che produce un array nuovo:
  per ogni elemento della collezione (stesse regole di visita del `for each`: array per indice,
  dizionari in ordine non specificato, tuple per posizione) le variabili vengono assegnate, il
  filtro `with` (se c'è) viene valutato e, se vero, il valore dell'espressione viene aggiunto in
  fondo all'array risultato.
- Il ciclo riusa **senza modifiche** i quattro p-code del `for each`, con la stessa forma
  (`LABEL`, `FOREACH_NEXT`, `IF_FALSE` per il `with`, `GOTO`, `POP` finale dell'iteratore).
- **Un solo p-code nuovo**, perché nessun p-code esistente sa aggiungere un elemento a un array
  che sta sullo stack sotto l'iteratore: `COMPR_APPEND` (`P_COMPR_APPEND 116`, `MAX_PCODE` a 116).
  Il risultato si costruisce partendo da `ARRAY 0` (già esistente). È l'unico punto in cui la
  richiesta "dovrebbero bastare i p-code del for-each" non si può soddisfare alla lettera; il
  design discute le alternative scartate.
- **Riordino del codice nel compilatore.** Il parser LALR emette il codice dell'espressione `e`
  *prima* di aver visto la collezione, ma a runtime `e` va eseguita dentro il ciclo, *dopo*
  `FOREACH_NEXT`. Il compilatore smette di scrivere il p-code direttamente sul file: lo accumula
  in un buffer in memoria (svuotato alla fine di `parse()`) e, chiusa la comprehension, sposta il
  blocco di `e` dentro il corpo del ciclo. Il formato del `.pcd` e il comportamento visibile del
  compilatore (file scritti, diagnostiche, exit code) non cambiano.
- Semantica scelta (dettagli nel design):
  - le variabili `v` (e `k`) seguono la regola del `for each`: devono essere dichiarate **nel
    contesto corrente**, la comprehension non le dichiara e dopo l'espressione mantengono l'ultimo
    valore assegnato (nessuno scope proprio);
  - collezione vuota, o filtro mai vero, → `[]`;
  - la collezione è valutata una volta, prima del ciclo; `e` e la condizione una volta per
    elemento, nell'ordine di visita;
  - le comprehension si possono annidare (anche nell'espressione `e` e nella collezione).
- `cpl2.c`/`cpl2.h` rigenerati con `lemon`; nuovo test completo `test/array_compr.cpl` + `.out`;
  `AGENTS.md` aggiornato (le array comprehension non sono più fra i costrutti non implementati).

Fuori scope: le **dict comprehension** (restano `COMMENT "da fare dict comprehension ..."`, ma
potranno riusare buffer e riordino introdotti qui), uno scope locale per le variabili della
comprehension, la dichiarazione implicita delle variabili, il controllo del tipo degli elementi
rispetto a un `array[T]` di destinazione (gli array a runtime non sono tipizzati, come oggi per
le costanti array), i generatori.

## Capabilities

### New Capabilities

- `array-comprehension`: semantica dell'espressione `[e for each v[,k] in <collezione> [with <cond>]]`
  — array prodotto, ordine degli elementi, filtro, variabili locali, annidamento, errori.

### Modified Capabilities

- `interpreter-pcodes`: nuovo requisito per l'esecuzione del p-code `COMPR_APPEND` (effetto sullo
  stack) e per il suo riconoscimento in `makePCode()` e in `./asm`.

## Impact

- `pcodes.h` (`P_COMPR_APPEND 116`, `MAX_PCODE 116`), `pcodes.c` (`pcodetxt`).
- `emit.c`: buffer in memoria dietro `emit()/emit_i()/emit_s()/emit_line()/emit_ver()`, con
  `emit_pos()`, `emit_rotate()` e `emit_flush()`; deve restare C valido (e C++, come gli altri
  file condivisi).
- `parser.c`: `parse()` svuota il buffer su `emit_out` alla fine; campo per la posizione nel
  buffer nella struct `foreachloop`.
- `cpl2.gram` (sezione `--- costanti ARRAY ...`: nuovo non-terminale `lsq` condiviso da costante e
  comprehension, regole `compr_*`), `cpl2.c`/`cpl2.h` rigenerati.
- `interpreter.cpp`: classe `pcodeComprAppend` e `case` in `makePCode()`.
- `asm.c`: nessuna modifica al codice, ma va ricompilato (`touch asm.c && make`).
- Formato del `.pcd` invariato (header `#cpl20.0.1`).
- `q.cpl` (già pronto), `test/`, `AGENTS.md`.
