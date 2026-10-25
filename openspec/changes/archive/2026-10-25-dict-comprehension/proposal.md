# Proposal

## Why

La grammatica riconosce già le dict comprehension, ma tutte le regole emettono soltanto
`COMMENT "da fare dict comprehension ..."`: il p-code contiene chiave, valore e collezione una volta
sola, senza ciclo, e a runtime il `COMMENT` finisce in `pcodeNotImpl`. Ora che `for each` e array
comprehension funzionano, il ciclo, il riordino del p-code nel compilatore (`emit_pos()`/
`emit_rotate()`) e l'accumulo del risultato (`COMPR_APPEND`) esistono già, e `q.cpl` termina con i
due esempi da far girare:

```
? {k:x for each x,k in [1,2,3]}
? {k:x for each x:k in [1,2,3] : ["alfa","beta","gamma"]}
```

## What Changes

- **Forma con chiave del `for each`**: `{ke:ve for each v,k in x [with c]}` diventa
  un'espressione che produce un dizionario nuovo. Per ogni elemento visitato di `x` (stesse regole
  del `for each`: array per indice, dizionari in ordine non specificato; una tupla è un errore
  perché non ha chiave) vengono assegnate `v` e `k`, valutato il filtro `with` e, se vero, aggiunta
  al dizionario la coppia `ke:ve`.
- **Forma parallela (zip)**: `{ke:ve for each v:w in A : B [with c]}` visita `A` e `B` insieme,
  posizione per posizione: `v` riceve l'elemento corrente di `A`, `w` quello di `B`. Se le due
  collezioni non hanno lo stesso numero di elementi il programma termina con un **errore a
  runtime** (scelta dell'utente: un disallineamento è quasi sempre un bug).
- Semantica comune: le chiavi sono convertite in stringa come nella costante dizionario
  (`print()` del valore), una chiave ripetuta tiene l'ultimo valore, collezione vuota o filtro mai
  vero → `{}`, le collezioni sono valutate una volta, `ke`/`ve`/la condizione una volta per
  elemento; le variabili seguono le regole del `for each` (dichiarate nel contesto corrente, nessuno
  scope proprio, conservano l'ultimo valore).
- **Nessun p-code nuovo** (richiesta: riciclare i p-code esistenti). Si estendono due p-code già
  esistenti con un operando che oggi è sempre assente (0):
  - `COMPR_APPEND 2` inserisce la coppia chiave/valore in cima allo stack nel dizionario sotto
    l'iteratore (`COMPR_APPEND` senza operando resta l'append all'array);
  - `FOREACH_INIT 2` sostituisce le **due** collezioni in cima allo stack con un unico iteratore
    parallelo, su cui `FOREACH_VAR`, `FOREACH_KEY`, `FOREACH_NEXT` funzionano senza modifiche
    (`FOREACH_KEY` registra la variabile che riceve gli elementi di `B`).
  Il p-code generato oggi per `for each` e array comprehension resta byte per byte identico.
- `cpl2.gram`: le quattro regole `dict_compr` con due variabili (con `,` e con `:`, con e senza
  `with`) emettono il ciclo; le due regole a una sola variabile restano commentate. `cpl2.c`/
  `cpl2.h` rigenerati.
- Nuovo test completo `test/dict_compr.cpl` + `.out`; `AGENTS.md` aggiornato (le dict comprehension
  non sono più fra i costrutti non implementati).

Fuori scope: la forma a una sola variabile `{ke:ve for each v in x}` (regole commentate in
`cpl2.gram`, resta un errore di sintassi), la forma parallela nell'array comprehension e nello
statement `for each`, uno scope locale per le variabili, il controllo del tipo dei valori rispetto a
un `dict[T]` di destinazione (come per la costante dizionario), un ordine definito di stampa dei
dizionari.

## Capabilities

### New Capabilities

- `dict-comprehension`: semantica di `{ke:ve for each v,k in x [with c]}` e
  `{ke:ve for each v:w in A : B [with c]}` — dizionario prodotto, chiavi, visita parallela ed errore
  di lunghezza, filtro, variabili, annidamento, errori.

### Modified Capabilities

- `interpreter-pcodes`: `FOREACH_INIT` accetta l'operando `2` (iteratore parallelo su due
  collezioni) e `FOREACH_NEXT` segnala l'errore di lunghezza diversa; `COMPR_APPEND` accetta
  l'operando `2` (inserimento chiave/valore in un dizionario).

## Impact

- `cpl2.gram` (sezione costanti dizionario: marcatore di inizio condiviso da costante e
  comprehension, regole `dict_compr`), `cpl2.c`/`cpl2.h` rigenerati.
- `parser.c`: `foreachTesta()` (e la struct `foreachloop`) emette `FOREACH_INIT 2` per la forma
  parallela, `FOREACH_INIT` senza operando altrimenti.
- `interpreter.cpp`: costruttori/`makePCode()` di `pcodeForeachInit` e `pcodeComprAppend` che
  leggono l'operando, nuova classe iteratore parallelo (con `mark()`/`expand()` per il GC).
- `pcodes.h`/`pcodes.c`/`asm.c`: nessuna modifica (nessun p-code nuovo, `MAX_PCODE` invariato).
- Formato del `.pcd` invariato (header `#cpl20.0.1`).
- `test/`, `AGENTS.md`.
