# interpreter-pcodes Specification

## Purpose

Descrive come l'interprete esegue i singoli p-code implementati: effetti sullo stack di esecuzione, oggetti prodotti e comportamento osservabile. In questo change copre i p-code `PROC_TYPE` e `FUNC_TYPE`; accoglierà gli altri p-code man mano che vengono implementati.

## Requirements

### Requirement: Esecuzione del p-code PROC_TYPE

L'interprete SHALL eseguire il p-code `PROC_TYPE`, il cui operando intero N è il numero di tipi dei parametri, consumando i N tipi in cima allo stack di esecuzione e lasciando al loro posto un unico oggetto tipo che rappresenta il tipo proc con quei N parametri, nell'ordine in cui erano sullo stack.

#### Scenario: Tipo proc con parametri

- **WHEN** viene eseguito `PROC_TYPE` con operando 2 quando lo stack di esecuzione contiene i tipi `int` e `str`
- **THEN** lo stack di esecuzione contiene un solo oggetto tipo proc con parametri `(int, str)`

#### Scenario: Tipo proc senza parametri

- **WHEN** viene eseguito `PROC_TYPE` con operando 0
- **THEN** lo stack di esecuzione contiene un solo oggetto tipo proc senza parametri

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `PROC_TYPE` con operando N su uno stack di esecuzione con N tipi in cima
- **THEN** la profondità dello stack diminuisce di N-1 e il tipo proc risiede in cima

### Requirement: Rappresentazione stampabile del tipo proc

L'oggetto tipo prodotto da `PROC_TYPE` SHALL avere una rappresentazione testuale che elenca i tipi dei parametri separati da virgola, nel formato `(t1, t2, ...)`, e `()` quando il tipo non ha parametri.

#### Scenario: Stampa di un tipo proc con parametri

- **WHEN** viene stampato un tipo proc con parametri `int` e `str`
- **THEN** la rappresentazione testuale è `(int, str)`

#### Scenario: Stampa di un tipo proc senza parametri

- **WHEN** viene stampato un tipo proc senza parametri
- **THEN** la rappresentazione testuale è `()`

### Requirement: Sicurezza GC del tipo proc

L'oggetto tipo prodotto da `PROC_TYPE` SHALL rendere raggiungibili per il garbage collector tutti i tipi dei parametri che lo compongono, sia durante la marcatura sia durante l'espansione delle generazioni.

#### Scenario: Marcatura dei tipi dei parametri

- **WHEN** il garbage collector marca un tipo proc marcato come raggiungibile
- **THEN** anche tutti i tipi dei suoi parametri risultano marcati o già raggiungibili

### Requirement: PROC_TYPE non è più un p-code non implementato

L'interprete SHALL riconoscere `PROC_TYPE` nella fabbrica dei p-code: l'esecuzione di un `.pcd` che contiene `PROC_TYPE` non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma cpl2 che dichiara un tipo proc

- **WHEN** viene eseguito un `.pcd` compilato da un sorgente che dichiara una variabile di tipo proc
- **THEN** il programma si esegue senza l'errore `pcode not implemented`

### Requirement: Esecuzione del p-code FUNC_TYPE

L'interprete SHALL eseguire il p-code `FUNC_TYPE`, il cui operando intero N è il numero di tipi dei parametri. Lo stack di esecuzione contiene il tipo di ritorno seguito dai N tipi dei parametri; il p-code li consuma tutti e lascia in cima un unico oggetto tipo che rappresenta il tipo func, con il tipo di ritorno e i parametri nell'ordine in cui erano sullo stack.

#### Scenario: Tipo func con parametri

- **WHEN** viene eseguito `FUNC_TYPE` con operando 2 quando lo stack contiene i tipi `int` (ritorno), `int` e `str` (parametri)
- **THEN** lo stack contiene un solo oggetto tipo func con tipo di ritorno `int` e parametri `(int, str)`

#### Scenario: Tipo func senza parametri

- **WHEN** viene eseguito `FUNC_TYPE` con operando 0 quando lo stack contiene il tipo di ritorno `str`
- **THEN** lo stack contiene un solo oggetto tipo func con tipo di ritorno `str` e nessun parametro

