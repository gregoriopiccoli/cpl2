# Spec Delta

## ADDED Requirements

### Requirement: Esecuzione del p-code OR_TYPE

L'interprete SHALL eseguire il p-code `OR_TYPE`, il cui operando intero N (sempre almeno 2, come garantito dalla grammatica) è il numero di alternative del tipo, consumando i N tipi in cima allo stack di esecuzione e lasciando al loro posto un unico oggetto tipo che rappresenta il tipo or con quelle N alternative, nell'ordine in cui erano sullo stack.

#### Scenario: Tipo or con due alternative

- **WHEN** viene eseguito `OR_TYPE` con operando 2 quando lo stack di esecuzione contiene i tipi `int` e `str`
- **THEN** lo stack di esecuzione contiene un solo oggetto tipo or con alternative `int` e `str`

#### Scenario: Tipo or con tre alternative

- **WHEN** viene eseguito `OR_TYPE` con operando 3 quando lo stack di esecuzione contiene i tipi `int`, `str` e `bool`
- **THEN** lo stack di esecuzione contiene un solo oggetto tipo or con alternative `int`, `str` e `bool`

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `OR_TYPE` con operando N su uno stack di esecuzione con N tipi in cima
- **THEN** la profondità dello stack diminuisce di N-1 e il tipo or risiede in cima

### Requirement: Rappresentazione stampabile del tipo or

L'oggetto tipo prodotto da `OR_TYPE` SHALL avere una rappresentazione testuale che elenca le alternative separate dal carattere `|`, nel formato `t1|t2|...`.

#### Scenario: Stampa di un tipo or

- **WHEN** viene stampato un tipo or con alternative `int` e `str`
- **THEN** la rappresentazione testuale è `int|str`

### Requirement: Sicurezza GC del tipo or

L'oggetto tipo prodotto da `OR_TYPE` SHALL rendere raggiungibili per il garbage collector tutti i tipi delle alternative che lo compongono, sia durante la marcatura sia durante l'espansione delle generazioni.

#### Scenario: Marcatura delle alternative

- **WHEN** il garbage collector marca un tipo or marcato come raggiungibile
- **THEN** anche tutti i tipi delle sue alternative risultano marcati o già raggiungibili

### Requirement: OR_TYPE non è più un p-code non implementato

L'interprete SHALL riconoscere `OR_TYPE` nella fabbrica dei p-code: l'esecuzione di un `.pcd` che contiene `OR_TYPE` non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma cpl2 che dichiara un tipo or

- **WHEN** viene eseguito un `.pcd` compilato da un sorgente che dichiara una variabile di tipo alternativo (per esempio `var int|str o` in `q.cpl`)
- **THEN** il programma si esegue senza l'errore `pcode not implemented` e arriva alla fine con exit 0
