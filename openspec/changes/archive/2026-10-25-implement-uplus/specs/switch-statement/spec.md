# Spec Delta

## ADDED Requirements

### Requirement: Costanti intere con più unario

Una costante di un `case` scritta `+<intero>` SHALL essere equivalente a `<intero>`: stesso valore
confrontato con il selettore e stesso p-code emesso. Il `+` SHALL essere accettato solo davanti a
una costante intera, come il `-` dei `case` negativi.

#### Scenario: Corrispondenza con case +1

- **WHEN** viene eseguito `switch i` con `i` uguale a 1 e rami `case 0` e `case +1`, ognuno dei
  quali stampa la propria costante
- **THEN** viene stampato solo `1`

#### Scenario: Più unario in un case con più costanti

- **WHEN** viene eseguito `switch n` con `n` uguale a 2 e un ramo `case 0,+2` che stampa
  `zero o due`
- **THEN** viene stampato `zero o due`

#### Scenario: Nessun p-code per il più unario nel case

- **WHEN** si compila uno `switch` con `case +1` e se ne guarda il listato con `./asm`
- **THEN** la costante compare come `INT_CONST 1` seguita direttamente da `CASE`, come per `case 1`

#### Scenario: Più unario davanti a una costante non intera

- **WHEN** si compila uno `switch` con `case +'a'`
- **THEN** il compilatore segnala `syntax error` ed esce con codice diverso da 0
