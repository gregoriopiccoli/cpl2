# Proposal

## Why

Il tipo `float` ha costanti, `is float`, il controllo di tipo e il meno unario, ma nessuna
operazione binaria: `? 1.5 + 1.5` termina con `plus not implemented` e `? 1.5 < 2.5` con
`lt not implemented`, perché `floatObj` non ridefinisce nessuno dei metodi aritmetici e di
confronto di `obj`. Un float con cui non si può fare conto né confronto è inutilizzabile; questo
change gli dà le stesse operazioni che ha `int`, più la promozione negli operandi misti
`int`/`float`.

## What Changes

- `floatObj` implementa `+ - * /` e i sei confronti `= < <= >= > <>` (p-code `PLUS`, `MINUS`,
  `MULT`, `DIV`, `EQ`, `LT`, `LE`, `GE`, `GT`, `NE`), come `intObj`: l'aritmetica produce un
  float **nuovo** (mai l'operando modificato, le costanti sono condivise), i confronti
  producono `true`/`false`.
- **Promozione a float negli operandi misti**: se uno dei due operandi è `float` e l'altro è
  `int`, l'`int` viene convertito in `double` e il risultato è un float (`1 + 0.5` → `1.5`,
  `0.5 + 1` → `1.5`, `2 * 1.5` → `3.0`) o un confronto fatto sui `double` (`1 < 1.5` → `true`,
  `1 = 1.0` → `true`). Per supportare `int op float` cambia anche `intObj`: oggi lo rifiuta
  con `integer + with a non integer`.
- `int op int` resta esattamente com'è: risultato `int`, `/` fra interi resta la divisione
  intera, nessuna penalità sul percorso caldo (`fib`).
- La divisione fra float segue IEEE 754: `1.0 / 0.0` stampa `inf`, `0.0 / 0.0` stampa `nan`
  (o `-nan`), nessun errore di runtime.
- Un float con un operando che non è né `int` né `float` (stringa, bool, nil, ...) resta un
  errore di runtime, come per gli interi.
- `mod` / `%` e `div` restano operazioni solo intere: con almeno un operando float terminano
  con un errore di runtime.
- Test completo `test/float.cpl`/`test/float.out` esteso; `AGENTS.md` aggiornato (oggi dice
  che ogni operazione float diversa da `UMINUS` lancia `... not implemented` e che non c'è
  promozione).

Fuori scope: promozione **nelle assegnazioni** (`var float x := 1` resta rifiutato da
`--typecheck`: la promozione riguarda solo gli operandi degli operatori), conversioni esplicite
(`int(x)`, `float(i)`), `mod`/`div` sui float, i confronti a tre termini (`LT_LT`, `LE_LE`, ...,
oggi non implementati per nessun tipo), `case` con costanti float (il `CASE` continua a non far
corrispondere tipi diversi: `case 1` non prende un selettore `1.0`), `for` con estremi float.

## Capabilities

### New Capabilities

(nessuna)

### Modified Capabilities

- `interpreter-pcodes`: nuovi requisiti per l'aritmetica (`PLUS`, `MINUS`, `MULT`, `DIV`) e i
  confronti (`EQ`, `LT`, `LE`, `GE`, `GT`, `NE`) sui float, per la promozione negli operandi
  misti `int`/`float`, per gli errori con operandi non numerici e per `MOD`/`IDIV` con float.

## Impact

- `interpreter.cpp`: `floatObj` (nuovi override `plus`, `minus`, `mult`, `div`, `eq`, `lt`,
  `le`, `ge`, `gt`, `ne`), `intObj` (le stesse operazioni accettano un operando float e
  delegano al calcolo in `double`).
- Nessuna modifica a grammatica, lexer, `pcodes.h`/`pcodes.c`, `asm.c`, formato `.pcd`,
  `pcodejump[]`: i p-code sono già emessi e già dispacciati con le chiamate virtuali di `obj`.
- `test/float.cpl`, `test/float.out`: nuovi casi.
- `AGENTS.md`: sezione "Don't assume a construct runs".
