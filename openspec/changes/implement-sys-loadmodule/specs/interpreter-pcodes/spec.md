# Spec Delta

## ADDED Requirements

### Requirement: Caricamento dei moduli da parte di IMPORT

L'interprete SHALL eseguire `IMPORT` usando l'intero N del suo operando come numero di nomi:
SHALL verificare che le N posizioni in cima allo stack siano stringhe, altrimenti SHALL
sollevare `domain_error` senza caricare nulla; SHALL caricare ed eseguire i moduli indicati
nell'ordine in cui compaiono sullo stack (dal basso verso la cima), e SHALL rimuovere le N
stringhe lasciando intatto ciò che sta sotto. `IMPORT` SHALL NOT stampare i nomi.

#### Scenario: Un solo modulo

- **WHEN** viene eseguito `IMPORT 1` quando in cima allo stack c'è la stringa `a` e `a.pcd`
  è caricabile
- **THEN** il modulo `a` viene eseguito, la stringa viene rimossa e sullo standard output
  compare solo ciò che il modulo stampa

#### Scenario: Più moduli

- **WHEN** viene eseguito `IMPORT 2` quando in cima allo stack ci sono due stringhe
  consecutive, `a.b` sotto `c.d`, ed entrambi i moduli sono caricabili
- **THEN** viene eseguito prima il modulo `a.b` e poi il modulo `c.d`, ed entrambe le stringhe
  vengono rimosse

#### Scenario: Stesso modulo ripetuto nella lista

- **WHEN** viene eseguito `IMPORT 2` quando in cima allo stack ci sono due stringhe uguali `a`
- **THEN** il modulo `a` viene eseguito una sola volta ed entrambe le stringhe vengono rimosse

#### Scenario: Operando minore delle stringhe presenti

- **WHEN** viene eseguito `IMPORT 1` quando in cima allo stack ci sono due stringhe
  consecutive, `a` sotto `b`
- **THEN** viene caricato e rimosso solo il modulo `b`, e `a` resta sullo stack

#### Scenario: Stack pulito per il comando successivo

- **WHEN** due comandi `import` vengono eseguiti uno dopo l'altro, ciascuno con un solo modulo
- **THEN** ciascun `IMPORT` carica il proprio modulo, perché il primo ha rimosso la propria
  stringa

#### Scenario: Meno stringhe dell'operando

- **WHEN** viene eseguito `IMPORT 2` quando in cima allo stack c'è una sola stringa
- **THEN** viene sollevato `domain_error` senza caricare alcun modulo

#### Scenario: Posizione che non contiene una stringa

- **WHEN** viene eseguito `IMPORT 2` quando una delle due posizioni in cima allo stack non
  contiene una stringa
- **THEN** viene sollevato `domain_error` prima di caricare qualunque modulo

#### Scenario: Modulo che non si carica

- **WHEN** `IMPORT` indica un modulo il cui p-code non esiste o non è valido
- **THEN** l'interprete segnala il fallimento e solleva `domain_error`, invece di proseguire
  come se il modulo fosse stato incluso

### Requirement: Esecuzione del p-code LOADX

L'interprete SHALL eseguire `LOADX`, il cui operando stringa è un nome di simbolo: SHALL usare
il contenitore in cima allo stack di esecuzione e SHALL sostituirlo con il valore che il
contenitore ha per quel nome, senza cambiare la profondità. Se il contenitore non espone quel
nome, l'interprete SHALL riportare l'errore del contenitore. L'esecuzione di un `.pcd` che
contiene `LOADX` SHALL NOT terminare con `pcode not implemented`.

#### Scenario: Lettura di un nome del modulo

- **WHEN** viene eseguito `LOAD q1` seguito da `LOADX i` e `q1` vale un contenitore con il nome
  `i` impostato a 100
- **THEN** lo stack contiene `100` al posto del contenitore e la profondità non cambia

#### Scenario: Catena di nomi puntati

- **WHEN** viene eseguita una catena `LOAD a` + `LOADX b` + `LOADX c`
- **THEN** ogni `LOADX` legge il nome dal contenitore prodotto dal passo precedente

#### Scenario: Nome assente

- **WHEN** viene eseguito `LOADX i` su un contenitore che non ha `i`
- **THEN** l'interprete riporta l'errore del contenitore e non produce un valore

### Requirement: Esecuzione del p-code STOREX

L'interprete SHALL eseguire `STOREX`, il cui operando stringa è un nome di simbolo: SHALL usare
il contenitore e il valore in cima allo stack (`[..., contenitore, valore]`), SHALL scrivere il
valore nel contenitore sotto quel nome e SHALL lasciare il valore in cima allo stack, come
`STORE`. L'esecuzione di un `.pcd` che contiene `STOREX` SHALL NOT terminare con `pcode not
implemented`.

#### Scenario: Scrittura di un nome del modulo

- **WHEN** viene eseguito `LOAD q1`, poi viene messo `5` in cima allo stack e viene eseguito
  `STOREX i`
- **THEN** il contenitore `q1` ha `i` impostato a `5` e lo stack contiene `5` al posto del
  contenitore, con profondità diminuita di uno

## MODIFIED Requirements

### Requirement: IMPORTID, IMPORTIDX e IMPORT non sono più p-code non implementati

L'interprete SHALL riconoscere `IMPORTID`, `IMPORTIDX` e `IMPORT` nella fabbrica dei p-code:
l'esecuzione di un `.pcd` che contiene un comando `import` senza `from` non SHALL terminare con
l'errore `pcode not implemented`.

#### Scenario: Programma cpl2 con import semplice

- **WHEN** viene eseguito il p-code compilato da un sorgente che contiene `import a` e il
  modulo `a.pcd` è caricabile
- **THEN** il programma si esegue senza l'errore `pcode not implemented` e termina con codice 0

#### Scenario: Programma cpl2 con nome puntato

- **WHEN** viene eseguito il p-code compilato da un sorgente che contiene `import a.b.c` e il
  modulo esiste come `a/b/c.pcd`
- **THEN** il programma si esegue senza l'errore `pcode not implemented` e non stampa `a.b.c`
  come faceva quando `IMPORT` si limitava a elencare i nomi

#### Scenario: Programma cpl2 con modulo mancante

- **WHEN** viene eseguito il p-code compilato da un sorgente che contiene `import a` e il
  modulo `a.pcd` non esiste
- **THEN** il programma non termina con l'errore `pcode not implemented` ma con l'errore di
  caricamento del modulo

## REMOVED Requirements

### Requirement: Stampa e rimozione delle stringhe da parte di IMPORT

**Reason**: `IMPORT` non è più un comando diagnostico: carica ed esegue i moduli e non stampa
più i loro nomi.

**Migration**: usare `import` come prima; i nomi non compaiono più sullo standard output e al
loro posto compare l'output prodotto dall'esecuzione del modulo. La verifica del numero N e
l'errore sulle posizioni non stringa restano descritti dal requisito "Caricamento dei moduli da
parte di IMPORT".

### Requirement: IMPORT non include ancora i moduli

**Reason**: l'inclusione dei moduli è ora implementata: `IMPORT` carica ed esegue il modulo
indicato invece di limitarsi a stamparne il nome.

**Migration**: nessuna azione necessaria per i programmi che usavano `import`; un modulo
assente ora è un errore a runtime invece di una riga stampata. I nomi del modulo si raggiungono
attraverso il simbolo creato dall'import (`q1.i`); resta fuori perimetro `from ... import ...`.