#### Scenario: Effetto sullo stack del FUNC_TYPE

- **WHEN** viene eseguito `FUNC_TYPE` con operando N su uno stack con N+1 tipi in cima (tipo di ritorno più i parametri)
- **THEN** la profondità dello stack diminuisce di N e il tipo func risiede in cima

### Requirement: Rappresentazione stampabile del tipo func

L'oggetto tipo prodotto da `FUNC_TYPE` SHALL avere una rappresentazione testuale nel formato `(ritorno:p1, p2, ...)`, con i tipi dei parametri separati da virgola, e `(ritorno:)` quando il tipo non ha parametri.

#### Scenario: Stampa di un tipo func con parametri

- **WHEN** viene stampato un tipo func con ritorno `int` e parametri `str`, `int`
- **THEN** la rappresentazione testuale è `(int:str, int)`

#### Scenario: Stampa di un tipo func senza parametri

- **WHEN** viene stampato un tipo func con ritorno `str` e nessun parametro
- **THEN** la rappresentazione testuale è `(str:)`

### Requirement: Sicurezza GC del tipo func

L'oggetto tipo prodotto da `FUNC_TYPE` SHALL rendere raggiungibili per il garbage collector sia il tipo di ritorno sia tutti i tipi dei parametri, sia durante la marcatura sia durante l'espansione delle generazioni.

#### Scenario: Marcatura di ritorno e parametri

- **WHEN** il garbage collector marca un tipo func marcato come raggiungibile
- **THEN** anche il tipo di ritorno e tutti i tipi dei parametri risultano marcati o già raggiungibili

### Requirement: FUNC_TYPE non è più un p-code non implementato

L'interprete SHALL riconoscere `FUNC_TYPE` nella fabbrica dei p-code: l'esecuzione di un `.pcd` che contiene `FUNC_TYPE` non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma cpl2 che dichiara un tipo func

- **WHEN** viene eseguito un `.pcd` compilato da un sorgente che dichiara una variabile di tipo func
- **THEN** il programma si esegue senza l'errore `pcode not implemented`

### Requirement: Esecuzione del p-code BOOL_TYPE

L'interprete SHALL eseguire il p-code `BOOL_TYPE`, p-code senza operando, spingendo in cima allo stack di esecuzione l'oggetto tipo che rappresenta il tipo bool.

#### Scenario: Tipo bool

- **WHEN** viene eseguito `BOOL_TYPE`
- **THEN** lo stack di esecuzione contiene in cima l'oggetto tipo bool

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `BOOL_TYPE` con lo stack di esecuzione a una certa profondità
- **THEN** la profondità dello stack aumenta di 1 e il tipo bool risiede in cima

### Requirement: Rappresentazione stampabile del tipo bool

L'oggetto tipo prodotto da `BOOL_TYPE` SHALL avere la rappresentazione testuale `bool`.

#### Scenario: Stampa del tipo bool

- **WHEN** viene stampato l'oggetto tipo bool
- **THEN** la rappresentazione testuale è `bool`

### Requirement: Sicurezza GC del tipo bool

L'oggetto tipo bool SHALL sopravvivere alle raccolte del garbage collector mentre è referenziato (per esempio sullo stack di esecuzione) e SHALL essere rilasciato alla fine dell'esecuzione insieme agli altri oggetti di sistema.

#### Scenario: Raccolta con il tipo bool sullo stack

- **WHEN** il garbage collector esegue una raccolta mentre un oggetto tipo bool è sullo stack di esecuzione
- **THEN** l'oggetto tipo bool sopravvive e resta utilizzabile

#### Scenario: Rilascio alla fine dell'esecuzione

- **WHEN** l'interprete rilascia gli oggetti di sistema a fine esecuzione
- **THEN** anche l'oggetto tipo bool viene rilasciato

### Requirement: BOOL_TYPE non è più un p-code non implementato

L'interprete SHALL riconoscere `BOOL_TYPE` nella fabbrica dei p-code: l'esecuzione di un `.pcd` che contiene `BOOL_TYPE` non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma cpl2 che dichiara un tipo bool

