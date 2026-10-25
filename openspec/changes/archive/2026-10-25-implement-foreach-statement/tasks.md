# Tasks

## 1. Iteratori e p-code del for each (nessun cambio di grammatica)

- [x] 1.1 In `pcodes.h` aggiungi `P_FOREACH_INIT 112`, `P_FOREACH_VAR 113`, `P_FOREACH_KEY 114`,
      `P_FOREACH_NEXT 115` accanto ai `P_FOR_*` e porta `MAX_PCODE` a 115; in `pcodes.c` i quattro
      `pcodetxt[...]` in `initpcodetxt()`. Verifica: `touch asm.c && make` completa e
      `grep -n 'P_FOREACH_' pcodes.h pcodes.c` mostra le otto righe.
- [x] 1.2 In `interpreter.cpp` aggiungi `iterObj` (astratta) e `arrayIter`, `tupleIter`,
      `dictIter` dopo `dictObj` (design D2/D3/D4): array e tuple con posizione viva confrontata con
      la lunghezza attuale, chiave `int` da `theIntCache` per gli array, nessuna chiave per le
      tuple; dizionari con copia delle chiavi alla creazione e valore letto al momento, chiave
      `strObj`; `mark()`/`expand()` visitano la collezione. Servono accessori di sola
      lettura su `arrayObj`/`tupleObj`/`dictObj` (lunghezza, elemento i-esimo, elenco delle chiavi,
      ricerca senza inserire).
      Verifica: `make` senza warning nuovi.
- [x] 1.3 Aggiungi le classi `pcodeForeachInit` (`pcode`, senza operando), `pcodeForeachVar` e
      `pcodeForeachKey` (`interningpcode`), `pcodeForeachNext` (`ipcode`) accanto ai `pcodeFor*`,
      con i quattro `case` in `makePCode()`; non in `pcodejump[]`. `exec()` secondo design D1/D2/D5:
      init sostituisce la cima con l'iteratore (errore se non iterabile); var/key controllano
      `isLocal()` e registrano il nome (key: errore su tupla); next salta alla label se finito,
      altrimenti `context->store` di valore e chiave; stack invariato. Errori con `forErrore()`.
      Verifica: `make` senza warning nuovi.
- [x] 1.4 Prova i p-code isolati con listati `asm` scritti a mano nello scratchpad (dichiarazioni
      con `VAR`, poi la sequenza del design D1 con `PRINT`) convertiti in binario con `./asm`:
      array con e senza chiave, array vuoto, dizionario con chiave, tupla, `FOREACH_INIT` su `NIL`,
      `FOREACH_KEY` su tupla, `FOREACH_VAR` su nome non dichiarato. Verifica: le sequenze stampate
      sono quelle attese, i casi d'errore stampano il messaggio e terminano con codice ≠ 0, i casi
      buoni terminano con 0 senza violare `assert(sp==-1)`.

## 2. Grammatica e test

- [x] 2.1 In `parser.c` aggiungi la struct `foreachloop` accanto a `forloop`; in `cpl2.gram`
      sostituisci le quattro regole `foreach_stm` con `foreach_stm`/`foreach_head`/`foreach_src`/
      `foreach_in` del design D6 (le comprehension restano `COMMENT`). Rigenera con
      `lemon cpl2.gram`. Verifica: `lemon` esce 0, `cpl2.out` non riporta conflitti, `make`
      ricompila `compiler`.
- [x] 2.2 Compila gli esempi 1 e 2 del design D1 (con `var int i,k` e `var array[int] a` prima) e
      confronta con `./asm`. Verifica: i listati, tolte le righe `LINE`, coincidono con quelli del
      design.
- [x] 2.3 Crea `test/foreach.cpl` + `test/foreach.out` con gli scenari *non di errore* di
      `specs/foreach-statement/spec.md`: array con e senza indice, array vuoto, dizionario con e
      senza chiave (output indipendente dall'ordine: dizionari a una chiave per le stampe, somme e
      conteggi per quelli a più chiavi), dizionario vuoto, chiave non stringa, chiave aggiunta nel
      corpo non visitata, valore cambiato prima della visita, tupla, `with` sul valore e sulla chiave, collezione riassegnata nel
      corpo, collezione da funzione che stampa una volta, assegnazione all'indice nel corpo,
      variabili dopo il ciclo completo e dopo quello vuoto, `for each` annidati, dentro un `for` e
      in una procedura chiamata due volte, un ciclo su `[1,2,3]` con un corpo che alloca molto
      (stringhe concatenate) per far partire il GC. Verifica: `./runtest` riporta `foreach` fra i
      test superati.
- [x] 2.4 Crea `test/foreach_err.cpl` senza `.out` (sola compilazione, come `test/for_err.cpl`)
      con i casi rifiutati commentati con l'esito atteso: tupla con chiave, `int` e `str` come
      collezione, variabile del valore non dichiarata, chiave visibile solo dal modulo dentro una
      procedura, `with` non booleano. Verifica: `./runtest` lo conta come superato; a mano, nello
      scratchpad, un sorgente per caso compilato ed eseguito stampa il messaggio (che nomina la
      variabile nei casi di contesto) e termina con codice ≠ 0 senza eseguire il corpo; il caso
      `--typecheck` con `var int i` su `['a']` stampa l'errore di tipo con `i`.
- [x] 2.5 Esegui gli esempi di `q.cpl` (non tracciato, già pronto con i cinque `for each`).
      Verifica: `./compiler q.cpl && ./interpreter q` esce 0 e stampa le sequenze dello scenario
      "Esempi di q.cpl" della spec (le righe di `d` in un ordine qualsiasi).

## 3. Documentazione e non-regressione

- [x] 3.1 Aggiorna `AGENTS.md`: conteggio dei p-code definiti/implementati in "Don't assume a
      construct runs" (ricontali da `pcodes.h` e `makePCode()`), togli `for each` dall'elenco dei
      `P_COMMENT` (restano le comprehension), aggiungi un paragrafo sul `for each` come quello del
      `for` (forma del p-code, variabili locali, iteratore sullo stack fino al `POP` finale — un
      futuro `break`/`return` dovrà toglierlo, ordine dei dizionari non specificato e chiavi copiate all'inizio, chiave `str`).
      Verifica: i numeri in `AGENTS.md` coincidono con il conteggio reale.
- [x] 3.2 Non-regressione: `make` completo e `./runtest` con codice di uscita 0; `cpl2.c` e
      `cpl2.h` rigenerati inclusi così come escono da `lemon`, nessun `.pcd` aggiunto a git.
      Verifica: `git status` mostra modificati solo i sorgenti attesi (`pcodes.h`, `pcodes.c`,
      `parser.c`, `cpl2.gram`, `cpl2.c`, `cpl2.h`, `interpreter.cpp`, `AGENTS.md`) e i nuovi
      `test/foreach.cpl`, `test/foreach.out`, `test/foreach_err.cpl`.

## Workflow follow-up

- Dopo la revisione: `openspec archive implement-foreach-statement` per fondere la delta in
  `openspec/specs/interpreter-pcodes/` e creare `openspec/specs/foreach-statement/`.
- I generatori (design D7) sono un change separato, da proporre dopo la revisione di questo.
