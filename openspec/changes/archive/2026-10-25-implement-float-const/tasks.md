# Tasks

## 1. floatObj

- [x] 1.1 In `interpreter.cpp`, porta `floatObj::value` e il costruttore da `float` a `double`
      (design D1) e riscrivi `print()` con `std::to_chars` in forma minima più `.0` quando il
      risultato non ha né `.` né `e` né lettere (design D3); aggiungi `#include <charconv>`.
      Verifica: `make` completa senza errori né warning nuovi.
- [x] 1.2 Dichiara `getBaseType()` in `floatObj` e definiscilo dopo `theFloatType`, accanto a
      `intObj::getBaseType` (design D5). Verifica: `make` completa.
- [x] 1.3 Ridefinisci `uminus()` in `floatObj` restituendo sempre `new floatObj(-value)`, mai
      `this` modificato (design D4). Verifica: `make` completa; il comportamento si prova al
      punto 2.2.

## 2. Il p-code FLOAT_CONST

- [x] 2.1 Aggiungi `pcodeFloatConst` come `spcode` con un `lockgc_ptr<obj> theValue` costruito da
      `strtod` dell'operando, azzerato nel distruttore, `exec()` che spinge `theValue` e
      `getObjValue()` (sullo schema di `pcodeIntConst`/`pcodeStrConst`, design D2/D4), e
      `case P_FLOAT_CONST:return new pcodeFloatConst(s);` in `makePCode()`. Non toccare
      `pcodejump[]`, grammatica, `pcodes.h`/`pcodes.c`, `asm.c`. Verifica: `make` completa.
- [x] 2.2 Prova a mano nello scratchpad gli scenari di `specs/interpreter-pcodes/spec.md`:
      `? 3.14`, `? 1_000.25`, `? 3.140`, `? 2.0`, `? 7.`, `? 0.1`, `? -2.5`, `? - -0.75`,
      `? (3.14 is float)`, `? (3.14 is int)`, `? (-2.5 is float)`, `var float x := 0.5` + `? x`
      con `--typecheck`. Verifica: `./compiler` esce 0, `./interpreter --typecheck` stampa
      `3.14`, `1000.25`, `3.14`, `2.0`, `7.0`, `0.1`, `-2.5`, `0.75`, `true`, `false`, `true`,
      `0.5` ed esce 0 (nessuna violazione di `assert(sp==-1)`).
- [x] 2.3 Prova a mano `? -'a'` e `? 1.5 + 1.5`. Verifica: il primo termina con
      `uminus not implemented`, il secondo con `plus not implemented` (l'aritmetica float resta
      fuori scope); nessuno dei due stampa `not impl!`.

## 3. Test e documentazione

- [x] 3.1 Crea `test/float.cpl` + `test/float.out` che coprano gli scenari di successo della spec:
      costanti e forma di stampa (`3.14`, `3.140`, `2.0`, `7.`, `0.1`, `1_000.25`), `is float` /
      `is int`, `var float` assegnata e stampata, meno unario (costante, doppia negazione,
      variabile negata e poi ristampata invariata) e un ciclo `while` che esegue tre volte
      `? -1.5` e `? 1.5`. Usa `?`, mai `print -...`. L'`.out` va scritto a mano dai valori attesi
      della spec, non copiato dall'output dell'interprete. Verifica: `./runtest` esce 0 e riporta
      `float` fra i test completi passati.
- [x] 3.2 In `AGENTS.md`, sezione "Don't assume a construct runs": togli le costanti float
      dall'elenco di ciò che compila e poi esplode (restano date/time/tuple), aggiorna il conteggio
      dei pcode implementati (76 su 116, 40 a `pcodeNotImpl`) e aggiungi una riga che dice che le
      costanti float girano (stampa in forma minima con `.0`, `UMINUS` sì, aritmetica e confronti
      no). Verifica: il conteggio corrisponde ai `case` di `makePCode()`
      (`grep -c "case P_" interpreter.cpp` nella funzione) e il testo non contraddice la spec.
- [x] 3.3 Verifica che nessun altro test sia cambiato: `make && ./runtest` esce 0 con lo stesso
      numero di test passati di prima più uno, e `git status` mostra solo `interpreter.cpp`,
      `AGENTS.md`, `test/float.cpl`, `test/float.out` (oltre ai file non tracciati già presenti).

## Workflow follow-up

- Commit in italiano aggiungendo esplicitamente `interpreter.cpp`, `AGENTS.md`,
  `test/float.cpl`, `test/float.out` e la cartella del change (mai `git add -A`).
- Archiviare il change con `/opsx:archive` dopo la revisione.
