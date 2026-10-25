# Tasks

## 1. La promozione int -> float nelle assegnazioni

- [x] 1.1 In `interpreter.cpp`, dopo la definizione di `tipoCompatibile()` (e con un prototipo
      accanto a quello di `tipoCompatibile` prima di `contextObj`), aggiungi
      `obj* adattaTipo(const obj* tipo,obj* valore)` secondo design D2/D3: fast path su
      `tipo==theNil`/`nullptr`/`theIntType`, poi solo un `intObj` non gia' compatibile e un tipo
      che accetterebbe un float (sonda `floatObj` statico passato a `tipoCompatibile`) diventa
      `new floatObj((double)v)`; in ogni altro caso il valore torna invariato. Commento in
      italiano come le funzioni vicine. Verifica: `make` compila senza errori.
- [x] 1.2 Chiama `adattaTipo` prima di `checkTipo` nei tre punti (design D4):
      `contextObj::add(intern,type,value)`, `contextObj::store()` (dentro il ramo con il tipo
      trovato) e `funcVars::store_result()`, memorizzando il valore restituito. Verifica: con
      `./compiler q.cpl`, `./interpreter -c q` stampa `10.12` dopo le prime righe invece di
      fermarsi su `f2: il valore 0 non e' compatibile con il tipo float`, e `./interpreter q`
      da' la stessa uscita.
- [x] 1.3 Verifica a mano dei rifiuti, snippet isolati in `tmp/` eseguiti con
      `./interpreter -c`: `var int i := 1.5` stampa
      `i: il valore 1.5 non e' compatibile con il tipo int`; `var float f := "a"` stampa
      `f: il valore a non e' compatibile con il tipo float`; entrambi escono con errore.
      Verifica anche `var array[float] a := [1, 2]` + `? a`: gli elementi restano interi.

## 2. Test e documentazione

- [x] 2.1 In `test/float.cpl` aggiungi una sezione `? '-- assegnazione di un int --'` con gli
      scenari validi della spec: `var float f := 0` + `? f` + `? (f is float)`;
      `f := 3` + `? f / 2`; `func float meta(float x)` chiamata con `meta(3)`;
      `func float uno()` con `result := 1`; `var float not g := 2`; `var float|str u := 1`;
      `var int|float w := 1` + `? (w is int)`; `var ? v := 1`; `f := nil` + `? f`.
      Rigenera `test/float.out` dal run reale con `./interpreter --typecheck` (esito 0) e
      controlla a occhio che le righe nuove siano quelle della spec (`0.0`, `true`, `1.5`,
      `1.5`, `1.0`, `2.0`, `1.0`, `1`, `true`, `1`, `nil`). Rimuovi il `.pcd` prodotto.
- [x] 2.2 In `test/type_check_err.cpl` aggiungi nel commento i due rifiuti di 1.3
      (`var int i := 1.5`, `var float f := "a"`) con il messaggio atteso. Verifica:
      `./compiler test/type_check_err.cpl` esce con 0 (poi rimuovi il `.pcd`).
- [x] 2.3 In `AGENTS.md` sostituisci la frase "Promotion happens only inside operators:
      `var float x := 1` and `var int i := 1 + 0.5` are rejected by `--typecheck`" con la regola
      nuova: un `int` assegnato a un nome il cui tipo richiede un float (`float`, `not float`,
      unione con `float` che non accetta gia' l'int) diventa un float, con e senza
      `--typecheck`, in variabili, parametri e `result`; non dentro i contenitori; `var int i
      := 1 + 0.5` resta rifiutato; un `for` su variabile `float` fallisce (design D5).
      Verifica: `grep -n "var float x := 1" AGENTS.md` non trova piu' la frase vecchia.
- [x] 2.4 `make && ./runtest`: tutti i test di `test/` superati ed exit 0. Poi un controllo
      di prestazioni a occhio: `./interpreter fib` (fib(34)) non piu' lento di prima in modo
      percepibile (design D3).

## Workflow follow-up

- Dopo la revisione: `openspec archive promote-int-to-float-assignment` per fondere i requisiti
  ADDED nella spec principale `type-checking`, poi verifica con
  `openspec show type-checking --type spec`.
