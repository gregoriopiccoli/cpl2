# for-statement Specification

## Purpose

Definisce la semantica dello statement `for` numerico di cpl2 (`for v:=inizio to|downto fine
[step passo] ... end`), il requisito della variabile di ciclo locale e il comportamento del
programma compilato che lo contiene.

## Requirements

### Requirement: Ciclo crescente con to

Un `for v:=a to b` SHALL eseguire il corpo una volta per ogni valore di `v` da `a` a `b` inclusi,
in ordine crescente, con passo 1 quando `step` è assente.

#### Scenario: to senza step

- **WHEN** viene eseguito `for i:=1 to 10` con un corpo che stampa `i`
- **THEN** vengono stampati i numeri da 1 a 10, uno per riga, in ordine crescente

#### Scenario: Inizio uguale alla fine

- **WHEN** viene eseguito `for i:=3 to 3` con un corpo che stampa `i`
- **THEN** viene stampato solo `3`

### Requirement: Ciclo decrescente con downto

Un `for v:=a downto b` SHALL eseguire il corpo una volta per ogni valore di `v` da `a` a `b`
inclusi, in ordine decrescente, con passo -1 quando `step` è assente.

#### Scenario: downto senza step

- **WHEN** viene eseguito `for i:=10 downto 1` con un corpo che stampa `i`
- **THEN** vengono stampati i numeri da 10 a 1, uno per riga, in ordine decrescente

### Requirement: Passo con segno

Il valore di `step` SHALL essere l'incremento con segno sommato alla variabile dopo ogni
iterazione: positivo con `to`, negativo con `downto`. Il ciclo SHALL terminare appena la
variabile supera la fine (maggiore con `to`, minore con `downto`), anche se non la raggiunge
esattamente.

#### Scenario: to con step 2

- **WHEN** viene eseguito `for i:=1 to 10 step 2` con un corpo che stampa `i`
- **THEN** vengono stampati `1`, `3`, `5`, `7`, `9`

#### Scenario: downto con step -2

- **WHEN** viene eseguito `for i:=10 downto 1 step -2` con un corpo che stampa `i`
- **THEN** vengono stampati `10`, `8`, `6`, `4`, `2`

#### Scenario: Passo che raggiunge esattamente la fine

- **WHEN** viene eseguito `for i:=0 to 9 step 3` con un corpo che stampa `i`
- **THEN** vengono stampati `0`, `3`, `6`, `9`

### Requirement: Passo nullo o di segno incoerente

Un passo uguale a 0, un passo negativo con `to` o un passo positivo con `downto` SHALL
interrompere il programma con un errore a runtime prima della prima iterazione, invece di
produrre un ciclo infinito o vuoto.

#### Scenario: step 0

- **WHEN** viene eseguito `for i:=1 to 10 step 0`
- **THEN** il corpo non viene eseguito e il programma si interrompe con un messaggio d'errore e
  codice di uscita diverso da 0

#### Scenario: to con step negativo

- **WHEN** viene eseguito `for i:=1 to 10 step -1`
- **THEN** il corpo non viene eseguito e il programma si interrompe con un messaggio d'errore

#### Scenario: downto con step positivo

- **WHEN** viene eseguito `for i:=10 downto 1 step 2`
- **THEN** il corpo non viene eseguito e il programma si interrompe con un messaggio d'errore

### Requirement: Nessuna iterazione se l'inizio è oltre la fine

Quando l'inizio è già oltre la fine (maggiore con `to`, minore con `downto`) il `for` SHALL non
eseguire il corpo e proseguire con l'istruzione dopo l'`end`, senza errori.

#### Scenario: to vuoto

- **WHEN** viene eseguito `for i:=5 to 1` con un corpo che stampa `i`, seguito da `? 'fine'`
- **THEN** viene stampato solo `fine`

#### Scenario: downto vuoto

- **WHEN** viene eseguito `for i:=1 downto 5` con un corpo che stampa `i`, seguito da `? 'fine'`
- **THEN** viene stampato solo `fine`

### Requirement: Variabile di ciclo locale

La variabile di ciclo SHALL essere già dichiarata nel contesto corrente (il modulo per il codice
di livello esterno, la funzione o procedura per il codice nel suo corpo). Se il nome non è
dichiarato nel contesto corrente, anche se esiste in un livello superiore, il programma SHALL
interrompersi con un errore che nomina la variabile, prima di eseguire il corpo.

#### Scenario: Variabile dichiarata nel modulo

- **WHEN** a livello esterno viene dichiarato `var int i` e poi eseguito `for i:=1 to 3`
- **THEN** il ciclo viene eseguito normalmente