- **WHEN** viene eseguito un `.pcd` compilato da un sorgente che dichiara una variabile di tipo bool
- **THEN** il programma si esegue senza l'errore `pcode not implemented`

### Requirement: Esecuzione del p-code OR_TYPE

L'interprete SHALL eseguire il p-code `OR_TYPE`, il cui operando intero N (sempre almeno 2, come garantito dalla grammatica) è il numero di alternative del tipo, consumando i N tipi in cima allo stack di esecuzione e lasciando al loro posto un unico oggetto tipo che rappresenta il tipo or con quelle N alternative, nell'ordine in cui erano sullo stack.

#### Scenario: Tipo or con due alternative

- **WHEN** viene eseguito `OR_TYPE` con operando 2 quando lo stack di esecuzione contiene i tipi `int` e `str`
- **THEN** lo stack di esecuzione contiene un solo oggetto tipo or con alternative `int` e `str`

#### Scenario: Tipo or con tre alternative

- **WHEN** viene eseguito `OR_TYPE` con operando 3 quando lo stack di esecuzione contiene i tipi `int`, `str` e `bool`
- **THEN** lo stack di esecuzione contiene un solo oggetto tipo or con alternative `int`, `str` e `bool`

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `OR_TYPE` con operando N su uno stack di esecuzione con N tipi in cima
- **THEN** la profondità dello stack diminuisce di N-1 e il tipo or risiede in cima

### Requirement: Rappresentazione stampabile del tipo or

L'oggetto tipo prodotto da `OR_TYPE` SHALL avere una rappresentazione testuale che elenca le alternative separate dal carattere `|`, nel formato `t1|t2|...`.

#### Scenario: Stampa di un tipo or

- **WHEN** viene stampato un tipo or con alternative `int` e `str`
- **THEN** la rappresentazione testuale è `int|str`

### Requirement: Sicurezza GC del tipo or

L'oggetto tipo prodotto da `OR_TYPE` SHALL rendere raggiungibili per il garbage collector tutti i tipi delle alternative che lo compongono, sia durante la marcatura sia durante l'espansione delle generazioni.

#### Scenario: Marcatura delle alternative

- **WHEN** il garbage collector marca un tipo or marcato come raggiungibile
- **THEN** anche tutti i tipi delle sue alternative risultano marcati o già raggiungibili

### Requirement: OR_TYPE non è più un p-code non implementato

L'interprete SHALL riconoscere `OR_TYPE` nella fabbrica dei p-code: l'esecuzione di un `.pcd` che contiene `OR_TYPE` non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma cpl2 che dichiara un tipo or

- **WHEN** viene eseguito un `.pcd` compilato da un sorgente che dichiara una variabile di tipo alternativo (per esempio `var int|str o` in `q.cpl`)
- **THEN** il programma si esegue senza l'errore `pcode not implemented` e arriva alla fine con exit 0

### Requirement: Esecuzione del p-code TUPLE

L'interprete SHALL eseguire il p-code `TUPLE`, il cui operando intero N (almeno 1, come garantito dalla grammatica) è il numero di elementi, consumando i N valori in cima allo stack di esecuzione e lasciando al loro posto un unico oggetto valore tuple contenente quegli N elementi, nell'ordine in cui erano sullo stack.

#### Scenario: Costante tuple con tre elementi

- **WHEN** viene eseguito `TUPLE` con operando 3 quando lo stack di esecuzione contiene `10`, `"alfa"` e `true`
- **THEN** lo stack di esecuzione contiene un solo oggetto valore tuple con elementi `10`, `alfa`, `true` in quell'ordine

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `TUPLE` con operando N su uno stack di esecuzione con N valori in cima
- **THEN** la profondità dello stack diminuisce di N-1 e l'oggetto valore tuple risiede in cima

#### Scenario: Un solo elemento

- **WHEN** viene eseguito `TUPLE` con operando 1
- **THEN** lo stack contiene un oggetto valore tuple con quell'unico elemento

### Requirement: Rappresentazione stampabile del valore tuple

