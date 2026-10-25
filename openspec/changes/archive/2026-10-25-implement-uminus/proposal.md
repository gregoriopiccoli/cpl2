# Proposal

## Why

Il meno unario si compila già ma non si esegue. La grammatica emette `UMINUS` per
`fact ::= MINUS fact` e l'interprete lo riconosce (`pcodeUMinus` in `makePCode()`), però
`pcodeUMinus::exec` delega a `obj::uminus()` e **nessun oggetto lo ridefinisce**: la versione base
lancia `domain_error("uminus not implemented")`. Risultato: `? -5`, `? -(2+3)`, `? 3 - -2`
compilano con exit 0 e poi abortiscono l'interprete. È l'unica operazione aritmetica di base
sugli interi che manca.

## What Changes

- `intObj` ridefinisce `uminus()`: restituisce un nuovo intero con il valore negato, senza
  modificare l'operando (che può essere la costante condivisa di un `INT_CONST` o il valore di una
  variabile).
- `UMINUS` su un valore che non è un intero (stringa, bool, nil, array, ...) continua a fallire a
  runtime con `domain_error`, come oggi: nessun cambiamento per quei tipi.
- Nuovo test completo `test/uminus.cpl` + `test/uminus.out` (costanti, espressioni tra parentesi,
  variabili, doppia negazione, meno unario dentro un'espressione binaria, precedenza rispetto a
  `*`).
- Nessuna modifica a grammatica, `pcodes.h`/`pcodes.c`, `asm.c` o formato del `.pcd`: il p-code
  esiste già ed è già emesso.

Fuori scope: meno unario sui float (i `FLOAT_CONST` non sono ancora eseguibili e `floatObj` non ha
nessuna operazione aritmetica), `+` unario (già ignorato dalla grammatica), l'ambiguità di
`print -5` che viene letto come `print - 5` (meno binario fra la funzione `print` e `5`):
è una questione di grammatica, non di questo p-code.

## Capabilities

### New Capabilities

(nessuna)

### Modified Capabilities

- `interpreter-pcodes`: nuovo requisito per l'esecuzione del p-code `UMINUS` sugli interi
  (effetto sullo stack, valore prodotto, operando non modificato, errore sui tipi non interi).

## Impact

- `interpreter.cpp`: dichiarazione e definizione di `intObj::uminus()`.
- `test/uminus.cpl`, `test/uminus.out`.
- Nessun impatto su compilatore, `asm`, formato del `.pcd` o altri test.
