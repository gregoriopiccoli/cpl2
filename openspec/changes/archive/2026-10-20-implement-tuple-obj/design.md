# Design

## Context

`P_TUPLE` (32) e `P_STORESLICE` (69) sono già definiti in `pcodes.h`, hanno la loro voce in `initpcodetxt()` (`pcodes.c`) e la grammatica li emette già (`tuple_const ::= VBAR tuple_list(N) VBAR` → `emit_i(P_TUPLE,N)`; `aexpr ::= ref LSQ expr RSQ ASSIGN expr` → `emit(P_STORESLICE)` — `cpl2.gram` righe 451 e 325). L'unica parte mancante è nell'interprete: `makePCode()` (`interpreter.cpp` righe 440-494) non ha i due `case`, quindi cadono su `pcodeNotImpl`. Confermato a runtime: `./interpreter q.pcd` termina con `pcode:32 TUPLE value:3 not impl!`, e un fixture scratch con `a[1]:=9` termina con `pcode:69 STORESLICE value: not impl!`.

Il modello da seguire è il binomio già esistente `pcodeArray`/`arrayObj`:

- `pcodeArray` (`interpreter.cpp` ~228): sotto-classe di `ipcode`, operando intero = numero di elementi.
- `arrayObj` (~892): sotto-classe di `obj` con `vector<obj*>`, `slice()`/`storeslice()` (virtuali su `obj`, righe 510-512), `mark()`/`expand()` GC-aware, `print()`.
- `pcodeArray::exec()` (~1519): danza di stack che consuma N elementi e lascia il contenitore al loro posto.
- `pcodeSlice::exec()` (~1566) è già generico: chiama `cnt->slice(idx)`, quindi funziona su `tupleObj` appena esiste.

Il tipo `tupleType` (`P_TUPLE_TYPE`, ~817) e `pcodeTupleType::exec()` (~1693) sono già implementati: la dichiarazione `var tuple[int,str,bool] t` e la lettura `t[0]` funzionano già oggi; manca solo l'oggetto valore.

Vedi `proposal.md` per la motivazione e `specs/interpreter-pcodes/spec.md` per i requisiti.

## Goals / Non-Goals

**Goals:**

- Far eseguire `q.cpl` per intero (exit 0): dichiarazione, costante `|...|`, lettura `t[i]`, assegnamento `t[1]:=...`, stampa `? t`.
- Rendere `STORESLICE` eseguibile per tutti i contenitori (tuple, array, dizionari), non solo per le tuple.
- Restare coerenti con lo schema del progetto: una classe per p-code, oggetti valore sottoclassi di `obj`, GC mark & sweep gestito da `mark()`/`expand()`.

**Non-Goals:**

- Nessuna modifica a grammatica, `cpl2.c`/`cpl2.h`, `pcodes.h`, `pcodes.c` o compiler: i due p-code esistono già e vengono già emessi.
- Nessun type-checking degli elementi rispetto al tipo dichiarato `tuple[...]` (non esiste `is` per nessun contenitore).
- Nessun controllo di bounds sugli indici (vedi Risks).
- Nessuna operazione aggiuntiva sui tuple (concatenazione, `len`, push/pop): la dimensione è fissa per costruzione.
- Nessun ingresso in `pcodejump[]`: né `TUPLE` né `STORESLICE` sono caldi (`TUPLE` compare solo nella costante, `STORESLICE` solo nelle assegnamenti per elemento).

## Decisions

1. **Classe valore: `tupleObj : public obj` separata da `arrayObj`.** Contiene un `vector<obj*>` a dimensione fissa con `slice()`, `storeslice()`, `setelem()` (aiuto per la costruzione da parte del p-code), `mark()`/`expand()` e `print()`. Alternativa: riusare `arrayObj` per le tuple e distinguere solo nella stampa — rifiutata: la stampa dipenderebbe da un flag o da un cast, il tuple non ha `append`/`remove`/`resize` (dimensione fissa) e il progetto segue lo schema "una classe per concetto" (`arrayObj`, `dictObj`, `tupleType` sono già separati); inoltre la richiesta esplicita è la classe `tupleObj`.

2. **Formato di `print()`: `|v1,v2,...|`.** Elementi separati da `,` senza spazi (stile `arrayObj::print()`, che unisce con `","`), racchiusi dalle barre verticali della sintassi letterale. Esempio: `|10,beta,true|`. Alternativa: separatore `", "` come fa `tupleType::print()` — rifiutata: i contenitori *valore* (`arrayObj`, `dictObj`) uniscono senza spazi, e `tupleType` è un tipo, non un valore. Nota: `? t` di `q.cpl` rende questo formato osservabile; è l'unica scelta non deducibile dal codice esistente ed è registrata qui come assunzione.

