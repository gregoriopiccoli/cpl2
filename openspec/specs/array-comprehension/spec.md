# array-comprehension Specification

## Purpose

Definisce la semantica dell'espressione `[e for each v[,k] in <collezione> [with <condizione>]]`
di cpl2: l'array che produce, l'ordine dei suoi elementi, il filtro `with`, le regole sulle
variabili condivise con lo statement `for each` e il comportamento del programma compilato.

## Requirements

### Requirement: Array prodotto da una comprehension

Un'espressione `[e for each v in x]` SHALL produrre un array nuovo con un elemento per ogni
elemento visitato di `x`: il valore di `e` calcolato con `v` uguale all'elemento corrente.
L'array SHALL essere un valore come gli altri: si può stampare, assegnare, passare e annidare.

#### Scenario: Identità su un array

- **WHEN** viene eseguito `? [x for each x in [1,2,3]]` con `x` dichiarata `int`
- **THEN** viene stampato `[1,2,3]`

#### Scenario: Espressione calcolata

- **WHEN** viene eseguito `? [x*x for each x in [1,2,3]]`
- **THEN** viene stampato `[1,4,9]`

#### Scenario: Assegnazione a una variabile array

- **WHEN** con `var array[int] a` viene eseguito `a:=[x for each x in [1,2,3]]` e poi `? a`
- **THEN** viene stampato `[1,2,3]` e il programma termina con exit 0, anche con `--typecheck`

#### Scenario: Collezione vuota

- **WHEN** viene eseguito `? [x for each x in []]`
- **THEN** viene stampato `[]`

### Requirement: Ordine degli elementi e collezioni ammesse

Gli elementi dell'array prodotto SHALL seguire l'ordine di visita dello statement `for each` sulla
stessa collezione: array per indice crescente, tuple per posizione, dizionari in ordine non
specificato. Ogni valore non iterabile dal `for each` SHALL essere un errore anche qui.

#### Scenario: Tupla

- **WHEN** viene eseguito `? [v for each v in |1,2|]` con `v` dichiarata `any`
- **THEN** viene stampato `[1,2]`

#### Scenario: Dizionario a una chiave

- **WHEN** viene eseguito `? [i for each i in {"alfa":100}]`
- **THEN** viene stampato `[100]`

#### Scenario: Collezione non iterabile

- **WHEN** viene eseguito `? [i for each i in 5]`
- **THEN** il programma stampa l'errore del `for each` su un valore non iterabile e termina con
  exit diverso da 0 senza stampare l'array

### Requirement: Seconda variabile della comprehension

Un'espressione `[e for each v,k in x]` SHALL assegnare a `k`, per ogni elemento, la stessa chiave
che riceve lo statement `for each v,k in x` (indice `int` da 0 per gli array, chiave `str` per i
dizionari); con una tupla SHALL essere un errore come nello statement.

#### Scenario: Indici di un array

- **WHEN** viene eseguito `? [k for each i,k in [7,8,9]]`
- **THEN** viene stampato `[0,1,2]`

### Requirement: Filtro with

Con `with <condizione>` la condizione SHALL essere valutata per ogni elemento dopo l'assegnazione
delle variabili e prima di `e`; solo gli elementi per cui vale `true` SHALL produrre un elemento
dell'array, e `e` non SHALL essere valutata per gli altri. La condizione deve essere `bool`.

#### Scenario: Filtro sul valore

- **WHEN** viene eseguito `? [x for each x in [1,2,3] with x>1]`
- **THEN** viene stampato `[2,3]`

#### Scenario: Filtro mai vero

- **WHEN** viene eseguito `? [x for each x in [1,2,3] with x>5]`
- **THEN** viene stampato `[]`

#### Scenario: Espressione non valutata sugli elementi scartati

- **WHEN** viene eseguito `? [f(x) for each x in [1,2,3] with x>1]` con `f` una func che stampa il
  proprio argomento e lo restituisce
- **THEN** `f` stampa solo `2` e `3`, poi viene stampato `[2,3]`

### Requirement: Valutazione della collezione e dell'espressione

La collezione SHALL essere valutata una sola volta, prima del primo elemento; `e` (e la condizione)
SHALL essere valutate una volta per elemento, nell'ordine di visita.

#### Scenario: Collezione restituita da una funzione

- **WHEN** la collezione è la chiamata a una func che stampa `chiamata` e restituisce `[1,2]`
- **THEN** `chiamata` viene stampato una volta sola e l'array prodotto è `[1,2]`

### Requirement: Variabili della comprehension

`v` e `k` SHALL seguire le regole dello statement `for each`: devono essere dichiarate nel contesto
corrente (una variabile visibile solo da un livello superiore, o non dichiarata, è un errore a
runtime che nomina la variabile), la comprehension non le dichiara e dopo l'espressione SHALL
contenere l'ultimo valore assegnato (invariate con zero elementi).

#### Scenario: Valore dopo la comprehension

- **WHEN** con `var int x:=0` viene eseguito `a:=[x for each x in [1,2,3]]` e poi `? x`
- **THEN** viene stampato `3`

#### Scenario: Variabile non dichiarata

- **WHEN** viene eseguito `? [z for each z in [1]]` senza dichiarare `z`
- **THEN** il programma stampa un errore che contiene `z` e termina con exit diverso da 0

### Requirement: Comprehension annidate

Una comprehension SHALL poter comparire dentro l'espressione `e`, dentro la collezione o dentro la
condizione di un'altra comprehension, con ciascuna che produce il proprio array.

#### Scenario: Comprehension nell'espressione

- **WHEN** con `x` e `y` dichiarate `int` viene eseguito
  `? [[x*y for each y in [1,2]] for each x in [1,2]]`
- **THEN** viene stampato `[[1,2],[2,4]]`

#### Scenario: Comprehension nella collezione

- **WHEN** viene eseguito `? [x+1 for each x in [x*10 for each x in [1,2]]]`
- **THEN** viene stampato `[11,21]`

### Requirement: Comprehension su più righe

Una comprehension SHALL produrre lo stesso risultato quando è scritta su più righe dentro le
parentesi quadre.

#### Scenario: A capo prima di for each

- **WHEN** viene eseguito `? [x*2` seguito su una nuova riga da `for each x in [1,2]]`
- **THEN** viene stampato `[2,4]`

### Requirement: Esempi di q.cpl

Le tre comprehension in fondo a `q.cpl` SHALL compilarsi ed eseguirsi.

#### Scenario: Fine di q.cpl

- **WHEN** viene eseguito `./compiler q.cpl && ./interpreter q`
- **THEN** il programma termina con exit 0 e, dopo `array comprehension`, stampa `[1,2,3]` e
  `[4,6]` (la seconda è `[x*2 for each x in [1,2,3] with x>1]`)

### Requirement: Nessun COMMENT per le array comprehension

Il p-code di un'array comprehension non SHALL contenere `COMMENT "da fare array comprehension ..."`
né altri p-code non implementati: l'esecuzione non SHALL terminare con `not impl!`.

#### Scenario: Listato di una comprehension

- **WHEN** viene compilato un sorgente con `? [x for each x in [1,2,3] with x>1]` e il `.pcd` viene
  letto con `./asm`
- **THEN** il listato non contiene `COMMENT` e contiene `FOREACH_INIT`, `FOREACH_NEXT` e
  `COMPR_APPEND`
