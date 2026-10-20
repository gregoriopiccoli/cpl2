# Spec Delta

## ADDED Requirements

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
