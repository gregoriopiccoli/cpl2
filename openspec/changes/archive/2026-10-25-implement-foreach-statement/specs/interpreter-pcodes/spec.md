# Spec Delta

## ADDED Requirements

### Requirement: Esecuzione del p-code FOREACH_INIT

L'interprete SHALL eseguire `FOREACH_INIT` sostituendo la collezione in cima allo stack con un
iteratore posizionato prima del primo elemento: la profondità dello stack non cambia. Una
collezione che non è array, dizionario o tupla SHALL sollevare un errore.

#### Scenario: Inizializzazione su un array

- **WHEN** viene eseguito `FOREACH_INIT` con l'array `[1,2]` in cima allo stack
- **THEN** in cima allo stack c'è un iteratore su quell'array e la profondità è invariata

#### Scenario: Valore non iterabile

- **WHEN** viene eseguito `FOREACH_INIT` con `nil` in cima allo stack
- **THEN** viene sollevato un errore

### Requirement: Esecuzione dei p-code FOREACH_VAR e FOREACH_KEY

L'interprete SHALL eseguire `FOREACH_VAR` e `FOREACH_KEY`, il cui operando stringa è un nome, con
un iteratore in cima allo stack: SHALL registrare il nome come destinazione del valore
(`FOREACH_VAR`) o della chiave (`FOREACH_KEY`) e lasciare lo stack invariato, senza assegnare
nulla.

#### Scenario: Registrazione delle variabili

- **WHEN** vengono eseguiti `FOREACH_VAR i` e `FOREACH_KEY k` con `i` e `k` dichiarate nel contesto
  corrente e un iteratore su array in cima allo stack
- **THEN** lo stack è invariato e `i` e `k` conservano il valore che avevano

### Requirement: Controlli di FOREACH_VAR e FOREACH_KEY

`FOREACH_VAR` e `FOREACH_KEY` SHALL sollevare un errore il cui messaggio contiene il nome quando il
nome non è dichiarato nel contesto corrente (anche se visibile da un livello superiore).
`FOREACH_KEY` SHALL sollevare un errore anche quando l'iteratore scorre una tupla.

#### Scenario: Nome non locale

- **WHEN** viene eseguito `FOREACH_VAR i` in un contesto che non dichiara `i`, mentre il suo
  livello superiore la dichiara
- **THEN** viene sollevato un errore il cui messaggio contiene `i`

#### Scenario: Chiave su una tupla

- **WHEN** viene eseguito `FOREACH_KEY k` con `k` locale e un iteratore su tupla in cima allo stack
- **THEN** viene sollevato un errore

### Requirement: Esecuzione del p-code FOREACH_NEXT

L'interprete SHALL eseguire `FOREACH_NEXT`, il cui operando intero è una label, con un iteratore in
cima allo stack: se non restano elementi SHALL saltare alla label; altrimenti SHALL avanzare
all'elemento successivo e assegnarne valore e chiave alle variabili registrate, con le regole di
`STORE`. Lo stack SHALL restare invariato in entrambi i casi.

#### Scenario: Elemento disponibile

- **WHEN** viene eseguito `FOREACH_NEXT 3` per la prima volta su un iteratore dell'array `[7,8]`
  con `FOREACH_VAR v` e `FOREACH_KEY k` registrati
- **THEN** `v` vale `7`, `k` vale `0`, l'esecuzione prosegue con il p-code successivo e lo stack è
  invariato

#### Scenario: Collezione esaurita

- **WHEN** viene eseguito `FOREACH_NEXT 3` su un iteratore che ha già restituito tutti gli elementi
- **THEN** l'esecuzione prosegue dalla posizione della `LABEL 3`, le variabili sono invariate e lo
  stack è invariato

### Requirement: Sicurezza GC dell'iteratore

L'iteratore SHALL rendere raggiungibile per il garbage collector la collezione che scorre, sia
durante la marcatura sia durante l'espansione delle generazioni, finché resta sullo stack.

#### Scenario: Collezione temporanea durante il ciclo

- **WHEN** un `for each` scorre un array costruito nell'espressione (`for each i in [1,2,3]`) e il
  corpo alloca abbastanza oggetti da far partire una raccolta
- **THEN** il ciclo visita tutti gli elementi dell'array senza errori

### Requirement: I p-code del for each non sono p-code non implementati

L'interprete SHALL riconoscere `FOREACH_INIT`, `FOREACH_VAR`, `FOREACH_KEY` e `FOREACH_NEXT` nella
fabbrica dei p-code: l'esecuzione di un `.pcd` che li contiene non SHALL terminare con l'errore
`pcode not implemented`.

#### Scenario: Programma cpl2 con for each

- **WHEN** viene eseguito un `.pcd` compilato da un sorgente che contiene un `for each`
- **THEN** il programma si esegue senza l'errore `pcode not implemented`

### Requirement: Rappresentazione leggibile dei p-code del for each

I p-code SHALL avere i nomi testuali `FOREACH_INIT`, `FOREACH_VAR`, `FOREACH_KEY` e
`FOREACH_NEXT`, così che `./asm` li mostri con il loro operando e la conversione inversa del
listato li riconosca.

#### Scenario: Listato di asm

- **WHEN** `./asm` legge un `.pcd` compilato da `for each v,k in a ... end`
- **THEN** il listato contiene le righe `FOREACH_INIT`, `FOREACH_VAR v`, `FOREACH_KEY k` e
  `FOREACH_NEXT <n>`
