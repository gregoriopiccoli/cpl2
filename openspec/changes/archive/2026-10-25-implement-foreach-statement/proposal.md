# Proposal

## Why

La grammatica riconosce già le quattro forme del `for each` (`for each v in x`,
`for each v,k in x`, ognuna con o senza `with <condizione>`), ma tutte emettono soltanto
`COMMENT "da fare for-each"`: il p-code contiene l'espressione della collezione e il corpo una
volta sola, senza ciclo, e il `COMMENT` a runtime finisce in `pcodeNotImpl`. Ora che il `for`
numerico funziona, il `for each` è l'unico modo che manca per scorrere array, dizionari e tuple, e
`q.cpl` ne contiene già cinque esempi (array con indice, dizionario con e senza chiave, tupla,
array con `with`).

## What Changes

- Nuovi p-code per il ciclo sulle collezioni:
  - `FOREACH_INIT`: sostituisce la collezione in cima allo stack con un **iteratore** interno
    (array, dizionario o tupla; qualsiasi altro valore è un errore a runtime);
  - `FOREACH_VAR <nome>`: verifica che la variabile del valore sia **locale** (dichiarata nel
    contesto corrente, stessa regola del `for`) e la registra nell'iteratore;
  - `FOREACH_KEY <nome>`: come sopra per la seconda variabile (indice o chiave); su una tupla è un
    errore a runtime, perché le tuple non hanno chiave;
  - `FOREACH_NEXT <label>`: se la collezione è finita salta alla label, altrimenti assegna
    valore (e chiave) alle variabili registrate.
- La grammatica del `for each` in `cpl2.gram` viene riscritta con non-terminali intermedi (come il
  `for`) e sparisce il `COMMENT "da fare for-each"` / `"da fare for-each-with"`. L'iteratore resta
  sullo stack per tutto il ciclo e viene tolto da un `POP` finale.
- Semantica scelta (dettagli e alternative nel design):
  - array: elementi in ordine di indice, chiave = indice `int` a partire da 0;
  - dizionari: ogni coppia una volta, in **ordine non specificato** (quello della hash map), chiave =
    la chiave del dizionario come `str` (è la rappresentazione interna attuale); le chiavi sono
    quelle presenti all'inizio del ciclo;
  - tuple: elementi in ordine di posizione, solo la forma a una variabile;
  - `with <condizione>` è un **filtro**: valutato dopo l'assegnazione delle variabili, se è
    `false` l'elemento viene saltato (il corpo non viene eseguito); deve essere `bool`;
  - le variabili mantengono dopo il ciclo l'ultimo valore assegnato; con zero iterazioni restano
    invariate; un'assegnazione alla variabile nel corpo non cambia l'elemento successivo.
- **Requisito dell'utente:** valore e chiave vanno su variabili già dichiarate nel contesto
  corrente; il `for each` non le dichiara e rifiuta una variabile visibile solo da un livello
  superiore, come il `for`.
- Il design descrive, **senza implementarli**, come gli stessi p-code potranno scorrere i futuri
  **generatori** (`yield`): l'iteratore è un'interfaccia, il generatore ne sarà un'altra
  implementazione e il p-code del ciclo non cambierà.
- `cpl2.c`/`cpl2.h` rigenerati con `lemon`; `pcodes.h`, `pcodes.c`, `parser.c` e
  `interpreter.cpp` aggiornati (`MAX_PCODE` a 115).
- Nuovo test completo `test/foreach.cpl` + `.out` e test compile-only `test/foreach_err.cpl` per
  i casi rifiutati a runtime; `AGENTS.md` aggiornato.

Fuori scope: generatori e `yield` (solo progetto), comprehension di array e dizionari (restano
`COMMENT`, potranno riusare questi p-code), iterazione sulle stringhe (errore a runtime come per
ogni valore non iterabile), un ordine di visita dei dizionari (dove serve si potrà ottenere
l'array delle chiavi e ordinarlo, con una funzione ancora da definire), `break`/`continue` (non esistono), dichiarazione implicita delle
variabili, controllo a compile time di località o tipi.

## Capabilities

### New Capabilities

- `foreach-statement`: semantica dello statement `for each v[,k] in <collezione> [with <cond>]`
  su array, dizionari e tuple, requisito delle variabili locali, filtro `with`, ordine di visita.

### Modified Capabilities

- `interpreter-pcodes`: nuovi requisiti per l'esecuzione dei p-code `FOREACH_INIT`,
  `FOREACH_VAR`, `FOREACH_KEY`, `FOREACH_NEXT` (effetto sullo stack, controlli, salti) e per il
  loro riconoscimento in `makePCode()` e in `./asm`.

## Impact

- `pcodes.h` (`P_FOREACH_INIT 112`, `P_FOREACH_VAR 113`, `P_FOREACH_KEY 114`,
  `P_FOREACH_NEXT 115`, `MAX_PCODE 115`), `pcodes.c` (`pcodetxt`).
- `parser.c`: nuova struct per portare i due nomi e le label fra le riduzioni.
- `cpl2.gram` (sezione `--- for each statement ---`), `cpl2.c`/`cpl2.h` rigenerati.
- `interpreter.cpp`: classe iteratore astratta con tre implementazioni (array, tupla,
  dizionario), quattro classi `pcodeForeach*`, i `case` in `makePCode()`; `dictObj` non cambia
  rappresentazione.
- `asm.c`: nessuna modifica al codice, ma va ricompilato (`touch asm.c && make`).
- Formato del `.pcd` invariato (header `#cpl20.0.1`).
- `q.cpl` (già pronto), `test/`, `AGENTS.md`.
