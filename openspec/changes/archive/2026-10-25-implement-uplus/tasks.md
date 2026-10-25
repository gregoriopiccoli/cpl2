# Tasks

## 1. Grammatica

- [x] 1.1 In `cpl2.gram` sostituisci il commento `// {printf("UPLUS\n");} non serve!` sulla regola
      `fact ::= PLUS fact.` con un commento in italiano che spiega la convenzione: il `+` unario non
      emette p-code e non controlla il tipo (design D1, D2). Non aggiungere azioni alla regola.
      Subito dopo `case_const ::= MINUS INT_CONST(value).` aggiungi
      `case_const ::= PLUS INT_CONST(value).` con azione `{emit_s(P_INT_CONST,value);dispose(&value);}`
      (design D4). Verifica: `git diff cpl2.gram` mostra solo queste due modifiche (oltre a quelle
      già presenti nel working tree).
- [x] 1.2 Rigenera `cpl2.c`/`cpl2.h` con il `lemon` installato e ricostruisci (`make`). Verifica:
      lemon esce 0 senza segnalare conflitti (design Risks), `make` completa, e l'output generato
      si committa così com'è senza ritoccarlo.
- [x] 1.3 Nello scratchpad compila `? +5` e `? 5` in due file distinti. Verifica: `cmp` dei due
      `.pcd` non riporta differenze; `./asm` su `? +i` (con `var int i:=4`) mostra `LOAD i` seguito
      direttamente da `PRINT 1` (requisito "Il più unario non produce p-code").
- [x] 1.4 Nello scratchpad compila due `switch` identici salvo `case +1` contro `case 1`.
      Verifica: `cmp` dei due `.pcd` non riporta differenze e `./asm` mostra `INT_CONST 1` seguito
      direttamente da `CASE`. Poi compila uno `switch` con `case +'a'`. Verifica: il compilatore
      stampa `syntax error` su stderr ed esce con codice diverso da 0 (spec `switch-statement`,
      scenario "Più unario davanti a una costante non intera").

## 2. Test completo e documentazione

- [x] 2.1 Crea `test/uplus.cpl` + `test/uplus.out` che coprano gli scenari di successo di
      `specs/unary-plus/spec.md` e `specs/switch-statement/spec.md`: `? +5`, `? + +7`, `? -+3`,
      `? +-3`, `? 3 + +2`, `? +(2+3)*2`, `var int i:=4` con `? +i` e `? i`, `? +'a'`, `? +true`;
      uno `switch` con selettore 1 e rami `case 0`/`case +1`, e uno con selettore 2 e un ramo
      `case 0,+2`. Non toccare `test/switch.cpl` (design D3). Usa `?`, mai `print +...` (design
      Risks). Scrivi l'`.out` a mano dai valori attesi delle spec, non copiandolo dall'output
      dell'interprete. Verifica: `./runtest` esce 0 e riporta `uplus` fra i test completi passati.
- [x] 2.2 In `AGENTS.md`, nel paragrafo sullo `switch`, accanto a "`case -1` emits a single
      `INT_CONST -1`, not `UMINUS`" aggiungi che `case +1` emette `INT_CONST 1` (nessun p-code per
      il `+`, solo davanti agli interi). Verifica: la frase descrive il comportamento osservato al
      task 1.4.
- [x] 2.3 Verifica che nient'altro sia cambiato: `make && ./runtest` esce 0 con un test passato in
      più rispetto a prima, e `git status` mostra come nuovi/modificati solo `cpl2.gram`, `cpl2.c`,
      `cpl2.h`, `AGENTS.md`, `test/uplus.cpl`, `test/uplus.out` oltre a quanto era già presente nel
      working tree.

## Workflow follow-up

- Commit in italiano aggiungendo esplicitamente `cpl2.gram`, `cpl2.c`, `cpl2.h`, `AGENTS.md`,
  `test/uplus.cpl`, `test/uplus.out` e la cartella del change (mai `git add -A`).
- Archiviare il change con `/opsx:archive` dopo la revisione.
