# Spec Delta

## MODIFIED Requirements

### Requirement: Esecuzione del p-code FOREACH_INIT

L'interprete SHALL eseguire `FOREACH_INIT` sostituendo la collezione in cima allo stack con un
iteratore posizionato prima del primo elemento: la profondità dello stack non cambia. Una
collezione che non è array, dizionario o tupla SHALL sollevare un errore. Con l'operando `2`
`FOREACH_INIT` SHALL invece sostituire le **due** collezioni in cima allo stack (la prima sotto, la
seconda in cima) con un unico iteratore parallelo su entrambe: la profondità diminuisce di 1, ognuna
delle due deve essere array, dizionario o tupla, e l'iteratore ammette `FOREACH_KEY` anche quando una
delle due è una tupla. Senza operando (o con `0`) il comportamento è quello a una collezione.

#### Scenario: Inizializzazione su un array

- **WHEN** viene eseguito `FOREACH_INIT` con l'array `[1,2]` in cima allo stack
- **THEN** in cima allo stack c'è un iteratore su quell'array e la profondità è invariata

#### Scenario: Valore non iterabile

- **WHEN** viene eseguito `FOREACH_INIT` con `nil` in cima allo stack
- **THEN** viene sollevato un errore

#### Scenario: Iteratore parallelo

- **WHEN** viene eseguito `FOREACH_INIT 2` con lo stack `[..., [1,2], ["a","b"]]`
- **THEN** lo stack diventa `[..., <iteratore>]`: un solo iteratore su entrambe le collezioni e la
  profondità diminuita di 1

#### Scenario: Iteratore parallelo su un valore non iterabile

- **WHEN** viene eseguito `FOREACH_INIT 2` con lo stack `[..., [1], 5]`
- **THEN** viene sollevato l'errore del `for each` su un valore non iterabile

### Requirement: Esecuzione del p-code FOREACH_NEXT

L'interprete SHALL eseguire `FOREACH_NEXT`, il cui operando intero è una label, con un iteratore in
cima allo stack: se non restano elementi SHALL saltare alla label; altrimenti SHALL avanzare
all'elemento successivo e assegnarne valore e chiave alle variabili registrate, con le regole di
`STORE`. Lo stack SHALL restare invariato in entrambi i casi. Su un iteratore parallelo il valore
SHALL essere l'elemento della prima collezione e la chiave l'elemento della seconda; SHALL saltare
alla label quando entrambe sono finite e SHALL sollevare un errore di lunghezze diverse quando ne è
finita una sola.

#### Scenario: Elemento disponibile

- **WHEN** viene eseguito `FOREACH_NEXT 3` per la prima volta su un iteratore dell'array `[7,8]`
  con `FOREACH_VAR v` e `FOREACH_KEY k` registrati
- **THEN** `v` vale `7`, `k` vale `0`, l'esecuzione prosegue con il p-code successivo e lo stack è
  invariato

#### Scenario: Collezione esaurita

- **WHEN** viene eseguito `FOREACH_NEXT 3` su un iteratore che ha già restituito tutti gli elementi
- **THEN** l'esecuzione prosegue dalla posizione della `LABEL 3`, le variabili sono invariate e lo
  stack è invariato

#### Scenario: Passo di un iteratore parallelo

- **WHEN** viene eseguito `FOREACH_NEXT 3` per la prima volta su un iteratore parallelo di `[7,8]` e
  `["a","b"]` con `FOREACH_VAR v` e `FOREACH_KEY k` registrati
- **THEN** `v` vale `7`, `k` vale `"a"` e lo stack è invariato

#### Scenario: Iteratore parallelo con lunghezze diverse

- **WHEN** viene eseguito `FOREACH_NEXT 3` su un iteratore parallelo di `[7]` e `["a","b"]` dopo
  che ha già restituito la prima coppia
- **THEN** viene sollevato un errore di lunghezze diverse e l'esecuzione non salta alla `LABEL 3`

### Requirement: Sicurezza GC dell'iteratore

L'iteratore SHALL rendere raggiungibile per il garbage collector la collezione che scorre, sia
durante la marcatura sia durante l'espansione delle generazioni, finché resta sullo stack.
L'iteratore parallelo SHALL rendere raggiungibili entrambe le collezioni.

#### Scenario: Collezione temporanea durante il ciclo

- **WHEN** un `for each` scorre un array costruito nell'espressione (`for each i in [1,2,3]`) e il
  corpo alloca abbastanza oggetti da far partire una raccolta
- **THEN** il ciclo visita tutti gli elementi dell'array senza errori

#### Scenario: Collezioni temporanee in parallelo

- **WHEN** una dict comprehension parallela scorre due array costruiti nell'espressione e il calcolo
  di chiave e valore alloca abbastanza oggetti da far partire una raccolta
- **THEN** la comprehension visita tutti gli elementi di entrambi gli array senza errori

### Requirement: Esecuzione del p-code COMPR_APPEND

L'interprete SHALL eseguire `COMPR_APPEND`, senza operando, con lo stack che contiene in cima un
valore, sotto di esso un iteratore e sotto ancora un array: SHALL aggiungere il valore in fondo a
quell'array e toglierlo dallo stack, lasciando array e iteratore al loro posto. Con l'operando `2`
lo stack SHALL contenere in cima un valore, sotto una chiave, sotto un iteratore e sotto ancora un
dizionario: SHALL inserire nel dizionario la coppia (chiave convertita in stringa come nella costante
dizionario, l'eventuale valore precedente sostituito) e togliere chiave e valore dallo stack.

#### Scenario: Aggiunta di un elemento

- **WHEN** viene eseguito `COMPR_APPEND` con lo stack `[..., [1], <iteratore>, 2]`
- **THEN** lo stack diventa `[..., [1,2], <iteratore>]`: la profondità diminuisce di 1 e l'array è
  lo stesso oggetto di prima, con un elemento in più

#### Scenario: Inserimento di una coppia

- **WHEN** viene eseguito `COMPR_APPEND 2` con lo stack `[..., {}, <iteratore>, 0, 7]`
- **THEN** lo stack diventa `[..., {0:7}, <iteratore>]`: la profondità diminuisce di 2 e il
  dizionario è lo stesso oggetto di prima

#### Scenario: Chiave già presente

- **WHEN** viene eseguito `COMPR_APPEND 2` con lo stack `[..., {a:1}, <iteratore>, "a", 2]`
- **THEN** il dizionario diventa `{a:2}`

### Requirement: COMPR_APPEND riconosciuto da interprete e asm

L'interprete SHALL riconoscere `COMPR_APPEND` nella fabbrica dei p-code (l'esecuzione non termina
con `not impl!`) e `./asm` SHALL mostrarlo con il suo nome nel listato, con l'operando quando c'è.

#### Scenario: Listato di asm

- **WHEN** `./asm` legge un `.pcd` compilato da `? [x for each x in [1,2]]`
- **THEN** il listato contiene la riga `COMPR_APPEND`

#### Scenario: Listato di asm di una dict comprehension

- **WHEN** `./asm` legge un `.pcd` compilato da `? {k:x for each x,k in [1,2]}`
- **THEN** il listato contiene la riga `COMPR_APPEND 2`
