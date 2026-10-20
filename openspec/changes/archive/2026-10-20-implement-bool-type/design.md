# Design

## Context

`P_BOOL_TYPE` (47) esiste già in `pcodes.h` con la sua voce in `initpcodetxt()` (`pcodes.c`), e la grammatica lo emette: `btype ::= BOOL.` (`cpl2.gram` riga 184). L'unica parte mancante è nell'interprete: `makePCode()` non ha il `case P_BOOL_TYPE`, quindi il p-code cade su `pcodeNotImpl`. Il modello da seguire è `P_INT_TYPE` (richiesta esplicita del change):

- `pcodeIntType` (`interpreter.cpp` ~341): sotto-classe di `pcode` (p-code **senza** operando, a differenza di `ipcode`), costruttore `explicit pcodeIntType(){code=P_INT_TYPE;}`.
- `intType` (~743): sotto-classe di `obj` con `print()` che restituisce `int`.
- Singleton `theIntType` (~758): `lockgc_ptr<obj>` che tiene vivo l'oggetto tipo; azzerato a `nullptr` in `releaseSysObjs()` (~1925).
- `pcodeIntType::exec()` (~1627): `sp++` e push del singleton sullo stack.
- `case P_INT_TYPE:` in `makePCode()` (~469) senza `atoi(s)` (nessun operando).
- `intObj::getBaseType()` (~762): ritorna `theIntType`.

Differenza sostanziale rispetto ai tipi composti (`TUPLE_TYPE`, `PROC_TYPE`, `FUNC_TYPE`): `BOOL_TYPE` non consuma nulla dallo stack, costruisce solo il push del singleton. Le classi `boolObj` (~636) e i singleton `theTrue`/`theFalse` esistono già; mancano il tipo runtime e il suo singleton.

## Goals / Non-Goals

**Goals:**

- Rendere `BOOL_TYPE` eseguibile dall'interprete replicando lo schema di `INT_TYPE`.
- Classe p-code `pcodeBoolType` + classe tipo runtime `boolType` + singleton `theBoolType`, come richiesto.
- Completare il quadro di `intObj`: `boolObj::getBaseType()` ritorna `theBoolType`.

**Non-Goals:**

- Non si toccano `pcodes.h`, `pcodes.c`, `cpl2.gram`/`cpl2.c` (già completi per questo p-code).
- Non si cambia la logica esistente di `boolObj` (`eq`/`ne`/`check_bool`) né `theTrue`/`theFalse`; l'unica aggiunta a `boolObj` è l'override di `getBaseType()` (Decision 4).
- Non si implementano `P_FLOAT_TYPE`, `P_DATE_TYPE`, `P_TIME_TYPE`, `P_DATETIME_TYPE` (stesso schema, ma change separati).
- Nessuna modifica al formato `.pcd` né alla testata `#cpl20.0.1`.

## Decisions

1. **Classe p-code: `pcodeBoolType : public pcode`**, identica nello schema a `pcodeIntType` (senza operando, campo `code=P_BOOL_TYPE`). Alternativa: estendere `pcodeIntType` con un flag — rifiutata: il progetto segue lo schema "una classe per p-code" e ogni p-code ha il proprio `code`.
2. **Classe tipo: `boolType : public obj`** accanto a `intType`, con `print()` che restituisce `bool` (come `intType` restituisce `int`, `strType` restituisce `str`). Nessun sotto-dato: è un tipo atomico, come `intType`.
3. **Singleton `theBoolType`** (`lockgc_ptr<obj> theBoolType{new boolType()}`) affiancato a `theIntType`/`theStrType` (~758), azzerato in `releaseSysObjs()` accanto agli altri. Il `lockgc_ptr` garantisce che il GC non lo raccolga mai mentre è vivo. Nessuna alternativa: lo schema è già consolidato per `theIntType`/`theStrType`.
4. **`boolObj::getBaseType()` ritorna `theBoolType`**: override che replica `intObj::getBaseType()`. Oggi `boolObj` eredita l'implementazione base (`obj::getBaseType()` restituisce `theNil`); il change la sostituisce. Alternativa considerata: lasciare `theNil` — rifiutata: non è coerente con il modello di `intObj`, e il type-checking futuro (`is`) si baserà su `getBaseType()`.
5. **`exec()` di `pcodeBoolType`**: `sp++` e push del singleton, identico a `pcodeIntType::exec()`.
6. **Registrazione in `makePCode()`**: `case P_BOOL_TYPE:return new pcodeBoolType();` accanto a `P_INT_TYPE` (~469), senza `atoi(s)`. Nessun ingresso in `pcodejump[]` (non è caldo: compare solo nelle dichiarazioni di tipo).

## Risks / Trade-offs

- [`boolObj::getBaseType()` cambia comportamento da `theNil` a `theBoolType`] → Nessun codice esistente dipende dal vecchio valore (l'unico uso di `getBaseType()` è in `check_int`, che confronta con `theIntType`): rischio nullo, ma è la modifica più "invasiva" del change; se emergesse una dipendenza imprevista, si annulla l'override senza toccare il resto.
- [`theBoolType` non ha utenti oltre a `pcodeBoolType::exec()`] → È il minimo vitale coerente con `intType` (anch'esso singleton poco usato); il tipo è comunque costruibile e stampabile, prerequisite per sviluppi futuri.
- [Verifica manuale, niente runner di test] → La verifica si fa con un `.cpl` di prova che dichiari una variabile di tipo bool, più `./asm < file.pcd` per controllare l'emissione; vedi tasks.

## Open Questions

(nessuno)