L'oggetto valore tuple SHALL avere una rappresentazione testuale nel formato `|v1,v2,...|`: elementi separati da virgola, senza spazi, racchiusi dalle barre verticali usate nella sintassi della costante.

#### Scenario: Stampa di un valore tuple

- **WHEN** viene stampato un valore tuple con elementi `10`, `beta`, `true`
- **THEN** la rappresentazione testuale è `|10,beta,true|`

#### Scenario: Stampa dopo la lettura di un elemento

- **WHEN** un programma esegue `? t` su una variabile tuple assegnata con `|10,"alfa",true|` e nessuna modifica successiva
- **THEN** viene stampato `|10,alfa,true|`

### Requirement: Sicurezza GC del valore tuple

L'oggetto valore tuple SHALL rendere raggiungibili per il garbage collector tutti gli elementi che lo compongono, sia durante la marcatura sia durante l'espansione delle generazioni.

#### Scenario: Marcatura degli elementi

- **WHEN** il garbage collector marca un valore tuple raggiungibile
- **THEN** anche tutti i suoi elementi risultano marcati o già raggiungibili

#### Scenario: Espansione delle generazioni

- **WHEN** un valore tuple raggiungibile viene espanso a una nuova generazione
- **THEN** anche i suoi elementi vengono espansi a quella generazione se non lo erano già

### Requirement: Esecuzione del p-code STORESLICE

