# Tasks

## 1. Fallback .pcd in asm.c

- [x] 1.1 In `main()` di `asm.c`, dove si apre il percorso indicato, aggiungi la seconda
      prova `<percorso>.pcd` quando la prima `fopen` fallisce con ENOENT e il nome di
      base (dopo l'ultimo `/`) non contiene punti, con un commento che richiama la regola
      D2 dell'interprete. Verifica: `make asm && cd /tmp && cp
      /home/greg/Progetti/cpl2/test/*.pcd /tmp/mio.pcd && ./asm mio` dal repo produce la
      stessa listing di `./asm mio.pcd`.
- [x] 1.2 Verifica i casi di non-fallback: `./asm /tmp/mancante` (né `mancante` né
      `mancante.pcd` esistono) stampa su stderr il nome digitato senza suffisso ed esce
      con 1; `./asm /tmp/mio` con `/tmp/mio` esistente traduce `mio` e non prova
      `mio.pcd`; un nome base con un punto (`./asm /tmp/mio.bak` inesistente) esce con 1
      senza provare `mio.bak.pcd`; `dir.d/mio` inesistente con `dir.d/mio.pcd` esistente
      viene tradotto.
- [x] 1.3 Verifica che la modalità stdin e le altre uscite siano invariate: `./asm <
      test/simple_class.pcd` e `./asm - < test/simple_class.pcd` producono la listing su
      stdout, `./asm -b test/simple_class.pcd` segnala `-b` come opzione sconosciuta ed
      esce con 1, `./asm a.pcd b.pcd` esce con 1, e nessun file viene creato.

## 2. Documentazione

- [x] 2.1 Aggiorna la sezione "`asm` is a checker, not a pipeline stage" di `AGENTS.md`:
      il nome senza estensione prova prima il file com'è scritto e poi `<nome>.pcd`,
      il punto si cerca solo nel nome di base, e l'errore nomina il percorso digitato.
      Verifica: la descrizione nei due riferimenti (riga dell'uso e paragrafo) è
      coerente col comportamento verificato al task 1.2.
- [x] 2.2 Compila anche il resto e fai la prova di non-regressione: `make` completa senza
      errori e `./runtest` termina con codice 0 (tutti i test di `test/` passano).

## Workflow follow-up

- Dopo la revisione: `openspec archive add-asm-pcd-fallback` per fonderlo nelle spec
  principali, poi verificare con `openspec list --specs` che `asm-cli` riporti i
  requisiti aggiornati.
