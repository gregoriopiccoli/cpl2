# Proposal

## Why

Il compilatore emette già `P_PROC_TYPE` e `P_FUNC_TYPE` (grammatica `btype ::= LPAR type_list RPAR`, `LPAR RPAR`, `LPAR type COLON type_list RPAR`, `LPAR type COLON RPAR`), ma l'interprete non li implementa: `makePCode()` li fa cadere su `pcodeNotImpl`, quindi ogni sorgente che dichiara un tipo proc o func — per esempio una variabile di tipo `(int, str)` o `(int:int)` — compila correttamente e poi termina con `not impl!` a runtime. `P_TUPLE_TYPE` mostra già lo schema completo: la classe p-code e il tipo runtime; `P_PROC_TYPE` e `P_FUNC_TYPE` vanno seguiti allo stesso modo.

## What Changes

- Nuova classe `pcodeProcType` in `interpreter.cpp` (sotto-classe di `ipcode`, come `pcodeTupleType`): legge l'operando intero = numero di tipi dei parametri.
- Nuova classe `procType` in `interpreter.cpp` (sotto-classe di `obj`, come `tupleType`): memorizza la lista dei tipi dei parametri, con `mark()`/`expand()` per il GC e `print()` nel formato `(t1, t2, ...)`.
- `pcodeProcType::exec()`: costruisce un `procType` consumando i N tipi dalla cima dello stack e lasciando l'oggetto risultante al loro posto, come fa `pcodeTupleType::exec()`.
- Nuova classe `pcodeFuncType` (stesso schema) e classe `funcType`: tipi dei parametri più il tipo di ritorno, `print()` nel formato `(ritorno:p1, p2)`.
- `pcodeFuncType::exec()`: consuma i N tipi dei parametri e il tipo di ritorno dallo stack, lasciando in cima l'oggetto tipo func.
- Aggiunta dei `case P_PROC_TYPE:` e `case P_FUNC_TYPE:` in `makePCode()`.
- Nessuna modifica a `pcodes.h`, `pcodes.c` né alla grammatica: `P_PROC_TYPE` (57), `P_FUNC_TYPE` (56) e la loro emissione esistono già.

## Capabilities

### New Capabilities

- `interpreter-pcodes`: comportamento dell'interprete nell'esecuzione dei singoli p-code implementati (in questo change: i p-code `PROC_TYPE` e `FUNC_TYPE` e i tipi runtime `proc` e `func`). Accoglierà i requisiti degli altri p-code man mano che vengono implementati.

### Modified Capabilities

(nessuna — `compiler-cli` e `interpreter-cli` non cambiano requisiti)

## Impact

- `interpreter.cpp` soltanto: quattro nuove classi, due nuovi `case` in `makePCode()`, due nuovi metodi `exec()`.
- Il `.pcd` prodotto dal compilatore non cambia: la testata `#cpl20.0.1` resta valida.
- I programmi cpl2 che dichiarano tipi proc o func da eseguire passano da runtime error a esecuzione corretta.
