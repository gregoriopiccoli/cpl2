# Tasks

## 1. I p-code del for (nessun cambio di grammatica)

- [x] 1.1 In `pcodes.h` aggiungi nella sezione `--- programma ---` `P_FOR_TO 108`,
      `P_FOR_DOWNTO 109`, `P_FOR_TEST 110`, `P_FOR_NEXT 111` e porta `MAX_PCODE` a 111; in
      `pcodes.c` aggiungi i quattro `pcodetxt[...]` (`"FOR_TO"`, `"FOR_DOWNTO"`, `"FOR_TEST"`,
      `"FOR_NEXT"`) in `initpcodetxt()`. Verifica: `make` completa e
      `grep -n 'P_FOR_' pcodes.h pcodes.c` mostra le otto righe.
- [x] 1.2 In `interpreter.cpp` aggiungi a `contextObj` il metodo `isLocal(int intern)` che cerca
      solo in `objs` del livello corrente (design D3). Verifica: `make` senza errori né warning
      nuovi.
- [x] 1.3 In `interpreter.cpp` aggiungi le classi `pcodeForTo`/`pcodeForDownto`
      (`interningpcode`, possono condividere un helper con la direzione), `pcodeForTest`
      (`ipcode`) e `pcodeForNext` (`interningpcode`) accanto a `pcodeCase`, più i quattro `case`
      in `makePCode()`; non aggiungerle a `pcodejump[]`. `exec()` secondo design D1/D2/D4:
      init = controllo locale + `int` + segno del passo, `store` dell'inizio, stack −1;
      test = toglie il valore, salta se oltre la fine nel verso del passo; next = `plus` del passo
      e `store`, stack invariato. Errori con messaggio su stdout e `throw domain_error` come
      `checkTipo`. Verifica: `make` senza errori né warning nuovi.
- [x] 1.4 Prova i p-code isolati scrivendo a mano, nello scratchpad, listati `asm` (con
      `VAR`/`INT_TYPE` per dichiarare `i`, poi la sequenza del design D1 con `PRINT`) convertiti in
      binario con `./asm`: `1 to 3`, `3 downto 1`, `1 to 0` (zero giri), passo 0, passo di segno
      sbagliato, fine `NIL`. Verifica: le sequenze stampate sono quelle attese, i casi d'errore
      stampano il messaggio e terminano con codice ≠ 0, i casi buoni terminano con 0 senza violare
      `assert(sp==-1)`.

## 2. Grammatica del for e test completo

- [x] 2.1 In `parser.c` aggiungi la struct `forloop` accanto a `ifelseif` (design D5). In
      `cpl2.gram` sostituisci le quattro regole `for_stm` della sezione `--- for statement ---` con
      `for_stm`/`for_head`/`for_init` del design D5, togliendo il `COMMENT "da fare for ..."`
      (il `for each` resta com'è). Rigenera con `lemon cpl2.gram`. Verifica: `lemon` esce 0,
      `cpl2.out` non riporta conflitti, `make` ricompila `compiler`.
- [x] 2.2 Compila `for i:=10 downto 1 step -2` (con `var int i` prima) e confronta con `./asm`.
      Verifica: il listato, tolte le righe `LINE`, coincide con l'esempio del design D1 e
      l'esecuzione stampa `10 8 6 4 2`.
- [x] 2.3 Crea `test/for.cpl` + `test/for.out` che coprano gli scenari *non di errore* di
      `specs/for-statement/spec.md`: le quattro forme, inizio = fine, passo che raggiunge
      esattamente la fine, zero iterazioni con `to` e `downto`, fine modificata nel corpo, fine con
      chiamata a funzione che stampa una volta, assegnazione a `i` nel corpo, valore di `i` dopo
      il ciclo completo e dopo quello vuoto, `for` annidati (tabellina), `for` con variabile
      locale dentro una procedura chiamata due volte, `for` dentro `if`/`switch` e viceversa.
      Verifica: `./runtest` riporta `for` fra i test superati.
- [x] 2.4 Crea `test/for_err.cpl` senza `.out` (test di sola compilazione, come
      `test/type_check_err.cpl`) con i casi d'errore della spec commentati con l'esito atteso:
      variabile non dichiarata, variabile visibile solo dal modulo dentro una procedura, `step 0`,
      `to ... step -1`, `downto ... step 2`, fine stringa. Verifica: `./runtest` lo conta come
      superato; a mano, nello scratchpad, un sorgente per ciascun caso compilato ed eseguito
      stampa il messaggio (che nomina la variabile nei due casi di contesto) e termina con
      codice ≠ 0 senza eseguire il corpo.
- [x] 2.5 Esegui gli esempi di `q.cpl` (non tracciato, già pronto con i quattro `for`).
      Verifica: `./compiler q.cpl && ./interpreter q` esce 0 e stampa dopo `statement FOR` le
      sequenze `1..10`, `1 3 5 7 9`, `10..1`, `10 8 6 4 2`.

## 3. Documentazione e non-regressione

- [x] 3.1 Aggiorna `AGENTS.md`: conteggio dei p-code definiti/implementati in "Don't assume a
      construct runs" (ricontali da `pcodes.h` e `makePCode()`), togli `for` dai costrutti non
      eseguibili se citato, aggiungi un paragrafo sul `for` come quello dello `switch` (forma del
      p-code in breve, variabile che deve essere locale, fine/passo sullo stack fino ai due `POP`
      finali — un futuro `break`/`return` dovrà toglierli). Verifica: i numeri in `AGENTS.md`
      coincidono con il conteggio reale.
- [x] 3.2 Non-regressione: `make` completo e `./runtest` con codice di uscita 0; `cpl2.c` e
      `cpl2.h` rigenerati inclusi così come escono da `lemon`, nessun `.pcd` aggiunto a git.
      Verifica: `git status` mostra modificati solo i sorgenti attesi (`pcodes.h`, `pcodes.c`,
      `parser.c`, `cpl2.gram`, `cpl2.c`, `cpl2.h`, `interpreter.cpp`, `AGENTS.md`) e i nuovi
      `test/for.cpl`, `test/for.out`, `test/for_err.cpl`.

## Workflow follow-up

- Dopo la revisione: `openspec archive implement-for-statement` per fondere la delta in
  `openspec/specs/interpreter-pcodes/` e creare `openspec/specs/for-statement/`.
