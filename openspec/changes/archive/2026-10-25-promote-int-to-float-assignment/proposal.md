# Proposal

## Why

Un intero deve poter essere assegnato a una variabile `float`: `var float f2 := 0` in `q.cpl`
oggi, con `--typecheck`, interrompe il programma con
`f2: il valore 0 non e' compatibile con il tipo float`, perche' `tipoCompatibile()` accetta per
`float` solo un `floatObj`. Senza `--typecheck` l'assegnazione passa ma `f2` contiene un intero,
quindi `f2 / 3` e' una divisione intera e `(f2 is float)` e' `false`: una variabile dichiarata
`float` non contiene un float. La promozione int -> float oggi esiste solo dentro gli operatori.

## What Changes

- Quando si assegna un `int` a un nome il cui tipo dichiarato richiede un float, il valore viene
  **convertito** in un nuovo float con lo stesso valore (`0` -> `0.0`): la variabile contiene
  sempre un float.
- Vale ovunque si assegna con un tipo dichiarato: inizializzazione (`VAR_STORE`), `STORE`,
  `STOREX` (nomi di modulo), parametri di proc/func (passati e di default), `result` di una func,
  e le variabili scritte da `for`/`for each`.
- "Richiede un float" vuol dire: il tipo e' `float`, `not float`, oppure un'unione che ha `float`
  fra le alternative **e** non accetta gia' l'intero cosi' com'e'. Quindi `var float|str x := 1`
  contiene `1.0`, mentre `var int|float x := 1` e `var ? x := 1` contengono ancora l'intero `1`.
- La conversione avviene **con e senza** `--typecheck`: e' una regola di assegnazione, non un
  controllo. Con `--typecheck` l'intero assegnato a un float non e' piu' un errore.
- Fuori perimetro: gli elementi di array/dict/tuple tipizzati (`var array[float] a := [1,2]`
  resta un array di interi, come oggi non se ne controllano gli elementi); l'operatore `is`
  (`(1 is float)` resta `false`); `case 1` continua a non corrispondere a un selettore `1.0`;
  nessuna conversione float -> int (`var int i := 1.5` resta rifiutato).
- **BREAKING** (minimo): un programma che senza `--typecheck` assegnava un intero a una variabile
  `float` ora la vede contenere un float (stampa `0.0` invece di `0`, divisione float).

## Capabilities

### New Capabilities
(nessuna)

### Modified Capabilities
- `type-checking`: nuovo requisito sulla promozione int -> float nelle assegnazioni ai nomi con
  tipo dichiarato float (variabili, parametri, `result`), con e senza `--typecheck`.

## Impact

- `interpreter.cpp`: una funzione unica che adatta il valore al tipo dichiarato, chiamata prima di
  `checkTipo()` in `contextObj::add(intern,type,value)`, `contextObj::store()` e
  `funcVars::store_result()`; quei tre punti coprono tutti i percorsi di assegnazione.
- `test/float.cpl`/`.out` (o un nuovo fixture) con i casi di promozione; `test/type_check_err.cpl`
  resta per i rifiuti.
- `AGENTS.md`: la frase "`var float x := 1` ... are rejected by `--typecheck`" va aggiornata.
- Nessun impatto su `parser.c`, `cpl2.gram`, `pcodes.*`, `asm.c`, formato `.pcd`.