L'interprete SHALL eseguire il p-code `STORESLICE`, che riceve sullo stack di esecuzione il contenitore, l'indice e il valore (in quest'ordine, con il valore in cima): SHALL assegnare il valore all'elemento indicato del contenitore tramite l'interfaccia di scrittura per slice e SHALL lasciare il valore assegnato in cima allo stack, consumando contenitore e indice.

#### Scenario: Assegnamento in un valore tuple

- **WHEN** viene eseguito `STORESLICE` quando lo stack contiene un valore tuple con almeno due elementi, l'indice `1` e il valore `"beta"`
- **THEN** l'elemento in posizione 1 del tuple contiene `"beta"` e in cima allo stack resta `"beta"`

#### Scenario: Assegnamento in un array o in un dizionario

- **WHEN** viene eseguito `STORESLICE` con un array e un indice intero, oppure con un dizionario e una chiave
- **THEN** l'elemento o la chiave indicata assume il valore assegnato

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `STORESLICE` su uno stack con contenitore, indice e valore in cima
- **THEN** la profondità dello stack diminuisce di 2 e il valore assegnato risiede in cima

### Requirement: TUPLE non è più un p-code non implementato

L'interprete SHALL riconoscere `TUPLE` nella fabbrica dei p-code: l'esecuzione di un `.pcd` che contiene `TUPLE` non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma cpl2 con una costante tuple

- **WHEN** viene eseguito `q.pcd`, compilato da un sorgente che assegna una costante `|...|` a una variabile tuple
- **THEN** il programma si esegue senza l'errore `pcode not implemented` e arriva alla fine con exit 0

### Requirement: STORESLICE non è più un p-code non implementato

L'interprete SHALL riconoscere `STORESLICE` nella fabbrica dei p-code: l'esecuzione di un `.pcd` che contiene `STORESLICE` non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma cpl2 con l'assegnamento di un elemento

- **WHEN** viene eseguito un `.pcd` compilato da un sorgente che contiene `a[1]:=9` su un array, una riga equivalente su un dizionario o `t[1]:="beta"` su un tuple
- **THEN** il programma si esegue senza l'errore `pcode not implemented` e l'elemento assume il valore assegnato

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

### Requirement: Esecuzione del p-code IMPORTID

L'interprete SHALL eseguire il p-code `IMPORTID`, il cui operando stringa è un identificatore
di modulo: SHALL spingere in cima allo stack di esecuzione un oggetto stringa con esattamente
quel valore, lasciando invariato il resto dello stack.

#### Scenario: Singolo identificatore

- **WHEN** viene eseguito `IMPORTID a` su uno stack vuoto
- **THEN** lo stack contiene una sola stringa e il suo valore è `a`

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `IMPORTID a` con lo stack di esecuzione a una certa profondità
- **THEN** la profondità aumenta di 1 e la nuova stringa `a` è in cima

### Requirement: Esecuzione del p-code IMPORTIDX

L'interprete SHALL eseguire il p-code `IMPORTIDX`, il cui operando stringa è un identificatore:
SHALL spingere quell'identificatore come stringa e poi SHALL sostituire le due stringhe in cima
allo stack con una sola stringa ottenuta concatenando la precedente, un punto e la nuova, nell'ordine
`precedente + "." + identificatore`.

#### Scenario: Due componenti

- **WHEN** viene eseguito `IMPORTID a` seguito da `IMPORTIDX b`
- **THEN** lo stack contiene un'unica stringa `a.b`

#### Scenario: Tre componenti

- **WHEN** viene eseguito `IMPORTID a`, poi `IMPORTIDX b`, poi `IMPORTIDX c`
- **THEN** lo stack contiene un'unica stringa `a.b.c`

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `IMPORTIDX` quando lo stack ha già la stringa del componente precedente
- **THEN** la profondità dello stack resta invariata e la cima è la stringa concatenata

### Requirement: Stampa e rimozione delle stringhe da parte di IMPORT

L'interprete SHALL eseguire `IMPORT` usando l'intero N del suo operando come numero di nomi
da consumare: SHALL stampare sullo standard output il valore delle N stringhe in cima allo stack, una
per riga e nell'ordine in cui compaiono sullo stack (dal basso verso la cima), quindi SHALL rimuoverle
lasciando intatto ciò che sta sotto di esse. SHALL verificare che le N posizioni in cima contengano
davvero stringhe e, in caso contrario, SHALL sollevare `domain_error` senza stampare nulla.

#### Scenario: Un solo modulo

- **WHEN** viene eseguito `IMPORT 1` quando in cima allo stack c'è una sola stringa `a`
- **THEN** viene stampato `a` e la stringa viene rimossa

#### Scenario: Più moduli

- **WHEN** viene eseguito `IMPORT 2` quando in cima allo stack ci sono due stringhe consecutive,
  `a.b` sotto `c.d`
- **THEN** vengono stampati `a.b` e poi `c.d`, ed entrambe le stringhe vengono rimosse

#### Scenario: Operando minore delle stringhe presenti

- **WHEN** viene eseguito `IMPORT 1` quando in cima allo stack ci sono due stringhe consecutive,
  `a` sotto `b`
- **THEN** viene stampato solo `b` e viene rimossa solo quella stringa, lasciando `a` sullo stack

#### Scenario: Stack pulito per il comando successivo

- **WHEN** due comandi `import` vengono eseguiti uno dopo l'altro, ciascuno con un solo modulo
- **THEN** ciascun `IMPORT` stampa il rispettivo nome, perché il primo ha rimosso la propria stringa

#### Scenario: Meno stringhe dell'operando

- **WHEN** viene eseguito `IMPORT 2` quando in cima allo stack c'è una sola stringa
- **THEN** viene sollevato `domain_error` senza stampare nulla

### Requirement: IMPORT non include ancora i moduli

L'interprete SHALL limitare `IMPORT` alla stampa e alla rimozione delle stringhe: NON SHALL caricare,
risolvere o valutare il modulo indicato, NON SHALL modificare il contesto di esecuzione e NON SHALL
conservare il nome del modulo dopo la rimozione dallo stack.

#### Scenario: Nessuna inclusione

- **WHEN** viene eseguito `IMPORTID a` seguito da `IMPORT 1`
- **THEN** viene stampato `a`, nessun modulo viene caricato e lo stack torna alla profondità che
  aveva prima di `IMPORTID`

### Requirement: IMPORTID, IMPORTIDX e IMPORT non sono più p-code non implementati

L'interprete SHALL riconoscere `IMPORTID`, `IMPORTIDX` e `IMPORT` nella fabbrica dei p-code:
l'esecuzione di un `.pcd` che contiene un comando `import` senza `from` non SHALL terminare con
l'errore `pcode not implemented`.

#### Scenario: Programma cpl2 con import semplice

- **WHEN** viene eseguito il p-code compilato da un sorgente che contiene `import a`
- **THEN** il programma si esegue senza l'errore `pcode not implemented` e termina con codice 0

#### Scenario: Programma cpl2 con nome puntato

- **WHEN** viene eseguito il p-code compilato da un sorgente che contiene `import a.b.c`
- **THEN** il programma si esegue senza l'errore `pcode not implemented` e stampa `a.b.c`

### Requirement: Esecuzione del p-code CASE

L'interprete SHALL eseguire il p-code `CASE`, il cui operando intero è una label, togliendo dalla
cima dello stack la costante del `case`, confrontandola con il selettore che sta subito sotto e
saltando alla label quando sono uguali. Il selettore SHALL restare in cima allo stack sia quando
il salto avviene sia quando non avviene.

#### Scenario: Costante uguale al selettore

- **WHEN** viene eseguito `CASE 5` con lo stack che contiene il selettore `2` e sopra la costante `2`
- **THEN** l'esecuzione prosegue dalla posizione della `LABEL 5` e in cima allo stack c'è il
  selettore `2`

#### Scenario: Costante diversa dal selettore

- **WHEN** viene eseguito `CASE 5` con lo stack che contiene il selettore `2` e sopra la costante `3`
- **THEN** l'esecuzione prosegue con il p-code successivo e in cima allo stack c'è il selettore `2`

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `CASE` su uno stack con selettore e costante in cima
- **THEN** la profondità dello stack diminuisce di 1, sia con salto sia senza

### Requirement: Confronto di CASE fra tipi diversi

Quando la costante e il selettore hanno tipi diversi, oppure il selettore è `nil`, `CASE` SHALL
considerarli non uguali e proseguire senza salto e senza sollevare errori. Per valori dello stesso
tipo (`int`, `str`, `bool`) SHALL usare la stessa uguaglianza dell'operatore `=`.

#### Scenario: Selettore int, costante stringa

- **WHEN** viene eseguito `CASE` con selettore `1` e costante `"1"`
- **THEN** non c'è salto e non viene sollevato alcun errore

#### Scenario: Selettore nil

- **WHEN** viene eseguito `CASE` con selettore `nil` e costante `0`
- **THEN** non c'è salto e non viene sollevato alcun errore

#### Scenario: Stringhe uguali

- **WHEN** viene eseguito `CASE` con selettore `"b"` e costante `"b"`
- **THEN** l'esecuzione salta alla label dell'operando

### Requirement: CASE non è un p-code non implementato

L'interprete SHALL riconoscere `CASE` nella fabbrica dei p-code: l'esecuzione di un `.pcd` che
contiene `CASE` non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma cpl2 con switch

- **WHEN** viene eseguito un `.pcd` compilato da un sorgente che contiene uno `switch`
- **THEN** il programma si esegue senza l'errore `pcode not implemented`

### Requirement: Rappresentazione leggibile di CASE

Il p-code `CASE` SHALL avere il nome testuale `CASE`, così che `./asm` lo mostri come
`CASE <label>` e la conversione inversa del listato lo riconosca.

#### Scenario: Listato di asm

- **WHEN** `./asm` legge un `.pcd` che contiene il p-code `CASE` con operando 3
- **THEN** il listato contiene una riga `CASE 3`

### Requirement: Esecuzione del p-code UMINUS sugli interi

L'interprete SHALL eseguire il p-code `UMINUS`, che non ha operando, sostituendo il valore intero in
cima allo stack di esecuzione con un intero di valore opposto. La profondità dello stack SHALL
restare invariata.

#### Scenario: Negazione di una costante positiva

- **WHEN** viene eseguito il programma `? -5`
- **THEN** l'output è `-5`

#### Scenario: Negazione di un valore negativo

- **WHEN** viene eseguito il programma `? - -7`
- **THEN** l'output è `7`

#### Scenario: Negazione dello zero

- **WHEN** viene eseguito il programma `? -0`
- **THEN** l'output è `0`

#### Scenario: Negazione di un'espressione tra parentesi

- **WHEN** viene eseguito il programma `? -(2+3)`
- **THEN** l'output è `-5`

#### Scenario: Meno unario come operando di un'operazione binaria

- **WHEN** viene eseguito il programma `? 3 - -2`
- **THEN** l'output è `5`

#### Scenario: Precedenza rispetto alla moltiplicazione

- **WHEN** viene eseguito il programma `? -2*3`
- **THEN** l'output è `-6`

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `UMINUS` con un intero in cima allo stack
- **THEN** la profondità dello stack non cambia e in cima c'è l'intero negato

### Requirement: UMINUS non modifica l'operando

Il p-code `UMINUS` SHALL produrre un valore nuovo e SHALL lasciare invariato l'intero da cui parte,
sia quando proviene da una variabile sia quando proviene da una costante del programma eseguita
più volte.

#### Scenario: Variabile non modificata

- **WHEN** viene eseguito il programma `var int i:=4` seguito da `? -i` e poi da `? i`
- **THEN** l'output è `-4` seguito da `4`

#### Scenario: Costante rieseguita in un ciclo

- **WHEN** un ciclo esegue tre volte `? -5`
- **THEN** l'output è `-5` per tutte e tre le iterazioni

### Requirement: UMINUS su valori non interi

L'esecuzione di `UMINUS` su un valore che non è né un intero né un float (stringa, bool, nil,
array, dict, tuple, tipo, procedura, modulo) SHALL interrompere il programma con un errore di
runtime, senza produrre un valore.

