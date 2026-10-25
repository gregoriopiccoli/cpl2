# Proposal

## Why

`runtest` esegue oggi ogni test completo con `./interpreter` senza alcuna opzione, quindi il check
di tipo nelle assegnazioni (`--typecheck`/`-c`) resta spento durante l'intero giro: un test come
`type_check.cpl` passa anche se il check fosse rotto, e una regressione che rendesse il check troppo
severo (assegnazioni compatibili rifiutate) non verrebbe mai vista dalla suite. I test devono girare
nella modalita' piu' rigorosa che l'interprete offre.

## What Changes

- `runtest` esegue il p-code di ogni test completo con il check di tipo acceso, passando
  `--typecheck` a `./interpreter`, sempre e senza bisogno di opzioni da parte dell'utente.
- Nessun cambiamento per i test di sola compilazione: l'interprete non viene invocato.
- Nessuna opzione nuova per spegnere il check: non e' richiesta (la verifica a mano senza check
  resta possibile con `./interpreter` diretto).
- Resta invariato il divieto della modalita' di misura: `--bench` non viene mai passato.
- Verificato oggi: tutti i 16 test completi di `test/` producono lo stesso output byte per byte con
  e senza `--typecheck`, quindi nessun `.out` va rigenerato.

## Capabilities

### New Capabilities

_Nessuna._

### Modified Capabilities

- `test-suite`: il requisito "Esecuzione di un test" stabilisce che il p-code di un test completo
  viene eseguito con il check di tipo acceso; il requisito "Nessuna interazione con la modalita' di
  misura" chiarisce che l'unica opzione passata all'interprete e' quella del check di tipo.

## Impact

- `runtest.c`: la riga di comando costruita in `eseguiTest()` per `./interpreter` e i commenti
  relativi (D4).
- `AGENTS.md`: la sezione "Tests" dice che i test girano in modalita' normale; va aggiunto che il
  check di tipo e' acceso.
- Nessun impatto su `interpreter.cpp`, `parser.c`, sui fixture in `test/` o sui loro `.out`.
