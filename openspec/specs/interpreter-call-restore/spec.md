# interpreter-call-restore Specification

## Purpose

Garantisce che dopo una chiamata a funzione o procedura definita in un modulo importato, l'interprete ripristini correttamente il programma, il contesto e il contatore di programma del chiamante, permettendo all'esecuzione di continuare nel file p-code corretto senza segfault o stampa di codice errato.

## Requirements

### Requirement: Ripristino del programma del chiamante dopo una chiamata tra moduli
L'interprete SHALL ripristinare il programma p-code del chiamante dopo il ritorno da una chiamata a una funzione o procedura definita in un modulo importato. Il programma corrente deve tornare all'oggetto p-code del modulo/file che ha effettuato la chiamata e deve contenere le istruzioni corrette da eseguire.

#### Scenario: Chiamata a funzione definita in modulo importato
- **WHEN** il programma principale importa un modulo che definisce una funzione e chiama quella funzione
- **THEN** dopo il ritorno dalla funzione, l'interprete continua l'esecuzione nel programma del chiamante e non esegue istruzioni del modulo

### Requirement: Ripristino del contesto del chiamante
L'interprete SHALL ripristinare il contesto di esecuzione del chiamante dopo il ritorno da una chiamata a una procedura/funzione definita in un modulo importato, in modo che i nomi e le variabili del chiamante restino accessibili come previsto.

#### Scenario: Accesso a variabili del chiamante dopo chiamata a modulo
- **WHEN** il chiamante accede a proprie variabili dopo aver chiamato una funzione definita in un modulo importato
- **THEN** le variabili del chiamante sono ancora accessibili nel contesto corretto

### Requirement: Il PC resta valido dopo il ritorno
L'interprete SHALL posizionare il contatore di programma sul punto successivo alla chiamata nel programma del chiamante e SHALL eseguire l'istruzione corretta, senza letture fuori dai limiti del programma corrente.

#### Scenario: Esecuzione continua correttamente
- **WHEN** una chiamata a funzione di modulo ritorna
- **THEN** l'istruzione successiva nel chiamante viene eseguita correttamente e il programma termina normalmente senza segfault

### Requirement: Chiamata a funzione di modulo restituisce il risultato
L'interprete SHALL pushare il risultato della funzione definita in un modulo importato nello stack del chiamante e rimuovere l'oggetto chiamata come previsto, lasciando lo stack nello stato corretto per le istruzioni successive del chiamante.

#### Scenario: Risultato usato nel chiamante
- **WHEN** il chiamante utilizza il valore di ritorno di una funzione definita in un modulo importato
- **THEN** il valore viene stampato o usato correttamente secondo il programma del chiamante

### Requirement: Nessun segfault per sequenze di chiamate a moduli
L'interprete SHALL gestire correttamente una sequenza di chiamate a funzioni/procedure definite in moduli importati (anche ripetute) senza errori di accesso alla memoria, permettendo al programma di completare l'esecuzione.

#### Scenario: q.cpl completo senza segfault
- **WHEN** viene eseguito `./interpreter q.pcd` (o `./interpreter q`) con `q` che importa `q1` e chiama `q1.doppio`
- **THEN** il programma produce l'output completo previsto (inclusa la riga finale) e termina normalmente
