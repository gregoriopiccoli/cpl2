# Spec Delta

## Purpose

Definisce la semantica dello statement `switch` di cpl2 (scelta di un ramo in base al valore di
un selettore confrontato con costanti) e il comportamento del programma compilato che lo contiene.

## ADDED Requirements

### Requirement: Esecuzione del ramo corrispondente

Uno `switch` SHALL eseguire le istruzioni del primo `case` che contiene una costante uguale al
valore del selettore, e nessun altro ramo.

#### Scenario: Corrispondenza con un case

- **WHEN** viene eseguito `switch i` con `i` uguale a 1 e rami `case 0`, `case 1`, `case 2`, ognuno
  dei quali stampa la propria costante
- **THEN** viene stampato solo `1`

#### Scenario: Costanti negative

- **WHEN** viene eseguito `switch i` con `i` uguale a -1 e rami `case 0`, `case -1`, `case -2`, ognuno
  dei quali stampa la propria costante
- **THEN** viene stampato solo `-1`

#### Scenario: Case con più costanti

- **WHEN** viene eseguito `switch x` con `x` uguale a 3 e un ramo `case 1,2,3` che stampa `piccolo`
- **THEN** viene stampato `piccolo`

#### Scenario: Selettore stringa

- **WHEN** viene eseguito `switch s` con `s` uguale a `"b"` e un ramo `case 'a','b',"c"`
- **THEN** viene eseguito quel ramo

#### Scenario: Selettore booleano

- **WHEN** viene eseguito `switch b` con `b` uguale a `false` e rami `case true` e `case false`
- **THEN** viene eseguito solo il ramo `case false`

### Requirement: Nessun fall-through

Dopo l'esecuzione delle istruzioni del ramo scelto l'esecuzione SHALL proseguire con
l'istruzione successiva all'`end` dello `switch`, senza eseguire i rami seguenti.

#### Scenario: Il ramo successivo non viene eseguito

- **WHEN** il selettore corrisponde a `case 0` e il ramo seguente è `case 1`
- **THEN** le istruzioni di `case 1` non vengono eseguite e l'esecuzione continua dopo l'`end`

### Requirement: Primo case corrispondente vince

Se più `case` contengono una costante uguale al selettore, lo `switch` SHALL eseguire solo il
primo nell'ordine del sorgente. Il compilatore non segnala le costanti duplicate.

#### Scenario: Costante ripetuta

- **WHEN** viene eseguito `switch i` con `i` uguale a 2, un ramo `case 1,2` che stampa `A` e un
  ramo successivo `case 2` che stampa `B`
- **THEN** viene stampato solo `A`

### Requirement: Ramo else

Quando nessun `case` corrisponde, lo `switch` SHALL eseguire le istruzioni del ramo `else` se
presente; se manca, SHALL non eseguire alcun ramo e proseguire dopo l'`end`.

#### Scenario: Else eseguito

- **WHEN** viene eseguito `switch i` con `i` uguale a 7, rami `case 0`, `case 1` e un ramo `else`
  che stampa `altro`
- **THEN** viene stampato solo `altro`

#### Scenario: Nessuna corrispondenza e nessun else

- **WHEN** viene eseguito uno `switch` senza `else` il cui selettore non corrisponde a nessun `case`
- **THEN** nessun ramo viene eseguito e il programma prosegue con l'istruzione dopo l'`end`

#### Scenario: Switch con solo else

- **WHEN** viene eseguito uno `switch` che contiene solo il ramo `else`
- **THEN** viene eseguito il ramo `else`

### Requirement: Costanti di tipo diverso dal selettore

Una costante di un `case` di tipo diverso dal valore del selettore SHALL essere considerata non
corrispondente, senza errore a runtime, anche quando il selettore è `nil`.

#### Scenario: Case misti int e stringa

- **WHEN** viene eseguito `switch x` con `x` uguale a `"a"`, un ramo `case 1,2,3` e un ramo
  `case 'a','b',"c"`
- **THEN** viene eseguito il secondo ramo e non viene segnalato alcun errore

#### Scenario: Selettore nil

- **WHEN** viene eseguito uno `switch` con selettore `nil`, rami `case 1` e un ramo `else`
- **THEN** viene eseguito il ramo `else`

### Requirement: Selettore valutato una sola volta

L'espressione del selettore SHALL essere valutata esattamente una volta per ogni esecuzione dello
`switch`, qualunque sia il numero di `case` e di costanti.

#### Scenario: Selettore con effetti collaterali

- **WHEN** il selettore è la chiamata di una funzione che stampa `chiamata` e lo `switch` ha tre
  `case` con due costanti ciascuno, nessuno dei quali corrisponde
- **THEN** `chiamata` viene stampato una sola volta

### Requirement: Stack invariato dallo switch

L'esecuzione di uno `switch` SHALL lasciare lo stack di esecuzione alla stessa profondità che aveva
prima del selettore, in ogni ramo e anche senza corrispondenze, inclusi `switch` annidati nei rami.

#### Scenario: Programma terminato con stack vuoto

- **WHEN** viene eseguito un programma che contiene `switch` annidati, con e senza `else`, in cui
  vengono percorsi sia rami `case` sia rami `else` sia nessun ramo
- **THEN** il programma termina normalmente con codice 0 e lo stack svuotato

### Requirement: Switch compilabile ed eseguibile

Un sorgente che contiene uno `switch` SHALL compilare senza errori e il suo p-code SHALL essere
eseguito senza l'errore `pcode not implemented`.

#### Scenario: Esempio di q.cpl

- **WHEN** viene compilato ed eseguito lo `switch i` con `case 0`, `case 1`, `case 2` ed `else`
  presente in `q.cpl`
- **THEN** il compilatore esce con codice 0 e l'esecuzione stampa il ramo corrispondente a `i`
