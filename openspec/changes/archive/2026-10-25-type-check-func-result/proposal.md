# Proposal

## Why

Con `--typecheck`/`-c` ogni assegnazione a una variabile dichiarata viene verificata
(`VAR_STORE`, `STORE`, `STOREX`, inizializzazione dei parametri), ma l'assegnazione alla
variabile implicita `result` di una `func` no: `func int f() result:="pippo" end` restituisce
una stringa da una func dichiarata `int` senza alcun errore. E' l'unico buco rimasto nel check
delle assegnazioni, ed e' annotato come "DA FARE" in testa a `interpreter.cpp` ("fare il check
di tipo su result") e come fuori perimetro nel change archiviato `type-checking`.

## What Changes

- L'assegnazione `result := expr` dentro una `func` (p-code `P_STORE_RESULT`, emesso dalla
  regola `aexpr ::= RESULT ASSIGN expr.`) controlla che il valore sia compatibile con il tipo
  di ritorno dichiarato della func, con le stesse regole di `tipoCompatibile()` usate per le
  variabili (unione, `not`, `?`, nil accettato senza `not`, tipi composti, firme proc/func).
- Un valore incompatibile produce lo stesso tipo di rifiuto delle altre assegnazioni: messaggio
  su stdout nella forma `result: il valore <v> non e' compatibile con il tipo <t>` e
  `domain_error`, che interrompe il programma.
- Il controllo e' attivo solo con `--typecheck`/`-c`, come tutti gli altri: senza l'opzione
  `result` si assegna senza controlli (comportamento attuale invariato).
- Nessun cambiamento alla grammatica, al compilatore o al formato `.pcd`: il p-code
  `STORE_RESULT` esiste gia' e porta gia' sullo stack il valore; il tipo di ritorno e' gia'
  memorizzato in `funcVars::result_type`.

## Capabilities

### New Capabilities
(nessuna)

### Modified Capabilities
- `type-checking`: si aggiunge il requisito che l'assegnazione al `result` di una func sia
  compatibile con il tipo di ritorno dichiarato.

## Impact

- `interpreter.cpp`: `funcVars::store_result()` (oggi `result_value=value;` senza controlli)
  esegue il check di tipo prima di memorizzare il valore. Copre entrambi i percorsi di dispatch
  (`pcodeStoreResult::exec` e la label `p_store_result` del computed goto), che chiamano
  entrambi `context->store_result()`.
- `interpreter.cpp`: si toglie la voce "fare il check di tipo su result" dalla lista DA FARE.
- `test/type_check.cpl`/`.out` e `test/type_check_err.cpl`: casi validi e rifiutati per
  `result`.
- Nessun impatto su `parser.c`, `cpl2.gram`, `pcodes.*`, `asm.c`.
