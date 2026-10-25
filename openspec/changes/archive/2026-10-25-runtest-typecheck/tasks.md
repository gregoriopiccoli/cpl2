# Tasks

## 1. runtest con il check di tipo acceso

- [x] 1.1 In `eseguiTest()` di `runtest.c` costruire la riga `./interpreter --typecheck '<appoggio>/<nome>'` al posto di `./interpreter '<appoggio>/<nome>'`, aggiornando in italiano il commento D4 ("modalita' normale, nessuna opzione di misura, check di tipo acceso") e il commento di testa del file se cita l'esecuzione senza opzioni; verificare con `make && ./runtest` che il rapporto dica `superati 25, falliti 0` (o il totale corrente, invariato rispetto a prima della modifica)
- [x] 1.2 Verificare che il check sia davvero acceso: in una directory di prova sotto `tmp/` mettere un `.cpl` con `var int i := 5` / `i := "pippo"` e un `.out` qualsiasi, eseguire `./runtest tmp/<dir>` e controllare che il test risulti fallito con "esecuzione fallita (codice N)" mentre con il `runtest` precedente sarebbe arrivato al confronto; rimuovere poi la directory di prova
- [x] 1.3 Aggiornare `AGENTS.md`, sezione "Tests": i test completi girano con `--typecheck` (oltre a "never with `--bench`"), e il commento in `test/type_check.cpl` che dice che i rami di rifiuto saltano i test automatici resta valido; verificare rileggendo che nessun'altra frase di `AGENTS.md` dica che l'interprete viene invocato senza opzioni (`grep -n "normal mode" AGENTS.md`)
