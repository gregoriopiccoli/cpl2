# Tasks

## 1. Correggere il ripristino del programma dopo chiamata tra moduli

- [x] 1.1 Cambiare `interp::prg` da `pcodeProgram&` a `pcodeProgram*` e aggiornare il costruttore di `interp` per inizializzarlo con `&p`. Verifica: `make` compila senza errori di tipo per il membro.
- [x] 1.2 Aggiornare tutti gli accessi a `prg` in `interpreter.cpp` da `prg[...]`, `prg.` a `(*prg)[...]`, `prg->` (inclusi `pcode*::exec`, computed-goto e punti vari). Verifica: `make` compila senza errori.
- [x] 1.3 Correggere `procObj::call` per salvare `pcodeProgram* retPrg = interpreter.prg;` e ripristinarlo con `interpreter.prg = retPrg;`, assicurando salvataggio/ripristino di `context` e `pc` invariati. Verifica: `make` compila senza errori.

## 2. Allineare CLI all'uso documentato

- [x] 2.1 Modificare `main()` per usare `pcodePathFor(percorsi[0])` e `runPcd` in modalità normale (non help/test), e `benchPcd` con `pcodePathFor` in modalità benchmark. Mantenere il comportamento attuale per opzioni. Verifica: `./interpreter q.pcd` esegue correttamente e `./interpreter q` trova `q.pcd`.
- [x] 2.2 Verificare che `./interpreter -b q` misuri correttamente con `pcodePathFor`. Verifica: nessun messaggio "cannot open p-code file" per `q/pcd.pcd`.

## 3. Aggiungere e verificare i test q/q1/q2

- [x] 3.1 Copiare o creare `test/q.cpl`, `test/q1.cpl`, `test/q2.cpl` (contenuti dagli esemplari root) in `test/` e aggiungere `.out` per `q` e `q2` (test completi). Assicurare `q1` accessibile come modulo. Verifica: i file esistono in `test/`.
- [x] 3.2 Eseguire `make && ./runtest` e verificare che `q` e `q2` passino (test completi) e non ci siano segfault. Verifica: `./runtest` esce 0 e nomina i test superati.
- [x] 3.3 Eseguire manualmente `./compiler q.cpl q2.cpl && ./interpreter q` e `./interpreter q2` per confermare output completo e riga finale di `q2`. Verifica: output corrisponde a `.out` e nessun segfault.

## 4. Integrazione e regressione

- [x] 4.1 Eseguire `make && ./runtest` due volte consecutivamente. Verifica: esito identico, exit 0, nessun file nuovo in `test/`.
- [x] 4.2 Verificare regressioni sui test esistenti relativi a moduli (`import_stack`, `module_access`, `a_module`). Verifica: `./runtest` passa interamente.
