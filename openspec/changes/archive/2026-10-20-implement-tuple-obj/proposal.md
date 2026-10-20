# Proposal

## Why

L'interprete non implementa né il valore tuple né l'assegnamento per elementi: `makePCode()` fa cadere `P_TUPLE` (32) e `P_STORESLICE` (69) su `pcodeNotImpl`, quindi `q.cpl` — il programma di test delle tuple — compila correttamente ma termina subito con `pcode:32 TUPLE value:3 not impl!`. Anche l'assegnamento `a[i]:=v` su array e dizionari è rotto per la stessa causa (`P_STORESLICE` non ha case nella fabbrica), come verificato su un fixture scratch. Il tipo `tuple[...]` (`tupleType`, `P_TUPLE_TYPE`) e la lettura `t[i]` (`P_SLICE`) già funzionano: manca solo l'oggetto valore.

## What Changes

- Nuova classe `tupleObj` in `interpreter.cpp` (sotto-classe di `obj`, modello `arrayObj`): vettore di elementi a dimensione fissa, `slice()`/`storeslice()` per `t[i]` in lettura e scrittura, `mark()`/`expand()` GC-aware, `print()` nel formato `|v1,v2,...|`.
- Nuova classe `pcodeTuple` in `interpreter.cpp` (sotto-classe di `ipcode`, come `pcodeArray`): l'operando intero N è il numero di elementi; consuma i N valori in cima allo stack e lascia al loro posto l'oggetto `tupleObj`.
- `case P_TUPLE:` in `makePCode()`.
- Nuova classe `pcodeStoreSlice` + `case P_STORESLICE:` in `makePCode()`: consume `container`, `indice` e `valore` dallo stack, chiama `storeslice()` sul contenitore e lascia in cima il valore assegnato (la grammatica emette poi un `POP`). È lo stesso p-code per tuple, array e dizionari: implementarlo è il prerequisito perché `q.cpl` arrivi alla fine.
- Nessuna modifica a grammatica, `pcodes.h`, `pcodes.c` o compiler: i due p-code esistono già e vengono già emessi.

## Capabilities

### New Capabilities

<!-- nessuna -->

### Modified Capabilities

- `interpreter-pcodes`: nuovi requisiti per l'esecuzione di `P_TUPLE` (costruzione del valore tuple dallo stack, stampa, GC) e di `P_STORESLICE` (assegnamento di un elemento su un contenitore), più la nuova condizione "non è più un p-code non implementato" per entrambi.

## Impact

- `interpreter.cpp`: nuove classi `tupleObj`, `pcodeTuple`, `pcodeStoreSlice` e due `case` in `makePCode()`.
- Test: `q.cpl` (dichiarazione `tuple[int,str,bool]`, costante `|...|`, lettura `t[i]`, assegnamento `t[1]:=...`, stampa) deve arrivare alla fine con exit 0; come regressioni `test/test_var1.cpl` e `terzo.cpl` usano già le tuple, e l'assegnamento su array/dizionari diventa eseguibile (prima: `not impl!`).
- Nessuna modifica al compiler, alla grammatica o ai file `.pcd` già generati; nessuna modifica ai due binari tranne `interpreter`.