#### Scenario: Meno unario su una stringa

- **WHEN** viene eseguito il programma `? -'a'`
- **THEN** l'esecuzione termina con l'errore `uminus not implemented` e non stampa alcun valore
  per quella riga

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

### Requirement: Esecuzione del p-code IS

L'interprete SHALL eseguire il p-code `IS`, p-code senza operando. Lo stack di esecuzione contiene
in cima un oggetto tipo e sotto di esso un valore; il p-code li consuma entrambi e lascia al loro
posto un unico booleano, `true` se il valore appartiene al tipo secondo le regole della capability
`is-operator`, `false` altrimenti.

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `IS` con un valore e un tipo in cima allo stack di esecuzione
- **THEN** la profondità dello stack diminuisce di 1 e in cima c'è un booleano

#### Scenario: Valore conforme

- **WHEN** viene eseguita la sequenza `INT_CONST 1`, `INT_TYPE`, `IS`
- **THEN** in cima allo stack c'è `true`

#### Scenario: Operando di tipo non riconosciuto

- **WHEN** viene eseguita la sequenza `INT_CONST 1`, `INT_CONST 3`, `IS`
- **THEN** l'esecuzione termina con un errore a runtime

### Requirement: IS non è più un p-code non implementato

L'interprete SHALL riconoscere `IS` nella fabbrica dei p-code: l'esecuzione di un `.pcd` che
contiene `IS` non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma con is

