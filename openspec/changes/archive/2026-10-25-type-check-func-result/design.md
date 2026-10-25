# Design

## Context

Vedi proposal.md - Why. Stato attuale nel codice:

- La grammatica emette `P_STORE_RESULT` per `result := expr` (`cpl2.gram:323`); il p-code
  lascia il valore in cima allo stack.
- I due percorsi di dispatch fanno la stessa cosa: `pcodeStoreResult::exec` e la label
  `p_store_result` del computed goto (`RUN_WITH_SWITCH`) chiamano entrambi
  `context->store_result(stack[sp])`. Il "RESULT_ASSIGN" della richiesta e' questo p-code.
- `store_result()` e' virtuale su `obj` (lancia "store result not implemented"); l'unica
  ridefinizione e' `funcVars::store_result`, che fa solo `result_value=value;`.
- `funcVars` ha gia' il tipo di ritorno in `result_type`, passato da `funcObj::mkVars()` (il
  tipo e' quello dichiarato in `funcname ::= FUNC generic type ID`, quindi puo' essere un tipo
  base, `?` (theNil), un `orType`, un `notType`, un tipo composto o una firma).
- `funcVars` deriva da `procVars` -> `contextObj`, che ha gia' `checkTipo(intern,tipo,valore)`:
  rispetta `tipocheckOpzione`, usa `tipoCompatibile()`, stampa il messaggio su stdout e lancia
  `domain_error`.

## Goals / Non-Goals

**Goals:**
- Un solo punto di controllo che copra entrambi i percorsi di dispatch.
- Messaggio e comportamento identici agli altri rifiuti di tipo (stesso formato, stesso
  interruttore `--typecheck`/`-c`).

**Non-Goals:**
- Controllare il risultato di una func che **non** assegna mai `result`: resta `nil` come oggi,
  anche con tipo di ritorno `not`. E' coerente con `var int not ni` dichiarata senza
  inizializzazione, che parte nil e viene controllata solo alla prima assegnazione.
- Cambiare il comportamento di `result` dentro una `proc` (oggi `procVars` non ridefinisce
  `store_result` e il runtime lancia "store result not implemented"): invariato.
- Catturare il `domain_error` in `main()`: il rifiuto interrompe l'interprete come gli altri.
- Il check degli elementi in `P_STORESLICE`: resta fuori, come nel change `type-checking`.

## Decisions

### D1: Il check va in `funcVars::store_result`, non nel p-code
`funcVars::store_result(value)` chiama `checkTipo(<intern di "result">, result_type, value)`
prima di `result_value=value`.

- Copre in un colpo sia `pcodeStoreResult::exec` sia `p_store_result` del computed goto,
  che passano entrambi da `store_result()`: niente codice duplicato nella dispatch che
  AGENTS.md chiede di non toccare.
- Solo `funcVars` conosce `result_type`; metterlo nel p-code richiederebbe un `dynamic_cast`
  del contesto a ogni assegnazione di `result`, e di nuovo due volte (una per percorso).
- Alternativa scartata: controllare il valore al ritorno da `procObj::call()`, quando lo
  stack riceve `vars->getResult()`. Segnalerebbe l'errore lontano dall'assegnazione
  sbagliata, e intercetterebbe anche il nil di default che i Non-Goals lasciano passare.

### D2: Il nome nel messaggio e' `result`, ottenuto come intern
`checkTipo` vuole un intern per comporre `<nome>: il valore ... non e' compatibile con il tipo
...`. Si usa `theStringIntern.add("result")`, calcolato una volta (statico), cosi' il messaggio
e' `result: il valore pippo non e' compatibile con il tipo int` senza toccare la firma di
`checkTipo`.

- Alternativa scartata: un overload di `checkTipo` con `const string&` — una seconda strada
  per lo stesso messaggio, solo per evitare un intern.

### D3: Interruttore `--typecheck`/`-c` invariato
Il check e' sotto lo stesso `tipocheckOpzione` di `checkTipo`, per costruzione: senza
l'opzione `result` si assegna come oggi, e il costo e' un test su un `int` statico.

## Risks / Trade-offs

- [Programmi che oggi restituiscono un tipo diverso da quello dichiarato smettono di girare
  con `-c`] → e' esattamente il comportamento voluto; senza `-c` nulla cambia. La verifica
  `./runtest` tutto SUPERATO lo conferma sui test esistenti.
- [`runtest` lancia l'interprete senza `-c`, quindi `test/type_check.cpl` non esercita davvero
  il controllo] → limite gia' presente nel change `type-checking`; i casi validi si verificano
  anche a mano con `./interpreter -c`, i rifiuti restano casi manuali documentati in
  `test/type_check_err.cpl`.
- [`theStringIntern.add("result")` statico calcolato prima che l'intern sia pronto] → si
  calcola alla prima chiamata (statico locale nella funzione), non a livello di file.
