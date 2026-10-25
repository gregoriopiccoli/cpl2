# Tasks

## 0. Riferimento prestazioni

- [x] 0.1 Prima di toccare il codice, `make` e misura `fib` con `./compiler fib.cpl &&
      ./interpreter -b fib.pcd` annotando il tempo per iterazione di nanobench (ignora l'exit
      code: il bug noto di `benchPcd()` fa uscire 1 anche quando misura bene). Verifica: il
      numero è annotato per il confronto del punto 3.1.

## 1. Operazioni fra float

- [x] 1.1 In `interpreter.cpp`, dopo la definizione di `theTrue`/`theFalse` e di `floatObj`, aggiungi
      le funzioni libere sui `double` per `+ - * /` (restituiscono sempre `new floatObj(...)`) e per
      `= < <= >= > <>` (restituiscono `theTrue`/`theFalse`) (design D2, D3). Verifica: `make`
      completa senza errori né warning nuovi.
- [x] 1.2 Aggiungi a `floatObj` l'helper `check_num()` (float → `value`, int → `(double)value`,
      altro → `domain_error` col messaggio `float <op> with a non number`) e gli override `plus`,
      `minus`, `mult`, `div`, `eq`, `lt`, `le`, `ge`, `gt`, `ne` che chiamano le funzioni del
      punto 1.1; non ridefinire `mod` né `idiv` (design D1, D4). Verifica: `make` completa.
- [x] 1.3 Prova a mano nello scratchpad, con `./interpreter --typecheck`, gli scenari di successo di
      "Aritmetica fra float", "Divisione float per zero" e "Confronti fra float" in
      `specs/interpreter-pcodes/spec.md`, compreso `float op int` (`? 0.5 + 1`, `? 3.0 / 2`,
      `? 2.5 > 3`, `? 1.0 <> 1`). Verifica: ogni riga stampa esattamente il valore della spec
      (`3.0`, `2.0`, `3.0`, `0.25`, `0.30000000000000004`, `inf`, `-inf`, ...) e il programma
      esce 0.
- [x] 1.4 Prova a mano gli errori con float a sinistra: `? 1.5 + 'a'`, `? 1.5 = nil`, `? 5.5 mod 2`.
      Verifica: ognuno termina con un errore di runtime (rispettivamente
      `float + with a non number`, `float = with a non number`, `mod not implemented`) senza
      stampare un valore per quella riga.
- [x] 1.5 Estendi `test/float.cpl` e `test/float.out` con una sezione `-- aritmetica e confronti --`:
      `+ - * /` fra float, `0.1 + 0.2`, `(1.5 + 1.5 is float)`, operandi non modificati
      (`var float a:=1.5`, `? a + a`, `? a * a`, `? a`), divisione per zero (`inf`, `-inf`, e il
      NaN solo attraverso `(z = z)`/`(z <> z)`, mai stampato direttamente), i sei confronti, un
      ciclo `while x < 1.0` con `x := x + 0.25` e un ciclo che esegue tre volte `? 0.5 + 0.25`.
      L'`.out` va scritto a mano dai valori della spec, non copiato dall'output dell'interprete.
      Verifica: `./runtest` esce 0 con `float` fra i test completi passati.

## 2. Promozione negli operandi misti con l'intero a sinistra

- [x] 2.1 In `intObj::plus`, `minus`, `mult`, `div`, `eq`, `lt`, `le`, `ge`, `gt`, `ne` lascia il
      controllo intero come primo e unico test del caso `int op int` (codice e riciclo da
      `theIntCache` invariati); solo quando l'operando non è intero e `getBaseType()==theFloatType`
      chiama la funzione libera del punto 1.1 con `(double)value`; altrimenti lancia il messaggio di
      oggi, invariato. `mod` e `idiv` non cambiano (design D2, D4). Verifica: `make` completa.
- [x] 2.2 Prova a mano gli scenari di "Promozione a float negli operandi misti" e "Le operazioni fra
      interi non cambiano" (`? 1 + 0.5`, `? 2 * 1.5`, `? (2 * 1.5 is float)`, `? 1 / 2.0`,
      `? 1 < 1.5`, `? 1 = 1.0`, `? 7 / 2`, `? (7 / 2 is int)`, `? (1 + 2 is float)`). Verifica:
      l'output è quello della spec (`1.5`, `3.0`, `true`, `0.5`, `true`, `true`, `3`, `true`,
      `false`).
- [x] 2.3 Prova a mano gli errori con l'intero o un altro tipo a sinistra: `? 5 div 2.0`,
      `? 5 mod 2.0`, `? 'a' < 1.5`, `? 1 + 'a'`. Verifica: ognuno termina con un errore di runtime;
      `1 + 'a'` riporta ancora `integer + with a non integer`.
- [x] 2.4 Prova a mano con `--typecheck` che la promozione resti confinata agli operatori:
      `var int i := 1 + 0.5` viene rifiutato, `var float x := 1` viene ancora rifiutato, e
      `switch 1.0` con `case 1` va nell'`else` (design D5). Verifica: i due assegnamenti
      interrompono l'esecuzione col messaggio del controllo di tipo, lo `switch` stampa il ramo
      `else`.
- [x] 2.5 Aggiungi a `test/float.cpl`/`test/float.out` una sezione `-- operandi misti --` con i casi
      di successo del punto 2.2 (entrambi gli ordini, aritmetica e confronti) e i due controlli
      `int op int` (`7 / 2` e `is int`). Verifica: `./runtest` esce 0.
- [x] 2.6 In `AGENTS.md`, sezione "Don't assume a construct runs", riscrivi il paragrafo sui float:
      `+ - * /` e i sei confronti funzionano su float e su operandi misti `int`/`float` (l'int viene
      promosso, il risultato è float; `int op int` invariato, `/` fra interi resta intera);
      divisione per zero IEEE (`inf`, `nan`); `mod`/`div` con un float sono ancora un errore; la
      promozione vale solo negli operatori (`var float x := 1` resta rifiutato da `--typecheck`).
      Verifica: il testo non contraddice la spec e non dice più che ogni operazione float diversa
      da `UMINUS` lancia `... not implemented`.

## 3. Verifica d'insieme

- [x] 3.1 Rimisura `fib` come al punto 0.1. Verifica: il tempo per iterazione non è peggiorato oltre
      il rumore di misura (qualche percento); se lo è, rivedi l'ordine dei controlli in `intObj`
      prima di proseguire.
- [x] 3.2 `make && ./runtest`. Verifica: esce 0 con lo stesso numero di test di prima (nessun test
      nuovo: `float` è esteso), e `git status` mostra modificati solo `interpreter.cpp`,
      `AGENTS.md`, `test/float.cpl`, `test/float.out` (oltre ai non tracciati già presenti e alla
      cartella del change).

## Workflow follow-up

- Commit in italiano aggiungendo esplicitamente `interpreter.cpp`, `AGENTS.md`,
  `test/float.cpl`, `test/float.out` e la cartella del change (mai `git add -A`).
- Archiviare il change con `/opsx:archive` dopo la revisione.
