# dict-comprehension Specification

## Purpose

Definisce la semantica delle espressioni `{ke:ve for each v,k in x [with c]}` e
`{ke:ve for each v:w in A : B [with c]}` di cpl2: il dizionario che producono, le chiavi, la visita
parallela di due collezioni, il filtro `with`, le variabili condivise con il `for each` e gli errori.

## Requirements

### Requirement: Dizionario prodotto da una comprehension con chiave

Un'espressione `{ke:ve for each v,k in x}` SHALL produrre un dizionario nuovo con una coppia per ogni
elemento visitato di `x`: chiave `ke` e valore `ve`, calcolati con `v` e `k` uguali al valore e alla
chiave che riceverebbe lo statement `for each v,k in x`. Il dizionario SHALL essere un valore come
gli altri: si può stampare, assegnare, passare e annidare.

#### Scenario: Indice come chiave

- **WHEN** con `x` e `k` dichiarate `int` viene eseguito `? {k:x for each x,k in [7]}`
- **THEN** viene stampato `{0:7}`

#### Scenario: Chiave e valore calcolati

- **WHEN** con `x` e `k` dichiarate `int` viene eseguito `d:={k*10:x*2 for each x,k in [1,2,3]}` e
  poi `? d["20"]`
- **THEN** viene stampato `6`

#### Scenario: Dizionario come collezione

- **WHEN** con `i` dichiarata `int` e `s` dichiarata `str` viene eseguito
  `? {s:i+1 for each i,s in {"alfa":100}}`
- **THEN** viene stampato `{alfa:101}`

#### Scenario: Assegnazione a una variabile dizionario

- **WHEN** con `var dict[int] d` viene eseguito `d:={k:x for each x,k in [1,2]}` e poi `? d["1"]`
- **THEN** viene stampato `2` e il programma termina con exit 0, anche con `--typecheck`

#### Scenario: Collezione vuota

- **WHEN** viene eseguito `? {k:x for each x,k in []}`
- **THEN** viene stampato `{}`

### Requirement: Chiavi del dizionario prodotto

La chiave `ke` SHALL essere convertita in stringa come nella costante dizionario (la forma stampata
del valore). Quando due elementi producono la stessa chiave il dizionario SHALL contenere una sola
coppia per quella chiave, con il valore dell'ultimo elemento visitato.

#### Scenario: Chiave intera letta con una stringa

- **WHEN** viene eseguito `d:={k:x for each x,k in [5,6]}` e poi `? d["0"]`
- **THEN** viene stampato `5`

#### Scenario: Chiave ripetuta

- **WHEN** viene eseguito `? {"c":x for each x,k in [1,2,3]}`
- **THEN** viene stampato `{c:3}`

### Requirement: Collezioni della forma con chiave

La forma `{ke:ve for each v,k in x}` SHALL accettare le collezioni che accetta `for each v,k in x`,
con lo stesso ordine di visita (array per indice crescente, dizionari in ordine non specificato).
Una tupla SHALL essere un errore come nello statement, perché non ha chiave; un valore non iterabile
SHALL essere l'errore del `for each`.

#### Scenario: Tupla

- **WHEN** viene eseguito `? {k:v for each v,k in |1,2|}`
- **THEN** il programma stampa l'errore del `for each` sulle tuple senza chiave e termina con exit
  diverso da 0 senza stampare un dizionario

#### Scenario: Valore non iterabile

- **WHEN** viene eseguito `? {k:x for each x,k in 5}`
- **THEN** il programma stampa l'errore del `for each` su un valore non iterabile e termina con exit
  diverso da 0

### Requirement: Visita parallela di due collezioni

Un'espressione `{ke:ve for each v:w in A : B}` SHALL visitare `A` e `B` insieme, posizione per
posizione: all'i-esimo passo `v` SHALL ricevere l'i-esimo valore di `A` e `w` l'i-esimo valore di
`B`, ciascuno nell'ordine di visita del `for each` sulla propria collezione (array, tuple e
dizionari, questi ultimi in ordine non specificato). Il risultato SHALL essere un dizionario con una
coppia `ke:ve` per passo.

#### Scenario: Esempio di q.cpl

- **WHEN** con `x` dichiarata `int` e `k` dichiarata `str` viene eseguito
  `d:={k:x for each x:k in [1,2,3] : ["alfa","beta","gamma"]}` e poi `? d["alfa"],d["beta"],d["gamma"]`
- **THEN** viene stampato `123`

#### Scenario: Tupla in parallelo

- **WHEN** con `v` dichiarata `any` e `s` dichiarata `str` viene eseguito
  `d:={s:v for each v:s in |1,"due"| : ["a","b"]}` e poi `? d["b"]`
- **THEN** viene stampato `due`

#### Scenario: Due collezioni vuote

- **WHEN** viene eseguito `? {k:x for each x:k in [] : []}`
- **THEN** viene stampato `{}`

### Requirement: Collezioni parallele di lunghezza diversa

Nella forma `for each v:w in A : B`, quando una delle due collezioni termina prima dell'altra il
programma SHALL fermarsi con un errore a runtime che dice che le collezioni hanno lunghezze diverse,
con exit diverso da 0. Le coppie già calcolate non SHALL essere stampate né assegnate.

#### Scenario: Prima collezione più corta

- **WHEN** viene eseguito `? {k:x for each x:k in [1,2] : ["a","b","c"]}`
- **THEN** il programma stampa l'errore di lunghezze diverse e termina con exit diverso da 0 senza
  stampare un dizionario

