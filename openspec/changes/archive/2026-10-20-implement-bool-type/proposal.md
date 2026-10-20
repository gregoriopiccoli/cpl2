# Proposal

## Why

Il compilatore emette già `P_BOOL_TYPE` (grammatica `btype ::= BOOL.`, `cpl2.gram` riga 184) e `pcodes.c` ne ha già la voce in `initpcodetxt()`, ma l'interprete non lo implementa: `makePCode()` non ha il `case P_BOOL_TYPE`, quindi ogni sorgente che dichiara un tipo bool — per esempio una variabile `BOOL b` — compila correttamente e poi termina con `not impl!` a runtime. L'implementazione di `P_INT_TYPE` (`pcodeIntType`, singleton `theIntType`, tipo runtime `intType`) è lo schema completo da replicare.

## What Changes

- Nuova classe `pcodeBoolType` in `interpreter.cpp` (sotto-classe di `pcode`, senza operando, come `pcodeIntType`): `exec()` spinge in cima allo stack di esecuzione il singleton del tipo bool.
- Nuova classe `boolType` in `interpreter.cpp` (sotto-classe di `obj`, come `intType`): `print()` restituisce `bool`.
- Nuovo singleton `theBoolType` (`lockgc_ptr<obj>`), affiancato a `theIntType`/`theStrType`, e relativo rilascio in `releaseSysObjs()`.
- `boolObj::getBaseType()` ritorna `theBoolType`, come `intObj::getBaseType()` ritorna `theIntType` (oggi `boolObj` eredita l'implementazione base che restituisce `theNil`).
- Aggiunta del `case P_BOOL_TYPE:return new pcodeBoolType();` in `makePCode()`.
- Nessuna modifica a `pcodes.h`, `pcodes.c` né alla grammatica: `P_BOOL_TYPE` (47), la sua voce testuale e la sua emissione esistono già; la testata `.pcd` `#cpl20.0.1` resta valida.

## Capabilities

### New Capabilities

(nessuna)

### Modified Capabilities

- `interpreter-pcodes`: aggiunta dei requisiti di esecuzione, rappresentazione stampabile, sicurezza GC e riconoscimento in `makePCode()` per il p-code `BOOL_TYPE`, allo stesso modo di quanto già coperto per `PROC_TYPE` e `FUNC_TYPE`.

## Impact

- `interpreter.cpp` soltanto: una nuova classe p-code, una nuova classe tipo runtime, un nuovo singleton, un `case` in `makePCode()`, un override di `getBaseType()`.
- Il `.pcd` prodotto dal compilatore non cambia.
- I programmi cpl2 che dichiarano tipi bool da eseguire passano da runtime error a esecuzione corretta.
