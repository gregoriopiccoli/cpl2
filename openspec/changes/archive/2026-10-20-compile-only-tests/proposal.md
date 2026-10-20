# Proposal

## Why

Oggi nella directory dei test un `.cpl` senza `.out` accanto non viene toccato da `runtest`: e'
uno scratchpad invisibile alla suite. Questo rende impossibile coprire con un test quei
sorgenti che devono solo compilare — per esempio `simple_class.cpl` o `simple_import.cpl`, le cui
istruzioni p-code corrispondenti (`class`, `import`) non sono ancora implementate
nell'interprete: compilarli senza eseguirli e' l'unico modo per verificare che il compilatore
non si rompa su di essi.

## What Changes

- Un file `.cpl` senza `.out` accanto diventa un **test di sola compilazione**: `runtest` lo
  compila con `./compiler` e deve ottenere codice di uscita 0; non viene mai eseguito con
  `./interpreter`.
- Un test di sola compilazione che compila entra nei **superati**; che non compila entra nei
  **falliti**, nominato nel rapporto come ogni altro test fallito, e il giro continua.
- Restano invariati: la coppia `.cpl` + `.out` come test completo (compila + esegue + confronta
  byte per byte), l' `.out` orfano come fallimento, l'assenza di ricorsione nelle sottodirectory,
  il rapporto su stderr, la directory di appoggio in `TMPDIR` e il codice di uscita.
- **BREAKING** (solo per la convenzione documentata): un `.cpl` senza `.out` non e' piu'
  "ignorato"; i tre scratchpad attuali (`test_expr1.cpl`, `test_stm1.cpl`, `test_var1.cpl`) e i
  tre fixture senza atteso (`simple_func.cpl`, `simple_import.cpl`, `simple_class.cpl`)
  entrano nel rapporto. Verificato in anticipo: tutti e sei compilano senza errori oggi, quindi
  il giro resta verde.
- Aggiornamento della documentazione (`AGENTS.md`) che oggi definisce il `.cpl` senza `.out` uno
  scratchpad non contato.

## Capabilities

### New Capabilities

<!-- nessuna: il comportamento nuovo vive interamente nel sistema di test gia' coperto da test-suite -->

### Modified Capabilities

- `test-suite`: la requirements "Selezione dei test in una directory" cambia — un `.cpl` senza
  `.out` non e' piu' ignorato ma diventa un test di sola compilazione; nuova requirement dedicata
  al comportamento e all'esito di un test di sola compilazione.

## Impact

- `runtest.c` — `raccogli()` classifica i `.cpl` senza `.out` come terza categoria; la fase di
  esecuzione compila questi senza chiamare `./interpreter`; il rapporto li conta.
- `AGENTS.md` — sezione "Tests": la regola "un `.cpl` senza `.out` non e' un test" va riscritta.
- Nessuna modifica a `compiler`, `interpreter`, `cli.c`, al grammar o ai fixture di `test/`
  (verificato: tutti i `.cpl` attuali senza `.out` compilano).
