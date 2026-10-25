# Tasks

## 1. Il check di tipo su `result`

- [x] 1.1 In `interpreter.cpp`, `funcVars::store_result(obj* value)`: prima di
      `result_value=value;` chiama `checkTipo(intern,result_type,value)`, dove `intern` e'
      `theStringIntern.add("result")` tenuto in uno `static int` locale alla funzione
      (calcolato alla prima chiamata, design D2). Nessuna modifica a `pcodeStoreResult::exec`
      ne' alla label `p_store_result` del computed goto: passano entrambi da
      `store_result()` (design D1). Verifica: `make` compila senza errori.
- [x] 1.2 Togli dalla lista "DA FARE" in testa a `interpreter.cpp` la riga
      `fare il check di tipo su "result"`. Verifica: `grep -n 'check di tipo su "result"'
      interpreter.cpp` non trova piu' nulla.
- [x] 1.3 Verifica a mano dei rifiuti, ciascuno come snippet isolato in `tmp/` compilato con
      `./compiler` ed eseguito con `./interpreter -c`:
      `func int f() result:="pippo" end` + `? f()` stampa
      `result: il valore pippo non e' compatibile con il tipo int` ed esce con errore;
      `func int not f() result:=nil end` + `? f()` viene rifiutato allo stesso modo col tipo
      `not int`. Gli stessi snippet **senza** `-c` girano senza errori (stampano `pippo` e
      `nil`), a prova che l'interruttore e' rispettato.
- [x] 1.4 Verifica a mano dei casi validi con `./interpreter -c`: `func int|str f(int x)`
      che assegna un `int` o una `str` a seconda di `x`, `func int f()` con `result:=nil`,
      `func ? f()` con `result:="ciao"`: nessun rifiuto, i valori restituiti sono quelli
      assegnati.

## 2. Test

- [x] 2.1 In `test/type_check.cpl` aggiungi, nella sezione proc/func, i casi validi di
      `result`: tipo di ritorno unione `int|str` usato da ambo i lati, `result:=nil` in una
      func `int` senza `not`, func `?` che restituisce una stringa. Aggiorna il commento
      iniziale con il rifiuto `func int f() / result:="pippo"`. Rigenera
      `test/type_check.out` dal run reale (`./compiler test/type_check.cpl` e
      `./interpreter test/type_check.pcd`, esito 0) e controlla a occhio che le righe nuove
      siano quelle attese; poi lo stesso `.pcd` con `./interpreter -c` deve dare la stessa
      uscita, esito 0. Rimuovi il `test/type_check.pcd` prodotto (non va committato).
- [x] 2.2 In `test/type_check_err.cpl` aggiungi i rifiuti di `result`: `func int r()`
      con `result:="pippo"` e `func int not rn()` con `result:=nil`, ciascuno chiamato e con
      il messaggio atteso nel commento a fianco. Resta un test di sola compilazione.
      Verifica: `./compiler test/type_check_err.cpl` esce con 0 (poi rimuovi il `.pcd`).
- [x] 2.3 `make && ./runtest`: tutti i test di `test/` SUPERATI ed exit 0, a prova che il
      check non rompe nessun programma esistente.

## Workflow follow-up

- Dopo la revisione: `openspec archive type-check-func-result` per fondere il requisito
  ADDED nella spec principale `type-checking`, poi verifica con
  `openspec show type-checking --type spec`.