#### Scenario: Seconda collezione più corta

- **WHEN** viene eseguito `d:={k:x for each x:k in [1,2,3] : ["a"]}`
- **THEN** il programma stampa l'errore di lunghezze diverse, termina con exit diverso da 0 e `d`
  non viene assegnata

### Requirement: Filtro with nelle dict comprehension

Con `with <condizione>`, in entrambe le forme, la condizione SHALL essere valutata per ogni elemento
dopo l'assegnazione delle variabili e prima di `ke` e `ve`; solo gli elementi per cui vale `true`
SHALL produrre una coppia, e `ke`/`ve` non SHALL essere valutate per gli altri. La condizione deve
essere `bool`.

#### Scenario: Filtro sulla forma con chiave

- **WHEN** viene eseguito `? {k:x for each x,k in [1,2,3] with x>2}`
- **THEN** viene stampato `{2:3}`

#### Scenario: Filtro sulla forma parallela

- **WHEN** viene eseguito `? {k:x for each x:k in [1,2,3] : ["a","b","c"] with x=2}`
- **THEN** viene stampato `{b:2}`

#### Scenario: Filtro mai vero

- **WHEN** viene eseguito `? {k:x for each x,k in [1,2,3] with x>5}`
- **THEN** viene stampato `{}`

### Requirement: Valutazione delle collezioni e delle espressioni

Ogni collezione SHALL essere valutata una sola volta, prima del primo elemento (nella forma parallela
prima `A` e poi `B`); `ke`, `ve` e la condizione SHALL essere valutate una volta per elemento,
nell'ordine di visita, prima `ke` e poi `ve`.

#### Scenario: Collezione restituita da una funzione

- **WHEN** la collezione è la chiamata a una func che stampa `chiamata` e restituisce `[1,2]`
- **THEN** `chiamata` viene stampato una volta sola e il dizionario prodotto ha due coppie

#### Scenario: Ordine fra chiave e valore

- **WHEN** viene eseguito `? {f("k"):f("v") for each x,k in [1]}` con `f` una func che stampa il
  proprio argomento e lo restituisce
- **THEN** viene stampato `k`, poi `v`, poi `{k:v}`

### Requirement: Variabili delle dict comprehension

Le due variabili, in entrambe le forme, SHALL seguire le regole dello statement `for each`: devono
essere dichiarate nel contesto corrente (altrimenti errore a runtime che nomina la variabile), la
comprehension non le dichiara e dopo l'espressione SHALL contenere l'ultimo valore assegnato
(invariate con zero elementi). Con `--typecheck` l'assegnazione segue le regole di `STORE`.

#### Scenario: Valore dopo la comprehension

- **WHEN** con `var int x:=0` viene eseguito `d:={k:x for each x,k in [1,2,3]}` e poi `? x`
- **THEN** viene stampato `3`

#### Scenario: Variabile non dichiarata

- **WHEN** viene eseguito `? {z:1 for each y:z in [1] : [2]}` con `y` dichiarata e `z` no
- **THEN** il programma stampa un errore che contiene `z` e termina con exit diverso da 0

### Requirement: Forma a una sola variabile non ammessa

Una dict comprehension con una sola variabile (`{ke:ve for each v in x}`) SHALL restare un errore di
sintassi del compilatore.

#### Scenario: Una variabile

- **WHEN** viene compilato un sorgente con `? {x:x for each x in [1,2]}`
- **THEN** il compilatore segnala `syntax error` e termina con exit diverso da 0

### Requirement: Dict comprehension annidate e su più righe

Una dict comprehension SHALL poter comparire dentro `ke`, `ve`, una collezione o la condizione di
un'altra comprehension (array o dict), e SHALL produrre lo stesso risultato quando è scritta su più
righe dentro le graffe.

#### Scenario: Comprehension nel valore

- **WHEN** con `x`, `y`, `k`, `j` dichiarate `int` viene eseguito
  `? {k:{j:x*y for each y,j in [1,2]} for each x,k in [5]}`
- **THEN** viene stampato un dizionario con la sola chiave `0`, il cui valore ha `0:5` e `1:10`

#### Scenario: A capo prima di for each

- **WHEN** viene eseguito `? {k:x` seguito su una nuova riga da `for each x,k in [4]}`
- **THEN** viene stampato `{0:4}`

### Requirement: Esempi di q.cpl

Le due dict comprehension in fondo a `q.cpl` SHALL compilarsi ed eseguirsi.

#### Scenario: Fine di q.cpl

- **WHEN** viene eseguito `./compiler q.cpl && ./interpreter q`
- **THEN** il programma termina con exit 0 e dopo `dict comprehension` stampa un dizionario con le
  coppie `0:1`, `1:2`, `2:3` e uno con le coppie `alfa:1`, `beta:2`, `gamma:3`

### Requirement: Nessun COMMENT per le dict comprehension

Il p-code di una dict comprehension non SHALL contenere `COMMENT "da fare dict comprehension ..."` né
altri p-code non implementati: l'esecuzione non SHALL terminare con `not impl!`.

#### Scenario: Listato di una dict comprehension

- **WHEN** viene compilato un sorgente con `? {k:x for each x:k in [1] : ["a"] with x>0}` e il
  `.pcd` viene letto con `./asm`
- **THEN** il listato non contiene `COMMENT` e contiene `DICT 0`, `FOREACH_INIT 2`, `FOREACH_NEXT` e
  `COMPR_APPEND 2`
