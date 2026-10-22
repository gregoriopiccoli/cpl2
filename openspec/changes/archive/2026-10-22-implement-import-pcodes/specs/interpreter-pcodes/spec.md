# Spec Delta

## ADDED Requirements

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
