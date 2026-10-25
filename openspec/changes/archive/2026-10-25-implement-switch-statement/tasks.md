# Tasks

## 1. Il p-code CASE (nessun cambio di grammatica)

- [x] 1.1 In `pcodes.h` aggiungi `#define P_CASE 107` nella sezione `--- programma ---` e porta
      `MAX_PCODE` a 107; in `pcodes.c` aggiungi `pcodetxt[P_CASE]="CASE";` in `initpcodetxt()`.
      Verifica: `make` completa e `grep -n P_CASE pcodes.h pcodes.c` mostra le due righe.
- [x] 1.2 In `interpreter.cpp` aggiungi `class pcodeCase: public ipcode` accanto a `pcodeIfFalse`
      e `case P_CASE:return new pcodeCase(atoi(s));` in `makePCode()`; `exec()` toglie la
      costante, lascia il selettore, confronta i `getBaseType()` e solo se coincidono usa `eq()`,
      salta a `getLabelPos(value)` se uguali (design D1/D2). Non aggiungerlo a `pcodejump[]`.
      Verifica: `make` senza errori e warning nuovi.
- [x] 1.3 Prova `CASE` isolato scrivendo a mano, nello scratchpad, un listato `asm` con
      `INT_CONST 2`, `INT_CONST 2`, `CASE 0`, stampa "no", `LABEL 0`, `POP`, `PCODEEND 0`
      (e varianti `INT_CONST 3`, `STR_CONST 1`, `NIL` come selettore), convertito in binario con
      `./asm`. Verifica: i salti avvengono solo nel caso uguale dello stesso tipo, nessuna
      eccezione sui tipi diversi, e l'interprete termina con 0 senza violare `assert(sp==-1)`.

## 2. Grammatica dello switch e test completo

- [x] 2.1 In `cpl2.gram` sostituisci la sezione `--- switch statement ---` con le regole del
      design D4 (`switch_case_list`, `case_head`, `case_test`, `case_else`/`else_tok`,
      `case_const` che emette la costante, inclusa la forma `MINUS INT_CONST` che emette
      `INT_CONST -n`), togliendo il `COMMENT "da fare switch ..."`.
      Rigenera con `lemon cpl2.gram`. Verifica: `lemon` esce 0, `cpl2.out` non riporta
      conflitti, `make` ricompila `compiler`.
- [x] 2.2 Compila l'esempio del design D5 e confrontalo con `./asm`. Verifica: il listato
      (tolte le righe `LINE`) coincide con quello di D5 e l'esecuzione stampa `uno o due`.
- [x] 2.3 Crea `test/switch.cpl` + `test/switch.out` che coprano tutti gli scenari di
      `specs/switch-statement/spec.md`: case singolo e multiplo, selettori int/str/bool,
      costanti negative (`case -1`, `case 0,-2`), no fall-through, costante duplicata (vince il primo), else eseguito, nessun match senza
      else, solo else, case misti int/str, selettore `nil`, selettore funzione che stampa una
      sola volta, `switch` annidato dentro un case e dentro una funzione chiamata più volte.
      Verifica: `./runtest` riporta `switch` fra i test superati.
- [x] 2.4 Togli `/* da fare! ... */` attorno allo `switch` in `q.cpl` (non è in `test/`) e
      compila/esegui `test/test_stm1.cpl` solo con il compilatore. Verifica: `./compiler q.cpl &&
      ./interpreter q` stampa il ramo corretto, e `./compiler test/test_stm1.cpl` non segnala
      errori di sintassi sulle righe dello `switch` (eventuali errori a runtime di altri costrutti
      non implementati non sono in scope).

## 3. Documentazione e non-regressione

- [x] 3.1 Aggiorna `AGENTS.md`: conteggio dei p-code definiti/implementati nella sezione
      "Don't assume a construct runs" (ricontali da `pcodes.h` e `makePCode()`), e una riga che
      dica che lo `switch` è eseguibile tramite `CASE` (forma del p-code in breve). Verifica:
      i numeri in `AGENTS.md` coincidono con il conteggio reale.
- [x] 3.2 Non-regressione: `make` completo e `./runtest` con codice di uscita 0; `cpl2.c` e
      `cpl2.h` rigenerati inclusi nel commit così come escono da `lemon`, nessun `.pcd` nuovo
      aggiunto a git. Verifica: `git status` mostra solo i sorgenti attesi modificati
      (`pcodes.h`, `pcodes.c`, `cpl2.gram`, `cpl2.c`, `cpl2.h`, `interpreter.cpp`, `AGENTS.md`,
      `q.cpl` se tracciato, `test/switch.*`).

## Workflow follow-up

- Dopo la revisione: `openspec archive implement-switch-statement` per fondere la delta in
  `openspec/specs/interpreter-pcodes/` e creare `openspec/specs/switch-statement/`.
