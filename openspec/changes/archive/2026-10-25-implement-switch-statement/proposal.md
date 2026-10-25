# Proposal

## Why

La grammatica riconosce già `switch ... case ... else ... end`, ma l'unica azione è
`emit_s(P_COMMENT,"da fare switch ...")` alla riduzione finale e le costanti dei `case` non
emettono nulla: il p-code prodotto contiene il selettore e i corpi di tutti i `case` uno dopo
l'altro, senza alcun salto, e poi un `COMMENT` che a runtime finisce in `pcodeNotImpl`.
Lo `switch` è l'ultimo statement di controllo "semplice" ancora non eseguibile (accanto a `for`),
e `q.cpl` e `test/test_stm1.cpl` lo usano già.

## What Changes

- Nuovo p-code `CASE` (operando intero = label): toglie dallo stack la costante del `case`, la
  confronta con il selettore che resta sotto, e salta alla label se sono uguali; il selettore
  resta sullo stack in ogni caso.
- La grammatica dello `switch` in `cpl2.gram` viene riscritta per generare p-code effettivi:
  il selettore viene calcolato una volta sola, ogni costante di un `case` produce
  `<costante>` + `CASE Lcorpo`, ogni `case` termina con `GOTO Lfine`, il selettore viene tolto
  dallo stack con `POP` all'ingresso di ogni corpo (e del ramo `else`, implicito se manca).
  Sparisce il `COMMENT "da fare switch ..."`.
- Semantica: nessun fall-through (si esegue un solo ramo), il primo `case` che corrisponde
  vince, l'`else` è opzionale; una costante di tipo diverso dal selettore **non** corrisponde
  (nessun errore), così `case 1,2,3` e `case 'a','b'` possono stare nello stesso `switch` come
  in `test/test_stm1.cpl`.
- `cpl2.c`/`cpl2.h` rigenerati con `lemon`; `pcodes.h`, `pcodes.c`, `asm.c` (tramite
  `MAX_PCODE`) e `interpreter.cpp` aggiornati per il nuovo p-code.
- Nuovo test completo `test/switch.cpl` + `test/switch.out`.
- `AGENTS.md`: conteggio dei p-code aggiornato.
- le costanti del "case" possono essere anche numeri negativi

Fuori scope: espressioni nei `case` (la grammatica accetta solo
`INT_CONST`, `STR_CONST`, `true`, `false`), controllo a compile time di costanti duplicate,
tabelle di salto/hash per `switch` con molti `case`.

## Capabilities

### New Capabilities

- `switch-statement`: semantica dello statement `switch` di cpl2 e forma del p-code che il
  compilatore genera per esso.

### Modified Capabilities

- `interpreter-pcodes`: nuovo requisito per l'esecuzione del p-code `CASE` (effetto sullo
  stack, confronto, salto) e per il suo riconoscimento in `makePCode()`.

## Impact

- `pcodes.h` (`P_CASE 107`, `MAX_PCODE 107`), `pcodes.c` (`pcodetxt`).
- `cpl2.gram` (sezione `--- switch statement ---`), `cpl2.c`/`cpl2.h` rigenerati.
- `interpreter.cpp`: classe `pcodeCase`, `case P_CASE:` in `makePCode()`.
- `asm.c`: nessuna modifica al codice, legge `MAX_PCODE`.
- Il formato del `.pcd` non cambia (header `#cpl20.0.1` invariato): un `.pcd` vecchio con uno
  `switch` conteneva comunque un `COMMENT` non eseguibile.
- `test/`, `AGENTS.md`.