3. **`pcodeTuple : public ipcode` con `exec()` che replica la danza di `pcodeArray::exec()`.** `sp = sp - N + 1`, riempimento degli N elementi dallo stack, `pop_back()` N-1 volte, sostituzione della cima con il nuovo `tupleObj`, `sp` aggiornato. Differenza rispetto all'originale: la grammatica non ammette la costante tuple vuota (`tuple_list` ha minimo 1 elemento, non esiste `||`), quindi il ramo `value==0` di `pcodeArray::exec()` non serve — precedente: `PROC_TYPE` non ha ereditato il ramo degenere che la grammatica non può produrre. Alternativa: un costruttore `tupleObj(vector<obj*>)` che prende gli elementi da un buffer temporaneo — rifiutata: è la stessa danza di stack già verificata in `pcodeArray`, copiarla è più leggibile di un secondo meccanismo.

4. **`pcodeStoreSlice : public pcode` (senza operando, come `pcodeSlice`) con `exec()` che consuma 3 elementi e ne lascia 1.** Layout stack: `[..., contenitore, indice, valore]` (`valore` in cima). L'`exec()` legge i tre, chiama `cnt->storeslice(idx, val)`, sostituye `stack[sp-2]` con `val`, fa due `pop_back()` e decrementa `sp` di 2: la profondità scende di 2 e il valore assegnato resta in cima, esattamente come richiesto dallo scenario dello spec (la grammatica emette poi un `POP`, perché l'assegnamento per elemento è un'espressione). Alternativa: lasciare i tre elementi e aggiustare il `POP` nella grammatica — rifiutata: toccare la grammatica è fuori scope e la stessa forma di stack è già quella che `STORESLICE` produce per ogni contenitore.

5. **Registrazione in `makePCode()`:** `case P_TUPLE:return new pcodeTuple(atoi(s));` accanto a `P_ARRAY`/`P_DICT` (~459), e `case P_STORESLICE:return new pcodeStoreSlice();` accanto a `P_SLICE` (~464). Nessuno dei due entra in `pcodejump[]`.

6. **GC: nessuna registrazione speciale.** `tupleObj` viene allocato con `new` come `arrayObj`; le radici sono stack di esecuzione e contesti, già gestiti da `gc.cpp`. Bastano `mark()`/`expand()` che propagano agli elementi, copiati pari pari da `arrayObj`.

## Risks / Trade-offs

- [Nessun controllo di bounds in `slice()`/`storeslice()`: un indice fuori intervallo è UB] → Accettato per coerenza: `arrayObj` e `dictObj` si comportano così oggi (`arrayObj` legge `a[pos->value]` senza verifica). Aggiungere il check solo al tuple creerebbe un'incoerenza; è un miglioramento separato da fare su tutti i contenitori insieme.
- [`STORESLICE` diventa eseguibile per array e dizionari, esponendo eventuali problemi latenti di quei contenitori] → Mitigato da una regressioni esplicita: un fixture con `a[1]:=9` su array e un assegnamento su dizionario, oltre a `q.cpl`.
- [Off-by-one nella danza di stack di `pcodeTuple::exec()` o `pcodeStoreSlice::exec()`] → La danza di `pcodeTuple` è la copia verificata di `pcodeArray`; quella di `pcodeStoreSlice` è controllabile a occhio contro `pcodeSlice::exec()` (stesso layout, meno un operando). Verifica finale: `q.cpl` arriva alla fine e l'ultimo `PRINT` della riga `? t` produce `|10,beta,true|`.
- [Il formato di stampa del tuple è un'assunzione, non un dato osservabile esistente] → Se non è quello desiderato basta cambiare `tupleObj::print()`; lo scenario nello spec va aggiornato insieme.

## Migration Plan

Nessuna migrazione: cambia solo `interpreter.cpp`, che viene ricompilato con `make`. Rollback: tornare al commit precedente. I `.pcd` esistenti non cambiano (nessuna modifica a grammatica o compiler), quindi restano eseguibili anche con la versione vecchia dell'interprete — solo che non arrivano a fine exec se contengono i due p-code.

## Open Questions

<!-- nessuna: il formato di stampa è deciso in Decisions (2), l'assunzione è registrata lì -->
