## ADDED Requirements

### Requirement: Aritmetica fra float

L'interprete SHALL eseguire `PLUS`, `MINUS`, `MULT` e `DIV` quando entrambi gli operandi sono
float, sostituendo i due operandi in cima allo stack con un nuovo float che vale la somma, la
differenza, il prodotto o il quoziente in precisione doppia. La profondità dello stack SHALL
diminuire di 1 e gli operandi SHALL restare invariati.

#### Scenario: Somma di due float

- **WHEN** viene eseguito il programma `? 1.5 + 1.5`
- **THEN** l'output è `3.0`

#### Scenario: Differenza, prodotto e quoziente

- **WHEN** viene eseguito il programma `? 2.5 - 0.5` seguito da `? 1.5 * 2.0` e da `? 1.0 / 4.0`
- **THEN** l'output è `2.0` seguito da `3.0` e da `0.25`

#### Scenario: Risultato in precisione doppia

- **WHEN** viene eseguito il programma `? 0.1 + 0.2`
- **THEN** l'output è `0.30000000000000004`

#### Scenario: Il risultato è un float

- **WHEN** viene eseguito il programma `? (1.5 + 1.5 is float)`
- **THEN** l'output è `true`

#### Scenario: Gli operandi non cambiano

- **WHEN** viene eseguito il programma `var float a:=1.5` seguito da `? a + a`, `? a * a` e `? a`
- **THEN** l'output è `3.0` seguito da `2.25` e da `1.5`

#### Scenario: La costante non cambia fra un giro e l'altro

- **WHEN** un ciclo esegue tre volte `? 0.5 + 0.25`
- **THEN** l'output è `0.75` per tutte e tre le iterazioni

### Requirement: Divisione float per zero

La divisione con almeno un operando float e divisore zero SHALL seguire IEEE 754 senza errore di
runtime: un dividendo non nullo dà un infinito col segno del risultato (stampato `inf` o `-inf`),
zero diviso zero dà un NaN (stampato `nan` o `-nan`).

#### Scenario: Divisione per zero positiva e negativa

- **WHEN** viene eseguito il programma `? 1.0 / 0.0` seguito da `? -1.0 / 0.0`
- **THEN** l'output è `inf` seguito da `-inf` e il programma termina normalmente

#### Scenario: Zero diviso zero

- **WHEN** viene eseguito il programma `var float z := 0.0 / 0.0` seguito da `? (z = z)` e da
  `? (z <> z)`
- **THEN** l'output è `false` seguito da `true`

### Requirement: Confronti fra float

L'interprete SHALL eseguire `EQ`, `LT`, `LE`, `GE`, `GT` e `NE` quando entrambi gli operandi sono
float, sostituendo i due operandi in cima allo stack con `true` o `false` secondo il confronto
numerico in precisione doppia. La profondità dello stack SHALL diminuire di 1.

#### Scenario: Minore e maggiore

- **WHEN** viene eseguito il programma `? 1.5 < 2.5` seguito da `? 1.5 > 2.5`
- **THEN** l'output è `true` seguito da `false`

#### Scenario: Confronti con valori uguali

- **WHEN** viene eseguito il programma `? 2.5 <= 2.5`, `? 2.5 >= 2.5`, `? 2.5 = 2.5` e
  `? 2.5 <> 2.5`
- **THEN** l'output è `true`, `true`, `true` e `false`

#### Scenario: Confronto in un ciclo

- **WHEN** viene eseguito il programma `var float x := 0.0` seguito da `while x < 1.0` con corpo
  `x := x + 0.25` e poi `? x`
- **THEN** l'output è `1.0`

### Requirement: Promozione a float negli operandi misti

Quando uno degli operandi di `PLUS`, `MINUS`, `MULT`, `DIV`, `EQ`, `LT`, `LE`, `GE`, `GT` o `NE`
è un float e l'altro è un intero, in qualunque ordine, l'intero SHALL essere convertito in float
e l'operazione SHALL essere eseguita come fra due float: l'aritmetica produce un float, i
confronti `true`/`false`.

#### Scenario: Intero a sinistra e a destra

- **WHEN** viene eseguito il programma `? 1 + 0.5` seguito da `? 0.5 + 1`
- **THEN** l'output è `1.5` seguito da `1.5`

#### Scenario: Il risultato misto è un float anche quando è intero

- **WHEN** viene eseguito il programma `? 2 * 1.5` seguito da `? (2 * 1.5 is float)`
- **THEN** l'output è `3.0` seguito da `true`

#### Scenario: Divisione mista

- **WHEN** viene eseguito il programma `? 1 / 2.0` seguito da `? 3.0 / 2`
- **THEN** l'output è `0.5` seguito da `1.5`

#### Scenario: Confronti misti

- **WHEN** viene eseguito il programma `? 1 < 1.5`, `? 2.5 > 3`, `? 1 = 1.0` e `? 1.0 <> 1`
- **THEN** l'output è `true`, `false`, `true` e `false`

### Requirement: Le operazioni fra interi non cambiano

Quando entrambi gli operandi sono interi, `PLUS`, `MINUS`, `MULT`, `DIV`, `MOD`, `IDIV` e i
confronti SHALL comportarsi come prima di questo change: l'aritmetica produce un intero e `DIV`
fra interi resta la divisione intera.

#### Scenario: Divisione fra interi

- **WHEN** viene eseguito il programma `? 7 / 2` seguito da `? (7 / 2 is int)`
- **THEN** l'output è `3` seguito da `true`

#### Scenario: Somma fra interi

- **WHEN** viene eseguito il programma `? 1 + 2` seguito da `? (1 + 2 is float)`
- **THEN** l'output è `3` seguito da `false`

### Requirement: Float con un operando non numerico

L'esecuzione di `PLUS`, `MINUS`, `MULT`, `DIV`, `EQ`, `LT`, `LE`, `GE`, `GT` o `NE` con un
operando float e l'altro che non è né un intero né un float (stringa, bool, nil, array, dict,
tuple, tipo, procedura, modulo) SHALL interrompere il programma con un errore di runtime, senza
produrre un valore.

#### Scenario: Float più stringa

- **WHEN** viene eseguito il programma `? 1.5 + 'a'`
- **THEN** l'esecuzione termina con un errore di runtime e non stampa alcun valore per quella
  riga

#### Scenario: Stringa confrontata con un float

- **WHEN** viene eseguito il programma `? 'a' < 1.5`
- **THEN** l'esecuzione termina con un errore di runtime e non stampa alcun valore per quella
  riga

#### Scenario: Float uguale a nil

- **WHEN** viene eseguito il programma `? 1.5 = nil`
- **THEN** l'esecuzione termina con un errore di runtime e non stampa alcun valore per quella
  riga

### Requirement: MOD e IDIV restano operazioni intere

L'esecuzione di `MOD` o `IDIV` con almeno un operando float SHALL interrompere il programma con
un errore di runtime, senza produrre un valore.

#### Scenario: Modulo con un float

- **WHEN** viene eseguito il programma `? 5.5 mod 2`
- **THEN** l'esecuzione termina con un errore di runtime e non stampa alcun valore per quella
  riga

#### Scenario: Divisione intera con un float a destra

- **WHEN** viene eseguito il programma `? 5 div 2.0`
- **THEN** l'esecuzione termina con un errore di runtime e non stampa alcun valore per quella
  riga
