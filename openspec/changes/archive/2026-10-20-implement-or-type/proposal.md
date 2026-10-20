# Proposal

## Why

Il compilatore emette già `P_OR_TYPE` (grammatica `type ::= or_type(N)`, `or_type ::= btype VBAR btype` e `or_type VBAR btype`), ma l'interprete non lo implementa: `makePCode()` lo fa cadere su `pcodeNotImpl`, quindi ogni sorgente che dichiara un tipo alternativo — per esempio `var int|str o` in `q.cpl` — compila correttamente e poi termina con `pcode:58 OR_TYPE value:2 not impl!` a runtime. `P_TUPLE_TYPE` mostra già lo schema completo (classe p-code con operando intero = numero di tipi consumati + tipo runtime con lista di sottotipi): `P_OR_TYPE` va seguito allo stesso modo.

## What Changes

- Nuova classe `pcodeOrType` in `interpreter.cpp` (sotto-classe di `ipcode`, come `pcodeTupleType`): legge l'operando intero = numero di alternative del tipo.
- Nuova classe `orType` in `interpreter.cpp` (sotto-classe di `obj`, come `tupleType`): memorizza la lista dei tipi alternativi, con `mark()`/`expand()` per il GC e `print()` nel formato `t1|t2|...`.
- `pcodeOrType::exec()`: costruisce un `orType` consumando i N tipi dalla cima dello stack e lasciando l'oggetto risultante al loro posto, come fa `pcodeTupleType::exec()`.
- Aggiunta del `case P_OR_TYPE:` in `makePCode()`.
- Nessuna modifica a `pcodes.h`, `pcodes.c` né alla grammatica: `P_OR_TYPE` (58), la sua voce in `initpcodetxt()` e la sua emissione esistono già.

## Capabilities

### New Capabilities

(nessuna)

### Modified Capabilities

- `interpreter-pcodes`: nuovi requisiti per l'esecuzione del p-code `OR_TYPE`, la rappresentazione stampabile del tipo or, la sicurezza GC del tipo or e il fatto che `OR_TYPE` non è più un p-code non implementato (stesso schema dei requisiti già presenti per `PROC_TYPE`, `FUNC_TYPE` e `BOOL_TYPE`).

## Impact

- `interpreter.cpp` soltanto: due nuove classi, un nuovo `case` in `makePCode()`, un nuovo metodo `exec()`.
- Il `.pcd` prodotto dal compilatore non cambia: la testata `#cpl20.0.1` resta valida.
- I programmi cpl2 che dichiarano tipi alternativi (es. `q.cpl`) passano da runtime error a esecuzione corretta.
