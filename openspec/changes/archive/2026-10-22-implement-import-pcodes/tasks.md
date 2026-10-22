# Tasks

## 1. Esecuzione dei tre p-code e test completo

- [x] 1.1 In `interpreter.cpp` aggiungi le classi `pcodeImportId : spcode`,
      `pcodeImportIdx : spcode` e `pcodeImport : ipcode` con i rispettivi `exec()`, e i tre
      `case P_IMPORTID`, `P_IMPORTIDX`, `P_IMPORT` in `makePCode()`; non aggiungerli a
      `pcodejump[]`. Verifica: `make` completa senza errori e una ricerca in `makePCode()`
      mostra i tre `case`.
- [x] 1.2 Implementa gli effetti: `IMPORTID` spinge `new strObj(value)`; `IMPORTIDX` spinge
      l'identificatore, verifica che le due stringhe in cima siano `strObj` e le sostituisce
      con `precedente + "." + identificatore` lasciando la profondità invariata; `IMPORT`
      prende N dall'operando, verifica che le N posizioni in cima siano `strObj`, stampa i
      nomi dal basso verso la cima con `cout << o->print() << endl`, poi li rimuove. Verifica:
      un sorgente temporaneo con `import a` e `import a.b.c` compilato ed eseguito stampa
      `a` e `a.b.c`, e il processo termina con 0 senza violare `assert(sp==-1)`.
- [x] 1.3 Crea `test/import_stack.cpl` con `import a`, `import a.b.c`, `import a.b,c.d` e una
      stampa finale, e `test/import_stack.out` con esattamente `a`, `a.b.c`, `a.b`, `c.d` e la
      riga finale. Verifica: `./runtest` include `import_stack` fra i test superati e
      `test/simple_import.cpl` resta senza `.out`.

## 2. Documentazione e non-regressione

- [x] 2.1 Aggiorna `AGENTS.md` nella sezione "Don't assume a construct runs": togli `import`
      dall'elenco dei costrutti che esplodono sempre a runtime, specifica che `IMPORTID`,
      `IMPORTIDX` e `IMPORT` ora costruiscono i nomi sullo stack e che `IMPORT` li stampa
      (N dall'operando), che l'inclusione vera non c'è ancora e che `from`/`FROM` resta non
      implementato. Verifica: la sezione non elenca più `import` fra i p-code non
      implementati e cita esplicitamente il limite.
- [x] 2.2 Verifica che il perimetro sia rispettato: `git diff -- pcodes.h pcodes.c cpl2.gram
      cpl2.c cpl2.h asm.c` è vuoto, `MAX_PCODE` è ancora 105 e non compaiono nuovi file
      generati da lemon.
- [x] 2.3 Esegui la non-regressione completa: `make` senza errori e `./runtest` con codice di
      uscita 0; verifica che `simple_import` sia ancora trattato come test di sola
      compilazione e che `runtest` non lasci nuovi `.pcd` in `test/` (i `.pcd` preesistenti
      restano intatti).

## Workflow follow-up

- Dopo la revisione: `openspec archive implement-import-pcodes` per fondere la delta nella
  spec `interpreter-pcodes`, poi verificare con `openspec show interpreter-pcodes --type spec`
  che i nuovi requisiti siano presenti.
