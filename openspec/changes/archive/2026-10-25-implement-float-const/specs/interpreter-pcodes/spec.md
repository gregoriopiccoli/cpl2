## ADDED Requirements

### Requirement: Esecuzione del p-code FLOAT_CONST

L'interprete SHALL eseguire il p-code `FLOAT_CONST`, il cui operando è il testo della costante
così come lo scrive il compilatore, spingendo in cima allo stack di esecuzione un valore float con
quel valore numerico (precisione doppia). La profondità dello stack SHALL aumentare di 1.

#### Scenario: Stampa di una costante float

- **WHEN** viene eseguito il programma `? 3.14`
- **THEN** l'output è `3.14`

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `FLOAT_CONST 2.5` con lo stack di esecuzione a una certa profondità
- **THEN** la profondità dello stack aumenta di 1 e in cima c'è il float `2.5`

#### Scenario: Costante con separatori di cifre

- **WHEN** viene eseguito il programma `? 1_000.25`
- **THEN** l'output è `1000.25`

#### Scenario: Il valore è un float

- **WHEN** viene eseguito il programma `? (3.14 is float)` seguito da `? (3.14 is int)`
- **THEN** l'output è `true` seguito da `false`

#### Scenario: Assegnazione a una variabile float con il controllo di tipo attivo

- **WHEN** viene eseguito con `--typecheck` il programma `var float x := 0.5` seguito da `? x`
- **THEN** l'assegnazione è accettata e l'output è `0.5`

### Requirement: Rappresentazione stampabile del valore float

La rappresentazione testuale di un valore float SHALL essere la forma decimale più corta che,
riletta, dà esattamente lo stesso valore, e SHALL sempre distinguersi da un intero: quando quella
forma non contiene né il punto decimale né un esponente, SHALL esserle aggiunto `.0`.

#### Scenario: Nessuno zero finale superfluo

- **WHEN** viene eseguito il programma `? 3.140`
- **THEN** l'output è `3.14`

#### Scenario: Float con valore intero

- **WHEN** viene eseguito il programma `? 2.0` seguito da `? 7.`
- **THEN** l'output è `2.0` seguito da `7.0`

#### Scenario: Valore non rappresentabile esattamente

- **WHEN** viene eseguito il programma `? 0.1`
- **THEN** l'output è `0.1`

### Requirement: FLOAT_CONST non è più un p-code non implementato

L'interprete SHALL riconoscere `FLOAT_CONST` nella fabbrica dei p-code: l'esecuzione di un `.pcd`
che contiene `FLOAT_CONST` non SHALL terminare con l'errore `not impl!`.

#### Scenario: Programma con una costante float

- **WHEN** viene eseguito il `.pcd` compilato da `? 1.5`
- **THEN** l'esecuzione termina con exit 0 e l'output è `1.5`

### Requirement: La costante float non viene modificata dall'esecuzione

Il valore prodotto da `FLOAT_CONST` SHALL restare lo stesso a ogni esecuzione dello stesso p-code,
anche quando il valore è stato usato come operando di un'operazione o assegnato a una variabile.

#### Scenario: Costante rieseguita in un ciclo

- **WHEN** un ciclo esegue tre volte `? -1.5` seguito da `? 1.5`
- **THEN** l'output è `-1.5` seguito da `1.5` per tutte e tre le iterazioni

### Requirement: Esecuzione del p-code UMINUS sui float

L'interprete SHALL eseguire `UMINUS` anche quando in cima allo stack c'è un float, sostituendolo
con un nuovo float di valore opposto e lasciando invariato il float da cui parte. La profondità
dello stack SHALL restare invariata.

#### Scenario: Negazione di una costante float

- **WHEN** viene eseguito il programma `? -2.5`
- **THEN** l'output è `-2.5`

#### Scenario: Doppia negazione

- **WHEN** viene eseguito il programma `? - -0.75`
- **THEN** l'output è `0.75`

#### Scenario: Variabile float non modificata

- **WHEN** viene eseguito il programma `var float f:=4.5` seguito da `? -f` e poi da `? f`
- **THEN** l'output è `-4.5` seguito da `4.5`

#### Scenario: Il risultato è ancora un float

- **WHEN** viene eseguito il programma `? (-2.5 is float)`
- **THEN** l'output è `true`

## MODIFIED Requirements

### Requirement: UMINUS su valori non interi

L'esecuzione di `UMINUS` su un valore che non è né un intero né un float (stringa, bool, nil,
array, dict, tuple, tipo, procedura, modulo) SHALL interrompere il programma con un errore di
runtime, senza produrre un valore.

#### Scenario: Meno unario su una stringa

- **WHEN** viene eseguito il programma `? -'a'`
- **THEN** l'esecuzione termina con l'errore `uminus not implemented` e non stampa alcun valore
  per quella riga
