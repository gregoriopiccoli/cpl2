# Spec Delta

## ADDED Requirements

### Requirement: Esecuzione del p-code UMINUS sugli interi

L'interprete SHALL eseguire il p-code `UMINUS`, che non ha operando, sostituendo il valore intero in
cima allo stack di esecuzione con un intero di valore opposto. La profondità dello stack SHALL
restare invariata.

#### Scenario: Negazione di una costante positiva

- **WHEN** viene eseguito il programma `? -5`
- **THEN** l'output è `-5`

#### Scenario: Negazione di un valore negativo

- **WHEN** viene eseguito il programma `? - -7`
- **THEN** l'output è `7`

#### Scenario: Negazione dello zero

- **WHEN** viene eseguito il programma `? -0`
- **THEN** l'output è `0`

#### Scenario: Negazione di un'espressione tra parentesi

- **WHEN** viene eseguito il programma `? -(2+3)`
- **THEN** l'output è `-5`

#### Scenario: Meno unario come operando di un'operazione binaria

- **WHEN** viene eseguito il programma `? 3 - -2`
- **THEN** l'output è `5`

#### Scenario: Precedenza rispetto alla moltiplicazione

- **WHEN** viene eseguito il programma `? -2*3`
- **THEN** l'output è `-6`

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `UMINUS` con un intero in cima allo stack
- **THEN** la profondità dello stack non cambia e in cima c'è l'intero negato

### Requirement: UMINUS non modifica l'operando

Il p-code `UMINUS` SHALL produrre un valore nuovo e SHALL lasciare invariato l'intero da cui parte,
sia quando proviene da una variabile sia quando proviene da una costante del programma eseguita
più volte.

#### Scenario: Variabile non modificata

- **WHEN** viene eseguito il programma `var int i:=4` seguito da `? -i` e poi da `? i`
- **THEN** l'output è `-4` seguito da `4`

#### Scenario: Costante rieseguita in un ciclo

- **WHEN** un ciclo esegue tre volte `? -5`
- **THEN** l'output è `-5` per tutte e tre le iterazioni

### Requirement: UMINUS su valori non interi

L'esecuzione di `UMINUS` su un valore che non è un intero (stringa, bool, nil, array, dict, tuple,
tipo, procedura, modulo) SHALL interrompere il programma con un errore di runtime, senza produrre
un valore.

#### Scenario: Meno unario su una stringa

- **WHEN** viene eseguito il programma `? -'a'`
- **THEN** l'esecuzione termina con l'errore `uminus not implemented` e non stampa alcun valore
  per quella riga
