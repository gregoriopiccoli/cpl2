# Spec Delta

## Purpose

Definisce la semantica dello statement `for each v[,k] in <collezione> [with <condizione>] ... end`
di cpl2 su array, dizionari e tuple: ordine di visita, variabili di valore e di chiave locali,
filtro `with` e comportamento del programma compilato che lo contiene.

## ADDED Requirements

### Requirement: Ciclo sugli elementi di un array

Un `for each v in a` con `a` array SHALL eseguire il corpo una volta per ogni elemento, in ordine
di indice crescente, con `v` che contiene l'elemento corrente.

#### Scenario: Array di interi

- **WHEN** viene eseguito `for each i in [1,2,3]` con un corpo che stampa `i`
- **THEN** vengono stampati `1`, `2`, `3`, uno per riga

#### Scenario: Array vuoto

- **WHEN** viene eseguito `for each i in []` con un corpo che stampa `i`, seguito da `? 'fine'`
- **THEN** viene stampato solo `fine`

### Requirement: Indice dell'array nella seconda variabile

Un `for each v,k in a` con `a` array SHALL assegnare a `k`, a ogni iterazione, l'indice `int`
dell'elemento corrente, partendo da 0, lo stesso indice con cui `a[k]` restituisce `v`.

#### Scenario: Array con indice

- **WHEN** con `a:=[1,2,3]` viene eseguito `for each i,k in a` con un corpo che stampa
  `'a[',k,']=',i`
- **THEN** vengono stampati `a[0]=1`, `a[1]=2`, `a[2]=3`

### Requirement: Ciclo sui valori di un dizionario

Un `for each v in d` con `d` dizionario SHALL eseguire il corpo una volta per ogni coppia, con `v`
che contiene il valore. Ogni coppia SHALL essere visitata esattamente una volta; l'ordine di
visita non è specificato.

#### Scenario: Dizionario senza chiave

- **WHEN** con `d:={"alfa":100,"beta":200}` viene eseguito `for each i in d` con un corpo che
  stampa `i`
- **THEN** vengono stampati `100` e `200`, ciascuno una volta, in un ordine qualsiasi

#### Scenario: Dizionario vuoto

- **WHEN** viene eseguito `for each i in {}` con un corpo che stampa `i`, seguito da `? 'fine'`
- **THEN** viene stampato solo `fine`

### Requirement: Chiave del dizionario nella seconda variabile

Un `for each v,k in d` con `d` dizionario SHALL assegnare a `k` la chiave della coppia corrente
come valore `str`, cioè il testo con cui la chiave è memorizzata nel dizionario.

#### Scenario: Dizionario con chiave

- **WHEN** con `d:={"alfa":100}` viene eseguito `for each i,s in d` con un corpo che stampa
  `'d["',s,'"]=',i`
- **THEN** viene stampato `d["alfa"]=100`

#### Scenario: Chiave non stringa

- **WHEN** con `d:={1:'uno'}` viene eseguito `for each v,k in d` con `k` dichiarata `str`
- **THEN** `k` vale la stringa `1`

### Requirement: Chiavi del dizionario fissate all'inizio del ciclo

Le chiavi visitate SHALL essere quelle presenti nel dizionario quando il ciclo comincia: una chiave
aggiunta nel corpo non SHALL essere visitata. Il valore assegnato alla variabile SHALL essere
quello che la chiave ha nel momento in cui viene visitata.

#### Scenario: Chiave aggiunta nel corpo

- **WHEN** con `d:={"a":1}` viene eseguito `for each v,k in d` con un corpo che stampa `k` ed
  esegue `d["b"]:=2`
- **THEN** viene stampato solo `a`

#### Scenario: Valore cambiato prima della visita

- **WHEN** con `d:={"a":1,"b":2}` viene eseguito `for each v,k in d` con un corpo che somma `v` a
  un totale ed esegue `d["a"]:=10` e `d["b"]:=20`
- **THEN** il totale finale vale `1+20` oppure `2+10`, secondo l'ordine di visita, cioè `21` o `12`

### Requirement: Ciclo sugli elementi di una tupla

Un `for each v in t` con `t` tupla SHALL eseguire il corpo una volta per ogni elemento, in ordine
di posizione, con `v` che contiene l'elemento corrente.

#### Scenario: Tupla eterogenea

- **WHEN** con `t:=|1,2,"t"|` e `var any v` viene eseguito `for each v in t` con un corpo che
  stampa `v`
- **THEN** vengono stampati `1`, `2`, `t`

### Requirement: Le tuple non hanno chiave

La forma a due variabili `for each v,k in t` con `t` tupla SHALL interrompere il programma con un
errore a runtime prima della prima iterazione.

#### Scenario: Tupla con chiave

- **WHEN** viene eseguito `for each v,k in |1,2|`
- **THEN** il corpo non viene eseguito e il programma si interrompe con un messaggio d'errore e
  codice di uscita diverso da 0

### Requirement: Solo collezioni iterabili

Una collezione che non è un array, un dizionario o una tupla (per esempio `int`, `str`, `bool`,
`nil`) SHALL interrompere il programma con un errore a runtime prima della prima iterazione.

#### Scenario: Ciclo su un intero

- **WHEN** viene eseguito `for each i in 5`
- **THEN** il corpo non viene eseguito e il programma si interrompe con un messaggio d'errore

#### Scenario: Ciclo su una stringa

- **WHEN** viene eseguito `for each c in 'abc'`
- **THEN** il corpo non viene eseguito e il programma si interrompe con un messaggio d'errore

### Requirement: Variabili del for each locali

La variabile del valore e, se presente, quella della chiave SHALL essere già dichiarate nel
contesto corrente (il modulo per il codice di livello esterno, la funzione o procedura per il suo
corpo). Un nome non dichiarato nel contesto corrente, anche se visibile da un livello superiore,
SHALL interrompere il programma con un errore che lo nomina, prima della prima iterazione.

