# Spec Delta

## ADDED Requirements

### Requirement: Esecuzione del p-code IS

L'interprete SHALL eseguire il p-code `IS`, p-code senza operando. Lo stack di esecuzione contiene
in cima un oggetto tipo e sotto di esso un valore; il p-code li consuma entrambi e lascia al loro
posto un unico booleano, `true` se il valore appartiene al tipo secondo le regole della capability
`is-operator`, `false` altrimenti.

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `IS` con un valore e un tipo in cima allo stack di esecuzione
- **THEN** la profondità dello stack diminuisce di 1 e in cima c'è un booleano

#### Scenario: Valore conforme

- **WHEN** viene eseguita la sequenza `INT_CONST 1`, `INT_TYPE`, `IS`
- **THEN** in cima allo stack c'è `true`

#### Scenario: Operando di tipo non riconosciuto

- **WHEN** viene eseguita la sequenza `INT_CONST 1`, `INT_CONST 3`, `IS`
- **THEN** l'esecuzione termina con un errore a runtime

### Requirement: IS non è più un p-code non implementato

L'interprete SHALL riconoscere `IS` nella fabbrica dei p-code: l'esecuzione di un `.pcd` che
contiene `IS` non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma con is

- **WHEN** viene eseguito il `.pcd` compilato da `? (1 is int)`
- **THEN** l'output è `true` e non compare `not impl!`

### Requirement: Esecuzione del p-code FLOAT_TYPE

L'interprete SHALL eseguire il p-code `FLOAT_TYPE`, p-code senza operando, spingendo in cima allo
stack di esecuzione l'oggetto tipo che rappresenta il tipo float, la cui rappresentazione testuale
è `float`.

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `FLOAT_TYPE` con lo stack di esecuzione a una certa profondità
- **THEN** la profondità dello stack aumenta di 1 e il tipo float risiede in cima

#### Scenario: Dichiarazione di una variabile float

- **WHEN** viene eseguito `var float x` seguito da `? (x is float)`
- **THEN** la dichiarazione non termina con `not impl!` e l'output è `false` (`x` vale `nil`)

### Requirement: FLOAT_TYPE non è più un p-code non implementato

L'interprete SHALL riconoscere `FLOAT_TYPE` nella fabbrica dei p-code: l'esecuzione di un `.pcd` che
contiene `FLOAT_TYPE` non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma con il tipo float

- **WHEN** viene eseguito il `.pcd` compilato da `? (1 is float)`
- **THEN** l'output è `false` e non compare `not impl!`