- **WHEN** viene eseguito il `.pcd` compilato da `? (1 is int)`
- **THEN** l'output è `true` e non compare `not impl!`

### Requirement: Esecuzione del p-code FLOAT_TYPE

L'interprete SHALL eseguire il p-code `FLOAT_TYPE`, p-code senza operando, spingendo in cima allo
stack di esecuzione l'oggetto tipo che rappresenta il tipo float, la cui rappresentazione testuale
è `float`.

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `FLOAT_TYPE` con lo stack di esecuzione a una certa profondità
- **THEN** la profondità dello stack aumenta di 1 e il tipo float risiede in cima

#### Scenario: Dichiarazione di una variabile float

- **WHEN** viene eseguito `var float x` seguito da `? (x is float)`
- **THEN** la dichiarazione non termina con `not impl!` e l'output è `false` (`x` vale `nil`)

### Requirement: FLOAT_TYPE non è più un p-code non implementato

L'interprete SHALL riconoscere `FLOAT_TYPE` nella fabbrica dei p-code: l'esecuzione di un `.pcd` che
contiene `FLOAT_TYPE` non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma con il tipo float

- **WHEN** viene eseguito il `.pcd` compilato da `? (1 is float)`
- **THEN** l'output è `false` e non compare `not impl!`

### Requirement: Esecuzione del p-code FLOAT_CONST

