# Spec Delta

## ADDED Requirements

### Requirement: PARM_DEFAULT e ENDPARM_DEFAULT non sono più p-code non implementati

L'interprete SHALL riconoscere `PARM_DEFAULT` e `ENDPARM_DEFAULT` nella fabbrica dei p-code: definire e chiamare una proc o una func che usa parametri con valore di default non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma cpl2 con proc con parametro di default

- **WHEN** viene eseguito un `.pcd` compilato da un sorgente che dichiara `proc p3(int i:=100)` e la chiama senza argomenti
- **THEN** il programma si esegue senza l'errore `pcode not implemented` e arriva alla fine con exit 0

#### Scenario: Programma cpl2 con func con parametro di default

- **WHEN** viene eseguito un `.pcd` compilato da un sorgente che dichiara una func con un parametro inizializzato e la chiama senza quel parametro
- **THEN** il programma si esegue senza l'errore `pcode not implemented`

### Requirement: Registrazione del parametro con default e salto dell'espressione alla definizione

Durante la creazione della lista dei parametri di una proc o func, `PARM_DEFAULT` SHALL registrare il parametro consumando il tipo dalla cima dello stack come fa `PARM`, e l'espressione di default che segue SHALL essere saltata fino a `ENDPARM_DEFAULT` senza essere valutata alla definizione.

#### Scenario: Definizione con espressione di default non valutabile

- **WHEN** un sorgente definisce una proc il cui parametro di default referenzia un nome inesistente e la chiamata passa quel parametro
- **THEN** la definizione della proc riesce senza errori e la chiamata si esegue regolarmente

### Requirement: Valore di default calcolato alla chiamata per i parametri non passati

Quando un parametro con valore di default non riceve un valore dalla chiamata, l'interprete SHALL eseguire i p-code dell'espressione di default fino a `ENDPARM_DEFAULT`, nel contesto della chiamata, e SHALL assegnare al parametro il valore risultante.

#### Scenario: Default costante

- **WHEN** una proc dichiarata `proc p3(int i:=100)` viene chiamata senza argomenti
- **THEN** il parametro `i` vale `100`

#### Scenario: Espressione di default non costante

- **WHEN** una proc dichiarata `proc p(int a, int b:=a*2)` viene chiamata come `p(5)`
- **THEN** il parametro `a` vale `5` e il parametro `b` vale `10`

### Requirement: Valore passato usato al posto del default e espressione saltata

Quando un parametro con valore di default riceve un valore dalla chiamata, l'interprete SHALL assegnare al parametro il valore passato nello stack e SHALL saltare i p-code dell'espressione di default fino a `ENDPARM_DEFAULT`, senza valutarli.

#### Scenario: Chiamata che passa tutti i parametri

- **WHEN** una proc dichiarata `proc p3(int i:=100)` viene chiamata come `p3(7)`
- **THEN** il parametro `i` vale `7`

#### Scenario: Espressione di default che non viene valutata

- **WHEN** una proc ha un parametro con default che produrrebbe un errore a runtime (per esempio un nome inesistente) e la chiamata passa quel parametro
- **THEN** l'espressione di default non viene valutata e la chiamata si esegue regolarmente

### Requirement: Chiamata parziale posizionale con parametri di default

All'inizializzazione dei parametri SHALL valere l'ordine posizionale: i parametri a partire da sinistra ricevono i valori passati finché ce ne sono, e quelli successivi — se hanno un valore di default — lo assumono calcolando la rispettiva espressione.

#### Scenario: Chiamata che passa solo i primi parametri

- **WHEN** una proc dichiarata `proc p(int a, int b:=10, int c:=20)` viene chiamata come `p(1)`
- **THEN** `a` vale `1`, `b` vale `10` e `c` vale `20`

### Requirement: Esecuzione del p-code ENDPARM_DEFAULT

`ENDPARM_DEFAULT` SHALL terminare l'esecuzione della valutazione dell'espressione di default senza consumare né aggiungere valori sullo stack di esecuzione: il valore in cima, prodotto dall'espressione, resta il valore che l'interprete assegna al parametro.

#### Scenario: Fine della valutazione del default

- **WHEN** durante la chiamata l'espressione di default di un parametro non passato viene eseguita fino a `ENDPARM_DEFAULT`
- **THEN** la valutazione termina e il parametro assume il valore che era in cima allo stack

#### Scenario: Effetto sullo stack di ENDPARM_DEFAULT

- **WHEN** viene eseguito `ENDPARM_DEFAULT` con l'espressione valutata in cima allo stack
- **THEN** la profondità dello stack di esecuzione resta invariata e la cima resta il valore calcolato
