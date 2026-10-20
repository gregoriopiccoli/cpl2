# Design

## Context

`P_OR_TYPE` (58) esiste già in `pcodes.h` con la sua voce in `initpcodetxt()` (`pcodes.c`), e la grammatica lo emette: `type ::= or_type(N). {emit_i(P_OR_TYPE,N);}` con `or_type ::= btype VBAR btype` (N=2) e `or_type ::= or_type VBAR btype` (N=P+1) — `cpl2.gram` righe 175-178. L'unica parte mancante è nell'interprete: `makePCode()` non ha il `case`, quindi il p-code cade su `pcodeNotImpl` (confermato: `./compiler q.cpl && ./interpreter q.pcd` termina con `pcode:58 OR_TYPE value:2 not impl!`). Il modello da seguire è `P_TUPLE_TYPE`, che ha lo stesso schema di forma (lista di tipi consumati, un operando intero):

- `pcodeTupleType` (`interpreter.cpp` ~371): sotto-classe di `ipcode`, costruttore `explicit pcodeTupleType(int v):ipcode(v){code=P_TUPLE_TYPE;}`.
- `tupleType` (~809): sotto-classe di `obj` con `vector<obj*> types`, `addSubType()`, `mark()`/`expand()` GC-aware, `print()` che restituisce `tuple[t1, t2, ...]`.
- `pcodeTupleType::exec()` (~1668): legge i `value` tipi dalla cima dello stack, li registra nel tipo e lascia il tipo prodotto al posto del primo di essi (`sp-=(value-1)`, `pop_back()` value-1 volte, sostituzione della cima).
- `case P_TUPLE_TYPE:` in `makePCode()` (~480) con `atoi(s)`.

Differenza sostanziale rispetto a `PROC_TYPE` (che aveva bisogno di un ramo dedicato per N=0): la grammatica ammette `OR_TYPE` solo con operando **almeno 2** (`btype VBAR btype` è il minimo), quindi il caso degenere N<2 non esiste e il `exec()` può replicare letteralmente la danza di stack di `pcodeTupleType::exec()`.

## Goals / Non-Goals

**Goals:**

- Rendere `OR_TYPE` eseguibile dall'interprete seguendo lo schema di `P_TUPLE_TYPE`.
- Classe p-code (`pcodeOrType`) + classe tipo runtime (`orType`), come richiesto.

**Non-Goals:**

- Non si toccano `pcodes.h`, `pcodes.c`, `cpl2.gram`/`cpl2.c` (già completi per questo p-code).
- Non si implementa il type-checking con `is`: `orType` serve a costruire e stampare il tipo, come `tupleType`.
- Nessuna modifica al formato `.pcd` né alla testata `#cpl20.0.1`.

## Decisions

1. **Classe p-code: `pcodeOrType : public ipcode`**, identica nello schema a `pcodeTupleType` (operando intero = numero di alternative). Alternativa: estendere `pcodeTupleType` a entrambi i p-code — rifiutata: il progetto segue lo schema "una classe per p-code".
2. **Classe tipo: `orType : public obj`** accanto a `tupleType`, con `vector<obj*> altTypes`, `addAltType(obj*)`, `mark()`/`expand()` che propagano alle alternative (stesso schema GC di `tupleType`). Nessun singleton: il tipo dipende dalle alternative, come `arrayType`.
3. **Formato di `print()`: `t1|t2|...`** con separatore `|`: riflette la sintassi della grammatica per il tipo or (la stampa di `int|str` è `int|str`). Scelta registrata nello spec come comportamento osservabile.
4. **`exec()` senza ramo dedicato**: con `value>=2` replica la danza di stack di `pcodeTupleType::exec()` (`sp-=(value-1)`, `pop_back()` di value-1 volte, sostituzione della cima). Alternativa: gestire anche `value<2` per difesa — rifiutata: la grammatica non può produrlo, e il precedente di `PROC_TYPE` (ramo dedicato solo perché N=0 è reale) conferma che i rami si aggiungono solo quando il caso esiste.
5. **Registrazione in `makePCode()`**: `case P_OR_TYPE:return new pcodeOrType(atoi(s));`, accanto a `P_TUPLE_TYPE`. Non entra in `pcodejump[]` (non è caldo: compare solo nelle dichiarazioni di tipo).

## Risks / Trade-offs

- [`orType` non è ancora usato da alcun type-checking] → È il minimo vitale coerente con `tupleType`/`procType` (anch'essi senza `is`); il tipo è costruibile e stampabile, prerequisite per gli sviluppi futuri.
- [Verifica manuale, niente runner di test] → La verifica si fa con il programma di prova `q.cpl` (che ora fallisce), più `./asm < q.pcd` per controllare l'emissione; vedi tasks.

## Open Questions

(nessuno)
