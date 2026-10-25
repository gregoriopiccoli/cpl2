# Proposal

## Why

Le costanti float si compilano ma non si eseguono. Il lexer riconosce `3.14` come
`TOK_FLOAT_CONST`, la grammatica emette `FLOAT_CONST 3.14` (`fact ::= FLOAT_CONST`) e il `.pcd`
lo contiene, però `makePCode()` non ha un `case P_FLOAT_CONST`: il p-code cade su `pcodeNotImpl`
e `? 3.14` stampa `not impl!` e abortisce. Il tipo `float` invece esiste già (`FLOAT_TYPE`,
`floatObj`, `is`, il controllo di tipo delle assegnazioni): `var float x` si dichiara, ma non c'è
nessun modo di metterci dentro un valore. Questo change chiude il buco più piccolo — avere un
valore float — senza ancora affrontare l'aritmetica.

## What Changes

- Nuovo `pcodeFloatConst` nell'interprete e `case P_FLOAT_CONST` in `makePCode()`: spinge sullo
  stack un `floatObj` costruito una volta sola, al caricamento, dal testo dell'operando (come
  `INT_CONST` fa con `intObj`).
- `floatObj` passa da `float` a **`double`** e stampa la **forma più corta che rilegge lo stesso
  valore**, sempre riconoscibile come float: `3.14` → `3.14`, `2.0` → `2.0`, `1.` → `1.0`
  (oggi `to_string(float)` darebbe `3.140000`).
- `floatObj` dichiara il suo tipo base (`getBaseType()` → `float`).
- `UMINUS` funziona anche sui float: `? -1.5` stampa `-1.5`, producendo un valore nuovo senza
  toccare la costante. **Cambia** il requisito esistente "UMINUS su valori non interi": il float
  esce dall'elenco dei tipi che danno errore.
- Nuovo test completo `test/float.cpl` + `test/float.out` (costanti, forma di stampa, meno
  unario, variabile `var float`, `is float`, la costante non cambia tra un giro e l'altro).
- `AGENTS.md`: le costanti float non sono più fra i costrutti che compilano e poi esplodono.
- Nessuna modifica a grammatica, lexer, `pcodes.h`/`pcodes.c`, `asm.c` o formato del `.pcd`: il
  p-code esiste già, è già emesso come operando stringa e `asm` lo mostra già.

Fuori scope (change successivi): aritmetica e confronti fra float (`+ - * /`, `= < ...`) e
operazioni miste int/float, che restano `... not implemented`; conversioni (`int(x)`,
`float(i)`); notazione esponenziale e costanti come `.5` nel lexer; `case` con costanti float;
`var float x := 1` resta rifiutato dal controllo di tipo (un intero non è un float, nessuna
promozione).

## Capabilities

### New Capabilities

(nessuna)

### Modified Capabilities

- `interpreter-pcodes`: nuovo requisito per l'esecuzione del p-code `FLOAT_CONST` (valore sullo
  stack, rappresentazione stampabile, costante non condivisa in modo mutabile, non più
  `pcodeNotImpl`); nuovo requisito per `UMINUS` sui float; modificato il requisito "UMINUS su
  valori non interi" per escludere i float dai tipi che danno errore.

## Impact

- `interpreter.cpp`: `floatObj` (`double`, `print()`, `uminus()`, `getBaseType()`), nuova classe
  `pcodeFloatConst`, riga in `makePCode()`.
- `test/float.cpl`, `test/float.out`: nuovo test completo, eseguito da `runtest` con
  `--typecheck`.
- `AGENTS.md`: sezione "Don't assume a construct runs".
- Nessun impatto su compilatore, `asm`, formato `.pcd`, prestazioni dei p-code caldi
  (`FLOAT_CONST` non entra in `pcodejump[]`).
