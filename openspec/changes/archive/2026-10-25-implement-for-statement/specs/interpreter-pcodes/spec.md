# Spec Delta

## ADDED Requirements

### Requirement: Esecuzione dei p-code FOR_TO e FOR_DOWNTO

L'interprete SHALL eseguire `FOR_TO` e `FOR_DOWNTO`, il cui operando stringa è il nome della
variabile di ciclo, con lo stack che contiene in cima inizio, fine e passo (il passo in cima):
SHALL togliere l'inizio, assegnarlo alla variabile e lasciare sullo stack fine e passo.

#### Scenario: Inizializzazione con to

- **WHEN** viene eseguito `FOR_TO i` con `i` dichiarata nel contesto corrente e lo stack che
  contiene `1`, `10`, `2`
- **THEN** `i` vale `1` e lo stack contiene `10` e sopra `2`

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `FOR_TO` o `FOR_DOWNTO` senza errori
- **THEN** la profondità dello stack diminuisce di 1

### Requirement: Controlli di FOR_TO e FOR_DOWNTO

`FOR_TO` e `FOR_DOWNTO` SHALL sollevare un errore a runtime, senza assegnare la variabile, quando
il nome non è dichiarato nel contesto corrente (anche se è visibile da un livello superiore),
quando inizio, fine o passo non sono `int`, o quando il passo non è strettamente positivo per
`FOR_TO` o strettamente negativo per `FOR_DOWNTO`.

#### Scenario: Nome non locale

- **WHEN** viene eseguito `FOR_TO i` in un contesto che non dichiara `i`, mentre il suo livello
  superiore la dichiara
- **THEN** viene sollevato un errore il cui messaggio contiene `i`

#### Scenario: Passo di segno sbagliato

- **WHEN** viene eseguito `FOR_DOWNTO i` con passo `1`
- **THEN** viene sollevato un errore

#### Scenario: Passo nullo

- **WHEN** viene eseguito `FOR_TO i` con passo `0`
- **THEN** viene sollevato un errore

#### Scenario: Valore non intero

- **WHEN** viene eseguito `FOR_TO i` con fine `nil`
- **THEN** viene sollevato un errore

### Requirement: Esecuzione del p-code FOR_TEST

L'interprete SHALL eseguire `FOR_TEST`, il cui operando intero è una label, con lo stack che
contiene fine, passo e in cima il valore corrente della variabile: SHALL togliere il valore e
saltare alla label quando il valore supera la fine nella direzione del passo (maggiore con passo
positivo, minore con passo negativo); altrimenti SHALL proseguire. Fine e passo restano sullo
stack. Un valore non `int` SHALL sollevare un errore.

#### Scenario: Dentro il limite

- **WHEN** viene eseguito `FOR_TEST 4` con lo stack `10`, `1`, `10`
- **THEN** l'esecuzione prosegue con il p-code successivo e lo stack contiene `10`, `1`

#### Scenario: Oltre il limite crescente

- **WHEN** viene eseguito `FOR_TEST 4` con lo stack `10`, `1`, `11`
- **THEN** l'esecuzione prosegue dalla posizione della `LABEL 4` e lo stack contiene `10`, `1`

#### Scenario: Oltre il limite decrescente

- **WHEN** viene eseguito `FOR_TEST 4` con lo stack `1`, `-2`, `0`
- **THEN** l'esecuzione prosegue dalla posizione della `LABEL 4`

### Requirement: Esecuzione del p-code FOR_NEXT

L'interprete SHALL eseguire `FOR_NEXT`, il cui operando stringa è il nome della variabile, sommando
alla variabile il passo che sta in cima allo stack e assegnando il risultato alla variabile con le
stesse regole di `STORE`. Lo stack SHALL restare invariato. Un valore della variabile non `int`
SHALL sollevare un errore.

#### Scenario: Incremento

- **WHEN** viene eseguito `FOR_NEXT i` con `i` uguale a `3` e lo stack `10`, `2`
- **THEN** `i` vale `5` e lo stack contiene ancora `10`, `2`

### Requirement: I p-code del for non sono p-code non implementati

L'interprete SHALL riconoscere `FOR_TO`, `FOR_DOWNTO`, `FOR_TEST` e `FOR_NEXT` nella fabbrica dei
p-code: l'esecuzione di un `.pcd` che li contiene non SHALL terminare con l'errore
`pcode not implemented`.

#### Scenario: Programma cpl2 con for

- **WHEN** viene eseguito un `.pcd` compilato da un sorgente che contiene un `for`
- **THEN** il programma si esegue senza l'errore `pcode not implemented`

### Requirement: Rappresentazione leggibile dei p-code del for

I p-code SHALL avere i nomi testuali `FOR_TO`, `FOR_DOWNTO`, `FOR_TEST` e `FOR_NEXT`, così che
`./asm` li mostri con il loro operando e la conversione inversa del listato li riconosca.

#### Scenario: Listato di asm

- **WHEN** `./asm` legge un `.pcd` compilato da `for i:=1 to 3 ... end`
- **THEN** il listato contiene le righe `FOR_TO i`, `FOR_TEST <n>` e `FOR_NEXT i`
