# Design

## Context

`P_PROC_TYPE` (57) e `P_FUNC_TYPE` (56) esistono già in `pcodes.h` con le loro voci in `initpcodetxt()` (`pcodes.c`), e la grammatica li emette: `btype ::= LPAR type_list(N) RPAR` e `btype ::= LPAR RPAR` per `PROC_TYPE`, `btype ::= LPAR type COLON type_list(N) RPAR` e `btype ::= LPAR type COLON RPAR` per `FUNC_TYPE` (`cpl2.gram` righe 201-204). L'unica parte mancante è nell'interprete: `makePCode()` non ha i rispettivi `case`, quindi i p-code cadono su `pcodeNotImpl`. Il modello da seguire è `P_TUPLE_TYPE`:

- `pcodeTupleType` (`interpreter.cpp` ~365): sotto-classe di `ipcode` (p-code con operando intero), costruttore `explicit pcodeTupleType(int v):ipcode(v){code=P_TUPLE_TYPE;}`.
- `tupleType` (~779): sotto-classe di `obj` con `vector<obj*> types`, `addSubType()`, `mark()`/`expand()` GC-aware, `print()` che restituisce `tuple[t1, t2, ...]`.
- `pcodeTupleType::exec()` (~1595): legge i `value` tipi dalla cima dello stack, li registra nel tipo e lascia il tipo prodotto al posto del primo di essi.
- `case P_TUPLE_TYPE:` in `makePCode()` (~461) con `atoi(s)`.

Differenza sostanziale rispetto a `TUPLE`: la grammatica ammette `PROC_TYPE` con operando **0** (`LPAR RPAR`), mentre `TUPLE [...]` ha sempre almeno un tipo. Il `exec()` di `pcodeTupleType` con `value==0` farebbe `sp-=(0-1)`, cioè *aumenterebbe* lo stack: il nuovo `exec()` deve gestire il caso 0. `FUNC_TYPE` ha sempre il tipo di ritorno sullo stack, quindi anche con 0 parametri lo stack ha almeno un elemento da consumare.

## Goals / Non-Goals

**Goals:**

- Rendere `PROC_TYPE` e `FUNC_TYPE` eseguibili dall'interprete seguendo lo schema di `P_TUPLE_TYPE`.
- Classe p-code (`pcodeProcType`, `pcodeFuncType`) + classi tipo runtime (`procType`, `funcType`), come richiesto.
- Gestire correttamente il caso N=0 (tipo proc senza parametri; tipo func senza parametri ma con tipo di ritorno).

**Non-Goals:**

- Non si toccano `pcodes.h`, `pcodes.c`, `cpl2.gram`/`cpl2.c` (già completi per questi p-code).
- Non si implementa `P_PARMNAME` (82) né il type-checking con `is`: `procType`/`funcType` servono a costruire e stampare il tipo, come `tupleType`.
- Nessuna modifica al formato `.pcd` né alla testata `#cpl20.0.1`.

## Decisions

1. **Classe p-code: `pcodeProcType : public ipcode`**, identica nello schema a `pcodeTupleType` (operando intero = numero di tipi dei parametri). Alternativa: estendere `pcodeTupleType` a entrambi i p-code — rifiutata: il progetto segue lo schema "una classe per p-code", e i due p-code hanno casistiche diverse (N=0).
2. **Classe tipo: `procType : public obj`** accanto a `tupleType`, con `vector<obj*> types`, `addSubType()`, `mark()`/`expand()` che propagano ai tipi dei parametri (stesso schema GC di `tupleType`/`arrayType`). Nessun singleton: il tipo dipende dai parametri, come `arrayType`.
3. **Formato di `print()`: `(t1, t2, ...)`** con le parentesi tonde (e `()` se vuoto): riflette la sintassi della grammatica per il tipo proc. Scelta registrata nello spec come comportamento osservabile (aggiornata in implementazione: era `proc(...)` nella bozza iniziale).
4. **`exec()` con ramo dedicato per N=0**: se `value==0` fa `sp++` e push del nuovo `procType` vuoto; se `value>=1` replica la danza di stack di `pcodeTupleType::exec()` (`sp-=(value-1)`, `pop_back()` di value-1 volte, sostituzione della cima). Alternativa: normalizzare sempre a `sp-=value-1; push` con value clamped — rifiutata: due rami espliciti sono più leggibili e ogni rami è verificabile a occhio.
5. **Registrazione in `makePCode()`**: `case P_PROC_TYPE:return new pcodeProcType(atoi(s));` e `case P_FUNC_TYPE:return new pcodeFuncType(atoi(s));`, accanto a `P_TUPLE_TYPE`. Nessuno dei due entra in `pcodejump[]` (non sono caldi: compaiono solo nelle dichiarazioni di tipo).
6. **Classe p-code: `pcodeFuncType : public ipcode`** con lo stesso schema di `pcodeProcType`. Attenzione al campo `code`: deve essere `P_FUNC_TYPE` (56), non `P_PROC_TYPE` — l'errore di copia-incolla iniziale è stato corretto; `exec()` virtuale non ne dipende, ma `getCode()` e i trace di debug sì.
7. **Classe tipo: `funcType : public obj`** con `vector<obj*> prmTypes` più `obj* retType`. L'ordine sullo stack al runtime è `[ritorno, p1, ..., pN]` (la grammatica emette `type` prima di `type_list`), quindi `exec()` legge il tipo di ritorno in `stack[sp-value]`, i parametri sopra di esso, e alla fine consuma `value+1` elementi lasciando il tipo func in cima: la profondità dello stack diminuisce di `value` (sarebbe `value-1` per PROC_TYPE + 1 per il ritorno, con N=0 il caso degenere è gestito dagli stessi calcoli perché non c'è il ramo dedicato che serve a PROC_TYPE). Alternativa considerata: una sola classe `signatureType` con flag proc/func — rifiutata: `funcType` senza tipo di ritorno non esiste nella grammatica, e due classi distinte mantengono `print()` semplici.
8. **Formato `print()` di `funcType`: `(ritorno:p1, p2)`** e `(ritorno:)` senza parametri, coerente con la sintassi della grammatica e con il formato proc aggiornato.

## Risks / Trade-offs

- [Il `exec()` di `pcodeTupleType` ha lo stesso difetto per value==0] → Non si tocca in questo change (fuori scope); si annota che il bug latente esiste solo se la grammatica produrrà mai `TUPLE_TYPE` con 0. Il nuovo codice non lo eredita.
- [`procType` non è ancora usato da alcun type-checking] → È il minimo vitale coerente con `tupleType` (anch'esso senza `is`); nessun rischio di comportamento corretto ma inutile: il tipo è costruibile e stampabile, prerequisite per gli sviluppi futuri.
- [Verifica manuale, niente runner di test] → La verifica si fa con un `.cpl` di prova che dichiari un tipo proc, più `./asm < file.pcd` per controllare l'emissione; vedi tasks.

## Open Questions

(nessuno)
