# Tasks

## 1. Buffer del p-code nel compilatore (nessun cambio di comportamento)

- [x] 1.1 In `emit.c` sposta l'output in un buffer in memoria (design D3): helper variadico con
      `vsnprintf` + `realloc` usato da `emit()`, `emit_i()`, `emit_s()`, `emit_line()`,
      `emit_ver()`; aggiungi `emit_pos()`, `emit_rotate(from,mid)` e `emit_flush()`; gli `#include`
      necessari (`stdarg.h`, `stdlib.h`, `string.h`) e codice valido sia in C sia in C++. In
      `parse()` (`parser.c`) chiama `emit_flush()` prima del `return`. Prima della modifica
      controlla con `grep -n "emit_out" *.c cpl2.gram` che nessun altro scriva su `emit_out`.
      Verifica: `make` senza warning nuovi; `c++ -fsyntax-only -x c++ emit.c` (con un piccolo
      file di prova nello scratchpad che includa `pcodes.h`/`pcodes.c` prima) compila; `./runtest`
      passa come prima della modifica.
- [x] 1.2 Controlla che l'output non sia cambiato: nello scratchpad compila con il `compiler`
      *prima* della modifica (da `git stash` o da una copia del binario) e *dopo* tutti i `.cpl` di
      `test/` e `q.cpl`, sia con i file come argomenti sia in modalità `< x.cpl > x.pcd`.
      Verifica: `cmp` dei `.pcd` byte per byte senza differenze (le comprehension producono ancora
      il `COMMENT`, la grammatica non è ancora cambiata).

## 2. P-code COMPR_APPEND

- [x] 2.1 In `pcodes.h` aggiungi `P_COMPR_APPEND 116` accanto ai `P_FOREACH_*` e porta `MAX_PCODE`
      a 116; in `pcodes.c` `pcodetxt[P_COMPR_APPEND]="COMPR_APPEND"` in `initpcodetxt()`.
      Verifica: `touch asm.c && make` completa e `grep -n COMPR_APPEND pcodes.h pcodes.c` mostra
      le due righe.
- [x] 2.2 In `interpreter.cpp` aggiungi `pcodeComprAppend` (`pcode`, senza operando) accanto a
      `pcodeForeachNext`, con `exec()` secondo design D2 (append sull'array in `sp-2`, `pop` del
      valore) e il `case P_COMPR_APPEND` in `makePCode()`; non in `pcodejump[]`.
      Verifica: `make` senza warning nuovi.
- [x] 2.3 Prova il p-code isolato con un listato `asm` scritto a mano nello scratchpad (il listato
      dell'esempio D1, con `VAR x` dopo l'header) convertito in binario con `./asm`.
      Verifica: `./interpreter` stampa `[2,3]` ed esce 0 senza violare `assert(sp==-1)`.

## 3. Grammatica e test

- [x] 3.1 In `parser.c` aggiungi `int mark` a `foreachloop` e sposta la testa del ciclo
      dell'azione di `foreach_src` in una funzione C (definita dopo `#include "emit.c"` e prima di
      `#include "cpl2.c"`); in `cpl2.gram` fai chiamare quella funzione da `foreach_src`, introduci
      `lsq` in `array_const` e sostituisci le quattro regole `array_compr` con
      `array_compr`/`compr_head`/`compr_src`/`compr_in` del design D4 (le `dict_compr` restano
      `COMMENT`). Rigenera con `lemon cpl2.gram`. Verifica: `lemon` esce 0, `cpl2.out` non riporta
      conflitti, `make` ricompila `compiler`, e `./runtest` passa (in particolare `foreach`, il cui
      p-code non deve cambiare: confrontalo con `cmp` con quello del task 1.2).
- [x] 3.2 Compila nello scratchpad `var int x` + `? [x for each x in [1,2,3] with x>1]` e leggilo
      con `./asm`. Verifica: il listato, tolte le righe `LINE`, coincide con l'esempio D1 del
      design; non contiene `COMMENT`.
- [x] 3.3 Crea `test/array_compr.cpl` + `test/array_compr.out` con gli scenari non di errore di
      `specs/array-comprehension/spec.md`: identità, espressione calcolata, assegnazione ad
      `array[int]` e stampa, collezione vuota, tupla, dizionario a una chiave, indici con la
      seconda variabile, `with` vero e mai vero, `e` non valutata sugli scartati (func che stampa),
      collezione da func valutata una volta, variabile dopo la comprehension, annidamento in `e` e
      nella collezione, comprehension su due righe, e una comprehension su `[i for each i in ...]`
      che costruisce qualche migliaio di stringhe concatenate (oltre `GC_OBJSLIM`) stampandone
      solo lunghezza o primo/ultimo elemento per far partire il GC. Verifica: `./runtest` riporta
      `array_compr` fra i test superati (gira con `--typecheck`).
- [x] 3.4 Casi d'errore a mano nello scratchpad (non nel suite, perché terminano con exit ≠ 0):
      `[i for each i in 5]`, variabile non dichiarata `[z for each z in [1]]`, `[v for each v,k in
      |1,2|]`, `with` non booleano. Verifica: ognuno stampa l'errore del `for each` (che nomina la
      variabile nei casi di contesto) ed esce con codice ≠ 0 senza stampare un array.
- [x] 3.5 Esegui gli esempi di `q.cpl` (non tracciato, già pronto). Verifica:
      `./compiler q.cpl && ./interpreter q` esce 0 e dopo `array comprehension` stampa `[1,2,3]` e
      `[4,6]`.
- [x] 3.6 Aggiorna `AGENTS.md`: in "Don't assume a construct runs" le array comprehension non sono
      più fra i `P_COMMENT` (restano le dict comprehension) e va aggiunta la descrizione della forma
      del p-code (design D1); nel paragrafo "Wiring" il buffer di `emit.c` (`emit_pos`,
      `emit_rotate`, `emit_flush` chiamata da `parse()`); nel paragrafo sui p-code il conteggio dei
      pcode definiti/implementati. Verifica: `grep -n "comprehension" AGENTS.md` mostra il testo
      aggiornato e i numeri citati coincidono con `pcodes.h` e `makePCode()`.

## Workflow follow-up

- Commit (in italiano) dei soli sorgenti: `emit.c`, `parser.c`, `pcodes.h`, `pcodes.c`,
  `interpreter.cpp`, `cpl2.gram`, `cpl2.c`, `cpl2.h`, `test/array_compr.cpl`, `test/array_compr.out`,
  `AGENTS.md` e la change; mai `git add -A`.
- Archiviare la change con `/opsx:archive` dopo la revisione.