#### Scenario: Variabile dichiarata nella procedura

- **WHEN** una procedura dichiara `var int k` e nel suo corpo esegue `for k:=1 to 3`
- **THEN** il ciclo viene eseguito normalmente

#### Scenario: Variabile non dichiarata

- **WHEN** viene eseguito `for j:=1 to 3` senza che `j` sia mai stato dichiarato
- **THEN** il corpo non viene eseguito e il programma si interrompe con un messaggio che contiene
  `j`

#### Scenario: Variabile visibile solo dal livello superiore

- **WHEN** il modulo dichiara `var int i` e una procedura dello stesso modulo, che non dichiara
  `i`, esegue `for i:=1 to 3`
- **THEN** il corpo non viene eseguito e il programma si interrompe con un messaggio che contiene
  `i`

### Requirement: Valori interi

Inizio, fine e passo SHALL essere valori `int`; un valore di altro tipo (stringa, bool, `nil`)
SHALL interrompere il programma con un errore a runtime prima della prima iterazione.

#### Scenario: Fine stringa

- **WHEN** viene eseguito `for i:=1 to 'dieci'`
- **THEN** il corpo non viene eseguito e il programma si interrompe con un messaggio d'errore

### Requirement: Fine e passo valutati una sola volta

Le espressioni di fine e di passo SHALL essere valutate esattamente una volta, prima della prima
iterazione, nell'ordine inizio, fine, passo; cambiare nel corpo le variabili da cui dipendono non
SHALL modificare il numero di iterazioni.

#### Scenario: Fine modificata nel corpo

- **WHEN** viene eseguito `for i:=1 to n` con `n` uguale a 3 e un corpo che stampa `i` ed esegue
  `n:=10`
- **THEN** vengono stampati solo `1`, `2`, `3`

#### Scenario: Fine con effetti collaterali

- **WHEN** la fine è la chiamata di una funzione che stampa `chiamata` e restituisce 3
- **THEN** `chiamata` viene stampato una sola volta e il corpo viene eseguito tre volte

### Requirement: Variabile riletta a ogni iterazione

Il test di fine ciclo e l'incremento SHALL usare il valore che la variabile ha nel contesto in
quel momento, quindi un'assegnazione alla variabile dentro il corpo SHALL influenzare le
iterazioni successive.

#### Scenario: Salto in avanti dal corpo

- **WHEN** viene eseguito `for i:=1 to 10` con un corpo che stampa `i` ed esegue `i:=i+4`
- **THEN** vengono stampati `1`, `6`

### Requirement: Valore della variabile dopo il ciclo

Al termine normale del ciclo la variabile SHALL contenere il primo valore oltre la fine
(l'ultimo valore del corpo più il passo); se il corpo non è mai stato eseguito SHALL contenere
l'inizio.

#### Scenario: Dopo un to completo

- **WHEN** dopo `for i:=1 to 10 ... end` viene stampato `i`
- **THEN** viene stampato `11`

#### Scenario: Dopo un ciclo vuoto

- **WHEN** dopo `for i:=5 to 1 ... end` viene stampato `i`
- **THEN** viene stampato `5`

### Requirement: Cicli annidati e stack invariato

Un `for` SHALL poter contenere altri `for` (con variabili diverse) e altri statement di
controllo, e la sua esecuzione completa SHALL lasciare lo stack di esecuzione alla stessa
profondità che aveva prima del `for`.

#### Scenario: Tabellina

- **WHEN** viene eseguito `for i:=1 to 3` con dentro `for j:=1 to 3` che stampa `i*j`
- **THEN** vengono stampati nove valori nell'ordine `1 2 3 2 4 6 3 6 9` e il programma termina
  con codice 0 e lo stack svuotato

#### Scenario: For dentro una procedura chiamata più volte

- **WHEN** una procedura con una variabile locale esegue un `for` e viene chiamata due volte
- **THEN** entrambe le chiamate eseguono il ciclo completo e il programma termina con codice 0

### Requirement: For compilabile ed eseguibile

Un sorgente che contiene un `for` in una delle quattro forme SHALL compilare senza errori e il suo
p-code SHALL essere eseguito senza l'errore `pcode not implemented`.

#### Scenario: Esempi di q.cpl

- **WHEN** vengono compilati ed eseguiti i quattro `for` di `q.cpl` (`to`, `to ... step 2`,
  `downto`, `downto ... step -2`)
- **THEN** il compilatore esce con codice 0 e l'esecuzione stampa le quattro sequenze
  `1..10`, `1 3 5 7 9`, `10..1`, `10 8 6 4 2`
