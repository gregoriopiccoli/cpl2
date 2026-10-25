# Design

## Context

`eseguiTest()` in `runtest.c` costruisce la riga `./interpreter '<appoggio>/<nome>'` e la esegue
con `eseguiCatturando()` (via shell, stdout catturato su file). Il commento D4 dice "modalita'
normale e senza nessuna opzione di misura". L'interprete accende il check di tipo con
`--typecheck`/`-c` (`tipocheckOpzione`, riga di `tabOpzioni[]` in `interpreter.cpp`); senza
l'opzione `checkTipo()` ritorna subito. Un'assegnazione rifiutata stampa il messaggio su stdout e
lancia `domain_error`, che fa uscire l'interprete con codice diverso da 0.

Verifica fatta prima di scrivere il change: compilati ed eseguiti con `-c` tutti i 16 test
completi di `test/` (con `q1` compilato accanto per `q`/`q2`), l'output e' identico al `.out` in
tutti i casi e il codice d'uscita e' 0.

## Goals / Non-Goals

**Goals:**
- Il check di tipo sempre acceso in ogni esecuzione fatta da `runtest`.

**Non-Goals:**
- Un'opzione di `runtest` per spegnere il check o per passare opzioni arbitrarie all'interprete.
- Trasformare `type_check_err.cpl` in un test completo: i rifiuti interrompono il programma con
  codice != 0, che `runtest` conta comunque come fallimento; resta di sola compilazione.
- Correggere il bug noto del codice d'uscita di `benchPcd()`: `runtest` non usa `-b`.

## Decisions

- **Opzione lunga `--typecheck`, non `-c`.** La riga di comando diventa
  `./interpreter --typecheck '<appoggio>/<nome>'`. La forma lunga si legge da sola nel sorgente e
  nei messaggi; l'interprete riconosce le opzioni prima o dopo il percorso, quindi la posizione e'
  indifferente — la si mette prima del percorso, dove non puo' essere scambiata per parte di esso.
- **Sempre acceso, senza interruttore.** La richiesta e' che la suite giri con il check; un
  interruttore aggiungerebbe una seconda modalita' da tenere allineata ai `.out` senza un caso
  d'uso. Chi vuole vedere un test senza check usa `./interpreter` direttamente.
  Alternativa scartata: opzione `--no-typecheck` in `runtest` — rinviabile a quando servira'.
- **Nessun cambio al confronto.** Il messaggio di un rifiuto finisce su stdout e quindi
  nell'output catturato, ma il test fallisce gia' per il codice d'uscita != 0 prima del confronto:
  il motivo riportato resta "esecuzione fallita (codice N)".

## Risks / Trade-offs

- [Un fixture futuro scritto contando sul check spento fallirebbe] → e' proprio il comportamento
  voluto; il rapporto nomina il test e il codice d'uscita.
- [Il check rallenta l'esecuzione] → trascurabile sui fixture di `test/`, che sono programmi brevi.
- [`./runtest` lanciato con un `interpreter` vecchio, senza `--typecheck`] → l'interprete
  segnalerebbe l'opzione sconosciuta e ogni test completo fallirebbe in modo visibile; `make`
  ricostruisce l'interprete comunque.
