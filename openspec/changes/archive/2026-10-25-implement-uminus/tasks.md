# Tasks

## 1. intObj::uminus

- [x] 1.1 In `interpreter.cpp`, nella classe `intObj`, dichiara
      `virtual obj* uminus() const override;` accanto a `minus`, e definiscilo subito dopo
      `intObj::minus` con lo stesso schema di riciclo da `theIntCache` (design D2), calcolando il
      valore come `(int)(0u-(unsigned)value)` (design D3) e restituendo sempre un oggetto nuovo,
      mai `this` modificato (design D1). Non toccare `pcodeUMinus`, `makePCode()` né `pcodejump[]`.
      Verifica: `make` completa senza errori né warning nuovi.
- [x] 1.2 Prova a mano nello scratchpad `? -5`, `? - -7`, `? -0`, `? -(2+3)`, `? 3 - -2`,
      `? -2*3`. Verifica: `./compiler` esce 0 e `./interpreter` stampa `-5`, `7`, `0`, `-5`, `5`,
      `-6` ed esce 0 (nessuna violazione di `assert(sp==-1)`).
- [x] 1.3 Prova a mano `? -'a'` e `? -true`. Verifica: l'interprete termina con
      `uminus not implemented` su stderr senza stampare un valore per quella riga (scenario
      "Meno unario su una stringa"; non entra nel test completo, vedi design Risks).

## 2. Test completo

- [x] 2.1 Crea `test/uminus.cpl` + `test/uminus.out` che coprano gli scenari di successo di
      `specs/interpreter-pcodes/spec.md`: costante, doppia negazione, zero, espressione tra
      parentesi, `3 - -2`, `-2*3`, variabile negata e poi ristampata invariata
      (`var int i:=4`, `? -i`, `? i`), e un ciclo `while` che esegue tre volte `? -5`. Usa `?`, mai
      `print -...` (design Risks). L'`.out` va scritto a mano dai valori attesi della spec, non
      copiato dall'output dell'interprete. Verifica: `./runtest` esce 0 e riporta `uminus` fra i
      test completi passati.
- [x] 2.2 Verifica che nessun altro test sia cambiato: `make && ./runtest` esce 0 con lo stesso
      numero di test passati di prima più uno, e `git status` mostra solo `interpreter.cpp`,
      `test/uminus.cpl`, `test/uminus.out` (oltre ai file non tracciati già presenti).

## Workflow follow-up

- Commit in italiano aggiungendo esplicitamente `interpreter.cpp`, `test/uminus.cpl`,
  `test/uminus.out` e la cartella del change (mai `git add -A`).
- Archiviare il change con `/opsx:archive` dopo la revisione.