#### Scenario: Variabili dichiarate nel modulo

- **WHEN** a livello esterno vengono dichiarate `var int i,k` e poi eseguito
  `for each i,k in [5,6]`
- **THEN** il ciclo viene eseguito normalmente

#### Scenario: Variabile del valore non dichiarata

- **WHEN** viene eseguito `for each j in [1,2]` senza che `j` sia mai stato dichiarato
- **THEN** il corpo non viene eseguito e il programma si interrompe con un messaggio che contiene
  `j`

#### Scenario: Chiave visibile solo dal livello superiore

- **WHEN** il modulo dichiara `var int k` e una procedura che dichiara solo `var int v` esegue
  `for each v,k in [1]`
- **THEN** il corpo non viene eseguito e il programma si interrompe con un messaggio che contiene
  `k`

### Requirement: Assegnazione con il controllo di tipo

Valore e chiave SHALL essere assegnati alle variabili con le stesse regole di un'assegnazione
`:=`, compreso il controllo del tipo dichiarato quando è attivo `--typecheck`.

#### Scenario: Tipo incompatibile con --typecheck

- **WHEN** con `--typecheck` e `var int i` viene eseguito `for each i in ['a']`
- **THEN** il programma si interrompe con l'errore di tipo che nomina `i`

### Requirement: Filtro with

Con `with <condizione>` la condizione SHALL essere valutata a ogni elemento dopo l'assegnazione di
valore e chiave; se vale `false` il corpo SHALL essere saltato per quell'elemento e il ciclo
prosegue col successivo. Una condizione non `bool` SHALL interrompere il programma con un errore.

#### Scenario: Array con with

- **WHEN** viene eseguito `for each i in [1,2,3] with i>1` con un corpo che stampa `i`
- **THEN** vengono stampati `2`, `3`

#### Scenario: Filtro sulla chiave

- **WHEN** viene eseguito `for each v,k in [10,20,30] with k<>1` con un corpo che stampa `v`
- **THEN** vengono stampati `10`, `30`

#### Scenario: Condizione non booleana

- **WHEN** viene eseguito `for each i in [1] with i`
- **THEN** il corpo non viene eseguito e il programma si interrompe con un messaggio d'errore

### Requirement: Collezione valutata una sola volta

L'espressione della collezione SHALL essere valutata esattamente una volta, prima della prima
iterazione; riassegnare nel corpo la variabile che la conteneva non SHALL cambiare gli elementi
visitati.

#### Scenario: Variabile della collezione riassegnata

- **WHEN** con `a:=[1,2,3]` viene eseguito `for each i in a` con un corpo che stampa `i` ed esegue
  `a:=[9]`
- **THEN** vengono stampati `1`, `2`, `3`

#### Scenario: Collezione con effetti collaterali

- **WHEN** la collezione è la chiamata di una funzione che stampa `chiamata` e restituisce
  `[1,2]`
- **THEN** `chiamata` viene stampato una sola volta e il corpo viene eseguito due volte

### Requirement: Variabili non influenzano la visita

Assegnare un altro valore alla variabile del valore o della chiave dentro il corpo SHALL non
cambiare l'elemento visitato all'iterazione successiva.

#### Scenario: Assegnazione all'indice nel corpo

- **WHEN** viene eseguito `for each v,k in [1,2,3]` con un corpo che stampa `v` ed esegue `k:=10`
- **THEN** vengono stampati `1`, `2`, `3`

### Requirement: Valore delle variabili dopo il ciclo

Al termine del ciclo le variabili SHALL contenere l'ultimo valore e l'ultima chiave assegnati,
anche se il filtro `with` ha saltato il corpo per quell'elemento; se la collezione è vuota SHALL
conservare il valore che avevano prima del `for each`.

#### Scenario: Dopo un ciclo completo

- **WHEN** dopo `for each i,k in [7,8] ... end` vengono stampati `i` e `k`
- **THEN** vengono stampati `8` e `1`

#### Scenario: Dopo un ciclo vuoto

- **WHEN** con `i:=4` dopo `for each i in [] ... end` viene stampato `i`
- **THEN** viene stampato `4`

### Requirement: Cicli annidati e stack invariato

Un `for each` SHALL poter contenere altri `for each`, `for` e statement di controllo, anche sulla
stessa collezione, e la sua esecuzione completa SHALL lasciare lo stack di esecuzione alla stessa
profondità che aveva prima.

#### Scenario: Prodotto cartesiano

- **WHEN** viene eseguito `for each i in [1,2]` con dentro `for each j in [10,20]` che stampa
  `i*j`
- **THEN** vengono stampati `10`, `20`, `20`, `40` e il programma termina con codice 0 e lo stack
  svuotato

#### Scenario: For each dentro una procedura chiamata più volte

- **WHEN** una procedura con una variabile locale esegue un `for each` e viene chiamata due volte
- **THEN** entrambe le chiamate eseguono il ciclo completo e il programma termina con codice 0

### Requirement: For each compilabile ed eseguibile

Un sorgente che contiene un `for each` in una delle quattro forme SHALL compilare senza errori e il
suo p-code SHALL essere eseguito senza l'errore `pcode not implemented`.

#### Scenario: Esempi di q.cpl

- **WHEN** vengono compilati ed eseguiti i cinque `for each` di `q.cpl`
- **THEN** il compilatore esce con codice 0 e l'esecuzione stampa `a[0]=1`..`a[2]=3`, poi `100`
  e `200` in un ordine qualsiasi, poi `d["alfa"]=100` e `d["beta"]=200` in un ordine qualsiasi,
  poi `1`, `2`, `t`, `2`, `3`
