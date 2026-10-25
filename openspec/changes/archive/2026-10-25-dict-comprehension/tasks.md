# Tasks

## 1. Riferimento prima delle modifiche

- [x] 1.1 Prima di toccare qualunque file, copia nello scratchpad i binari `compiler` e `interpreter`
      attuali e con quel `compiler` compila tutti i `.cpl` di `test/` e `q.cpl` in una directory
      dello scratchpad (sorgenti copiati lì, così niente `.pcd` finisce in `test/`).
      Verifica: un `.pcd` di riferimento per ogni sorgente; `./runtest` passa.

## 2. Operandi di COMPR_APPEND e FOREACH_INIT nell'interprete

- [x] 2.1 In `interpreter.cpp` fai derivare `pcodeComprAppend` e `pcodeForeachInit` da `ipcode` e
      passa `atoi(s)` nei due `case` di `makePCode()`. `pcodeComprAppend::exec()`: operando `0` come
      oggi; `2` → `storeslice(chiave,valore)` sul dizionario in `sp-3`, due pop (design D2).
      Verifica: `make` senza warning nuovi e `./runtest` passa (p-code esistente, operando 0).
- [x] 2.2 Aggiungi `zipIter` accanto a `dictIter` (design D3): due `iterObj*` interni,
      `next()` che restituisce valore della prima e "chiave" = valore della seconda, errore di
      lunghezze diverse con `forErrore()` se ne finisce una sola, `haChiave()` true, `mark()`/
      `expand()` sui due interni. In `pcodeForeachInit::exec()` estrai la creazione dell'iteratore
      in una funzione e con operando `2` combina i due iteratori in cima allo stack in un `zipIter`
      (un pop). Verifica: `make` senza warning nuovi e `./runtest` passa.
- [x] 2.3 Prova i due operandi con un listato `asm` scritto a mano nello scratchpad (la sequenza D1
      del design, forma parallela su `[1,2]` e `["a","b"]`, con `VAR` delle due variabili dopo
      l'header, e `PRINT` del risultato) convertito in binario con `./asm`. Verifica:
      `./interpreter` stampa un dizionario con `a:1` e `b:2` ed esce 0; con `["a"]` come seconda
      collezione stampa l'errore di lunghezze diverse ed esce ≠ 0.

## 3. Compilatore e grammatica

- [x] 3.1 In `parser.c` aggiungi `int n` a `foreachloop` e fai emettere a `foreachTesta()`
      `emit(P_FOREACH_INIT)` per `n==1` ed `emit_i(P_FOREACH_INIT,2)` per `n==2`; in `cpl2.gram`
      imposta `N.n=1` nelle due regole `foreach_in`. Verifica: dopo `lemon cpl2.gram && make`, i
      `.pcd` di tutti i sorgenti del task 1.1 ricompilati sono identici con `cmp` a quelli di
      riferimento.
- [x] 3.2 In `cpl2.gram` introduci `lbr` in `dict_const` e sostituisci le quattro regole
      `dict_compr` a due variabili con `dict_compr`/`dcompr_head`/`dcompr_src`/`dcompr_in`/
      `dcompr_zip` del design D4; lascia commentate le due a una variabile e aggiorna il commento
      della forma parallela (in italiano). Rigenera `cpl2.c`/`cpl2.h` con `lemon cpl2.gram`.
      Verifica: `lemon` esce 0, `cpl2.out` non riporta conflitti, `make` ricompila `compiler`, i
      `.pcd` del task 1.1 restano identici con `cmp` tranne `q.pcd` (che ora contiene il ciclo al
      posto del `COMMENT`), e `./runtest` passa.
- [x] 3.3 Compila nello scratchpad `? {k:x for each x,k in [1,2,3] with x>1}` e
      `? {k:x for each x:k in [1] : ["a"] with x>0}` (con le variabili dichiarate) e leggili con
      `./asm`. Verifica: tolte le righe `LINE`, i listati coincidono con la sequenza D1 del design;
      contengono `DICT 0`, `COMPR_APPEND 2` (e `FOREACH_INIT 2` nel secondo), nessun `COMMENT`;
      `./asm` sul listato riconverte il binario identico (`cmp`).
- [x] 3.4 Compila `? {x:x for each x in [1,2]}`. Verifica: il compilatore stampa `syntax error` ed
      esce ≠ 0.

## 4. Test e documentazione

- [x] 4.1 Crea `test/dict_compr.cpl` + `test/dict_compr.out` con gli scenari non di errore di
      `specs/dict-comprehension/spec.md`, stampando solo dizionari con al massimo una chiave o
      leggendo i valori per chiave: indice come chiave, chiave e valore calcolati, dizionario come
      collezione, assegnazione a `dict[int]`, collezione vuota, chiave intera letta con stringa,
      chiave ripetuta, forma parallela (esempio di `q.cpl` con `k` dichiarata `str`, tupla in
      parallelo, due vuote), `with` su entrambe le forme e mai vero, collezione da func valutata una
      volta, ordine chiave/valore con una func che stampa, variabile dopo la comprehension,
      annidamento nel valore, comprehension su due righe, e una forma parallela su due array di
      qualche migliaio di elementi costruiti con array comprehension che allocano stringhe (oltre
      `GC_OBJSLIM`) stampandone solo il numero di coppie o un valore per far partire il GC.
      Verifica: `./runtest` riporta `dict_compr` fra i test superati (gira con `--typecheck`).
- [x] 4.2 Casi d'errore a mano nello scratchpad (non nel suite, terminano con exit ≠ 0):
      `{k:v for each v,k in |1,2|}`, `{k:x for each x,k in 5}`, `{k:x for each x:k in [1,2] :
      ["a","b","c"]}`, `d:={k:x for each x:k in [1,2,3] : ["a"]}` seguito da `? d`, variabile non
      dichiarata `{z:1 for each y:z in [1] : [2]}`, `with` non booleano. Verifica: ognuno stampa
      l'errore atteso (del `for each`, di lunghezze diverse, o con il nome della variabile) ed esce
      ≠ 0 senza stampare un dizionario.
- [x] 4.3 Esegui `./compiler q.cpl && ./interpreter q` (`q.cpl` non va modificato). Verifica: exit 0
      e, dopo `dict comprehension`, un dizionario con `0:1`,`1:2`,`2:3` e uno con
      `alfa:1`,`beta:2`,`gamma:3`.
- [x] 4.4 Aggiorna `AGENTS.md` (in inglese, come il resto del file): togli le dict comprehension
      dai costrutti che emettono `P_COMMENT`, aggiungi un paragrafo accanto a quello delle array
      comprehension con le due forme, la sequenza di p-code, `COMPR_APPEND 2`, `FOREACH_INIT 2`
      (iteratore parallelo, errore di lunghezze diverse) e il fatto che `foreachTesta()` è condivisa
      anche dalle dict comprehension. Verifica: `grep -n "dict comprehension" AGENTS.md` non trova
      più la frase "likewise unimplemented" e trova il nuovo paragrafo.

## Workflow follow-up

- Commit (messaggio in italiano, file aggiunti esplicitamente, mai `git add -A`): `cpl2.gram`,
  `cpl2.c`, `cpl2.h`, `parser.c`, `interpreter.cpp`, `test/dict_compr.cpl`, `test/dict_compr.out`,
  `AGENTS.md`, `openspec/`. Nota: `cpl2.gram` ha già modifiche non committate dell'utente (le due
  regole a una variabile commentate), che entrano nello stesso commit.
- Archiviare la change con `/opsx:archive` dopo la verifica.