L'interprete SHALL eseguire il p-code `FLOAT_CONST`, il cui operando è il testo della costante
così come lo scrive il compilatore, spingendo in cima allo stack di esecuzione un valore float con
quel valore numerico (precisione doppia). La profondità dello stack SHALL aumentare di 1.

#### Scenario: Stampa di una costante float

- **WHEN** viene eseguito il programma `? 3.14`
- **THEN** l'output è `3.14`

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `FLOAT_CONST 2.5` con lo stack di esecuzione a una certa profondità
- **THEN** la profondità dello stack aumenta di 1 e in cima c'è il float `2.5`

#### Scenario: Costante con separatori di cifre

- **WHEN** viene eseguito il programma `? 1_000.25`
- **THEN** l'output è `1000.25`

#### Scenario: Il valore è un float

- **WHEN** viene eseguito il programma `? (3.14 is float)` seguito da `? (3.14 is int)`
- **THEN** l'output è `true` seguito da `false`

#### Scenario: Assegnazione a una variabile float con il controllo di tipo attivo

- **WHEN** viene eseguito con `--typecheck` il programma `var float x := 0.5` seguito da `? x`
- **THEN** l'assegnazione è accettata e l'output è `0.5`

### Requirement: Rappresentazione stampabile del valore float

La rappresentazione testuale di un valore float SHALL essere la forma decimale più corta che,
riletta, dà esattamente lo stesso valore, e SHALL sempre distinguersi da un intero: quando quella
forma non contiene né il punto decimale né un esponente, SHALL esserle aggiunto `.0`.

#### Scenario: Nessuno zero finale superfluo

- **WHEN** viene eseguito il programma `? 3.140`
- **THEN** l'output è `3.14`

#### Scenario: Float con valore intero

- **WHEN** viene eseguito il programma `? 2.0` seguito da `? 7.`
- **THEN** l'output è `2.0` seguito da `7.0`

#### Scenario: Valore non rappresentabile esattamente

- **WHEN** viene eseguito il programma `? 0.1`
- **THEN** l'output è `0.1`

### Requirement: FLOAT_CONST non è più un p-code non implementato

L'interprete SHALL riconoscere `FLOAT_CONST` nella fabbrica dei p-code: l'esecuzione di un `.pcd`
che contiene `FLOAT_CONST` non SHALL terminare con l'errore `not impl!`.

#### Scenario: Programma con una costante float

- **WHEN** viene eseguito il `.pcd` compilato da `? 1.5`
- **THEN** l'esecuzione termina con exit 0 e l'output è `1.5`

### Requirement: La costante float non viene modificata dall'esecuzione

Il valore prodotto da `FLOAT_CONST` SHALL restare lo stesso a ogni esecuzione dello stesso p-code,
anche quando il valore è stato usato come operando di un'operazione o assegnato a una variabile.

#### Scenario: Costante rieseguita in un ciclo

- **WHEN** un ciclo esegue tre volte `? -1.5` seguito da `? 1.5`
- **THEN** l'output è `-1.5` seguito da `1.5` per tutte e tre le iterazioni

### Requirement: Esecuzione del p-code UMINUS sui float

L'interprete SHALL eseguire `UMINUS` anche quando in cima allo stack c'è un float, sostituendolo
con un nuovo float di valore opposto e lasciando invariato il float da cui parte. La profondità
dello stack SHALL restare invariata.

#### Scenario: Negazione di una costante float

- **WHEN** viene eseguito il programma `? -2.5`
- **THEN** l'output è `-2.5`

#### Scenario: Doppia negazione

- **WHEN** viene eseguito il programma `? - -0.75`
- **THEN** l'output è `0.75`

#### Scenario: Variabile float non modificata

- **WHEN** viene eseguito il programma `var float f:=4.5` seguito da `? -f` e poi da `? f`
- **THEN** l'output è `-4.5` seguito da `4.5`

#### Scenario: Il risultato è ancora un float

- **WHEN** viene eseguito il programma `? (-2.5 is float)`
- **THEN** l'output è `true`
