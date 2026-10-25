# unary-plus Specification

## Purpose

Definisce la semantica del `+` unario nelle espressioni di cpl2: una convenzione della grammatica
che lascia invariato l'operando e non produce alcun p-code.

## Requirements

### Requirement: Il più unario è l'identità

Un'espressione `+ <fattore>` SHALL avere lo stesso valore di `<fattore>`. Il `+` unario SHALL avere
la stessa precedenza del `-` unario e SHALL potersi ripetere e combinare con esso.

#### Scenario: Costante

- **WHEN** viene eseguito il programma `? +5`
- **THEN** l'output è `5`

#### Scenario: Più unario ripetuto

- **WHEN** viene eseguito il programma `? + +7`
- **THEN** l'output è `7`

#### Scenario: Combinazione con il meno unario

- **WHEN** vengono eseguiti `? -+3` e `? +-3`
- **THEN** l'output è `-3` per entrambi

#### Scenario: Dentro un'espressione binaria

- **WHEN** viene eseguito il programma `? 3 + +2`
- **THEN** l'output è `5`

#### Scenario: Espressione tra parentesi

- **WHEN** viene eseguito il programma `? +(2+3)*2`
- **THEN** l'output è `10`

#### Scenario: Variabile non modificata

- **WHEN** viene eseguito il programma `var int i:=4` seguito da `? +i` e poi da `? i`
- **THEN** l'output è `4` seguito da `4`

### Requirement: Il più unario non produce p-code

La compilazione di `+ <fattore>` SHALL produrre esattamente lo stesso p-code della compilazione di
`<fattore>`. Non SHALL esistere un p-code dedicato al `+` unario.

#### Scenario: Stesso p-code con e senza più unario

- **WHEN** si compilano separatamente `? +5` e `? 5`
- **THEN** i due `.pcd` prodotti sono identici byte per byte

#### Scenario: Il listato non contiene un'istruzione per il più unario

- **WHEN** si compila `? +i` e se ne guarda il listato con `./asm`
- **THEN** il listato contiene `LOAD i` seguito direttamente da `PRINT 1`

### Requirement: Il più unario non controlla il tipo

Poiché non esegue alcuna istruzione, il `+` unario SHALL accettare un operando di qualunque tipo
senza errori né a compile time né a runtime, e il valore SHALL restare quello dell'operando.

#### Scenario: Più unario su una stringa

- **WHEN** viene eseguito il programma `? +'a'`
- **THEN** l'output è `a` e l'interprete esce con codice 0

#### Scenario: Più unario su un booleano

- **WHEN** viene eseguito il programma `? +true`
- **THEN** l'output è `true` e l'interprete esce con codice 0
