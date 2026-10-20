# Tasks

## 1. Classe valore `tupleObj`

- [x] 1.1 Dichiarare in `interpreter.cpp`, accanto a `arrayObj` (~riga 892), la classe `tupleObj : public obj`: `vector<obj*>` a dimensione fissa, `slice(idx)` e `storeslice(idx, value)` con lo stesso schema di `arrayObj` (indice intero obbligatorio, `domain_error` altrimenti), `setelem(int, obj*)` per il riempimento da parte del p-code, `mark()` e `expand(int gen)` che propagano agli elementi con lo schema di `arrayObj` — verifica: `make interpreter` compila senza errori né warning nuovi
- [x] 1.2 Implementare `tupleObj::print()` nel formato `|v1,v2,...|`: elementi uniti con `","` senza spazi (stile `arrayObj::print()`), racchiusi tra `|` — verifica: `make interpreter` compila; la stampa osservabile è collaudata in 2.2 e 4.1

## 2. P-code TUPLE

- [x] 2.1 Dichiarare in `interpreter.cpp`, accanto a `pcodeArray` (~riga 228), la classe `pcodeTuple : public ipcode` con `code=P_TUPLE` e `exec()` override che replica la danza di `pcodeArray::exec()` senza il ramo `value==0` (la grammatica garantisce N≥1): calcola `sp = sp - N + 1`, riempie gli N elementi dallo stack nel nuovo `tupleObj`, fa `pop_back()` N-1 volte, sostituisce la cima e aggiorna `sp` — verifica: `make interpreter` compila
- [x] 2.2 Aggiungere `case P_TUPLE:return new pcodeTuple(atoi(s));` in `makePCode()` accanto a `P_ARRAY`/`P_DICT` (~riga 459), senza metterlo in `pcodejump[]` — verifica: `make interpreter` compila e un programma scratch con solo la costante tuple (es. `? |1,2|`) compilato con `./compiler < scratch.cpl > scratch.pcd` e eseguito con `./interpreter scratch.pcd` stampa `|1,2|` ed esce con codice 0 (copre anche `print()` di 1.2)

## 3. P-code STORESLICE

- [x] 3.1 Dichiarare in `interpreter.cpp`, accanto a `pcodeSlice` (~riga 258), la classe `pcodeStoreSlice : public pcode` con `code=P_STORESLICE` ed `exec()` che legge contenitore (`sp-2`), indice (`sp-1`) e valore (cima), chiama `cnt->storeslice(idx, val)`, scrive `val` in `stack[sp-2]`, fa due `pop_back()` e decrementa `sp` di 2 — verifica: `make interpreter` compila e la danza di stack è confrontata a occhio con `pcodeSlice::exec()`
- [x] 3.2 Aggiungere `case P_STORESLICE:return new pcodeStoreSlice();` in `makePCode()` accanto a `P_SLICE` (~riga 464) — verifica: `make interpreter` compila; il fixture scratch `var array[int] a := [1,2,3]` + `a[1] := 9` + `? a[1]` (che prima del change termina con `pcode:69 STORESLICE ... not impl!`) ora stampa `9` ed esce con codice 0, e un assegnamento per elemento equivalente su un dizionario fa altrettanto

## 4. Integrazione e regressioni

- [x] 4.1 Collaudare il programma di test completo: `./compiler q.cpl` poi `./interpreter q.pcd` — verifica: nessun `not impl!`, la riga `? t[0],' ',t[1],' ',t[2]` stampa `10 alfa true`, la riga `? t` stampa `|10,beta,true|`, compare `------------ arrivato alla fine ------------` e il processo esce con codice 0
- [x] 4.2 Collaudare le regressioni sui fixture esistenti che usano le tuple e i contenitori (per esempio `test/test_var1.cpl`, `terzo.cpl`, `test/test_stm1.cpl`, oltre a un fixture con assegnamento `a[i]:=v` su array già visto in 3.2) — verifica: nessun comportamento peggiorato rispetto a prima del change (`./compiler` + `./interpreter` su ciascuno, confronto dell'output/exit code con il comportamento precedente; i `.pcd` non cambiano)

## Workflow follow-up

- Archiviare il change (`openspec archive`) dopo la revisione del progetto.
- Verificare il risultato archiviato: lo spec delta su `interpreter-pcodes` è stato fuso nello spec principale.
