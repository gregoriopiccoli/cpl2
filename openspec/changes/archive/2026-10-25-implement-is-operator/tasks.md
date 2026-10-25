# Tasks

Nota: `interpreter.cpp` e `openspec/specs/interpreter-pcodes/spec.md` hanno già modifiche non
committate nel working tree: non toccarle e non includerle nei commit di questo change.

## 1. Tipo float

- [x] 1.1 In `interpreter.cpp` aggiungi `class pcodeFloatType` (dichiarazione accanto a
      `pcodeBoolType`, `exec` copia di `pcodeBoolType::exec` con `theFloatType`) e
      `case P_FLOAT_TYPE:return new pcodeFloatType();` in `makePCode()` (design D6). Verifica: `make`
      completa; nello scratchpad `var float x` seguito da `? x` compila, gira con
      `./interpreter --typecheck` e stampa `nil` senza `not impl!`.

## 2. Accessori in sola lettura

- [x] 2.1 Aggiungi i getter const di design D2: `arrayType::elem()`, `dictType::elem()`,
      `tupleType::tipi()`, `arrayObj::elementi()`, `tupleObj::elementi()` e l'accesso ai valori di
      `dictObj`, con un commento breve in italiano come quello di `orType::alts()`. Nessun altro
      codice cambia. Verifica: `make` completa e `./runtest` esce 0 come prima.

## 3. Operatore is

- [x] 3.1 Scrivi `tipoIs(tipo,valore)` subito dopo `tipoCompatibile` (design D1, D3, D4, D5), con
      un blocco di commento in italiano che dica in cosa differisce dal check di assegnazione (nil
      solo in `any`, contenitori in profondità, guardia sui cicli). `tipoCompatibile` non cambia.
      Verifica: `git diff interpreter.cpp` non mostra righe modificate dentro `tipoCompatibile`.
- [x] 3.2 Aggiungi `class pcodeIs` e il suo `exec` (design D6) e `case P_IS:return new pcodeIs();`
      in `makePCode()`. Non toccare `pcodejump[]`. Verifica: `make`; nello scratchpad `? (1 is int)`
      e `? is("a",int)` stampano `true` e `false` con `./interpreter --typecheck`.
- [x] 3.3 Crea `test/is.cpl` + `test/is.out` che coprano tutti gli scenari di successo di
      `specs/is-operator/spec.md` e lo scenario `var float x` / `(x is float)` di
      `specs/interpreter-pcodes/spec.md`, entrambe le forme sintattiche, un `if` con `is` come
      condizione, una variabile `int` che vale `nil`, e un array che contiene se stesso
      (`var array[?] a := [1]`, `a[0] := a`, `? (a is array[array[?]|int])` → `true`, verificando che
      termini). In testa al file, in un commento come in `type_check.cpl`, annota il caso d'errore
      verificato a mano (`var int mytype := 3` / `? (1 is mytype)`). Scrivi l'`.out` a mano dagli
      scenari, non copiandolo dall'interprete. Verifica: `./runtest` esce 0 e riporta `is` fra i
      test completi passati.
- [x] 3.4 Verifica a mano lo scenario "Tipo non riconosciuto": nello scratchpad
      `var int mytype := 3` / `? (1 is mytype)` termina con exit diverso da 0, con il messaggio di
      design D4 e senza stampare `true`/`false`; la sequenza `INT_CONST 1`, `LOAD mytype`, `IS` è
      quella che mostra `./asm`.
- [x] 3.5 In `AGENTS.md`, sezione "Don't assume a construct runs": togli `is` dall'elenco dei
      costrutti non implementati, correggi il conteggio "73 of the 116" (+2 implementati, -2 in
      `pcodeNotImpl`; ricontalo da `makePCode()`), precisa che i tipi `float` ora girano mentre le
      costanti float no, e aggiungi un paragrafo breve su `is` (le due forme, nil solo in `any`,
      controllo profondo, cicli, errore sui tipi non riconosciuti). Verifica: il conteggio coincide
      con `grep -c 'case P_' ` dentro `makePCode()` e le frasi descrivono il comportamento osservato
      ai task 3.2–3.4.

## 4. Integrazione

- [x] 4.1 `make && ./runtest` esce 0 con un test completo in più rispetto a prima, e
      `git status` mostra come nuovi/modificati solo `interpreter.cpp`, `AGENTS.md`, `test/is.cpl`,
      `test/is.out` oltre a quanto era già presente nel working tree.

## Workflow follow-up

- Commit in italiano aggiungendo esplicitamente i file del change e la sua cartella (mai
  `git add -A`); per `interpreter.cpp`, che ha modifiche preesistenti, includere solo gli hunk di
  questo change (`git add -p`) o chiedere prima all'utente.
- Archiviare il change con `/opsx:archive` dopo la revisione.
