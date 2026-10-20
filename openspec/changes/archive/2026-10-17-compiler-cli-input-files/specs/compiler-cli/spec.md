# Spec Delta

## Purpose

Definisce il contratto della command line del compilatore cpl2: da dove arrivano i
sorgenti da tradurre, dove viene scritto il p-code risultante, come vengono riportate
le diagnostiche e con quale codice di uscita termina il processo.

## ADDED Requirements

### Requirement: Selezione dei sorgenti
Il compilatore SHALL leggere i sorgenti dagli argomenti della command line quando ne
sono presenti, e dallo standard input quando non ne sono presenti.

#### Scenario: Compilazione di un file indicato in command line
- **WHEN** l'utente esegue `compiler fib.cpl`
- **THEN** il compilatore traduce il contenuto di `fib.cpl` e non legge lo standard input

#### Scenario: Compilazione di piu' file indicati in command line
- **WHEN** l'utente esegue `compiler fib.cpl primo.cpl test/simple_class.cpl`
- **THEN** il compilatore traduce ciascuno dei tre file, nell'ordine in cui sono stati indicati

#### Scenario: Nessun file in command line
- **WHEN** l'utente esegue `compiler` senza argomenti, anche con lo standard input reindirizzato da un file
- **THEN** il compilatore traduce lo standard input, come ha sempre fatto

### Requirement: Un p-code per file sorgente
Quando sono indicati dei file in command line, il compilatore SHALL produrre un file
p-code separato per ciascun sorgente e SHALL NOT scrivere p-code su stdout.

#### Scenario: Ogni sorgente genera il proprio p-code
- **WHEN** l'utente esegue `compiler fib.cpl primo.cpl`
- **THEN** vengono prodotti due file p-code distinti, uno per sorgente, e lo standard output resta vuoto

#### Scenario: Piu' esecuzioni non si sovrascrivono
- **WHEN** l'utente esegue due volte `compiler fib.cpl`
- **THEN** il p-code di `fib.cpl` viene riscritto dalla seconda esecuzione senza errori e senza accumulare contenuto della prima

### Requirement: Derivazione del nome del file di output
Il nome del file p-code SHALL essere ottenuto dal percorso del sorgente sostituendone
l'estensione con `.pcd`, e SHALL essere scritto nella stessa directory del sorgente.

#### Scenario: Estensione sostituita nella stessa directory
- **WHEN** l'utente esegue `compiler fib.cpl`
- **THEN** il p-code viene scritto in `fib.pcd`

#### Scenario: Sorgente in una sottodirectory
- **WHEN** l'utente esegue `compiler test/simple_class.cpl`
- **THEN** il p-code viene scritto in `test/simple_class.pcd` e non nella directory corrente

#### Scenario: Sorgente senza estensione
- **WHEN** l'utente esegue `compiler fib`
- **THEN** il p-code viene scritto in `fib.pcd`

#### Scenario: Estensione diversa da `.cpl`
- **WHEN** l'utente esegue `compiler sorg.txt`
- **THEN** il p-code viene scritto in `sorg.pcd`

### Requirement: Compatibilita' della modalita' standard input
Senza argomenti il compilatore SHALL leggere dallo standard input e SHALL scrivere il
p-code su stdout, producendo lo stesso risultato e le stesse diagnostiche prodotti
prima di questa change.

#### Scenario: Redirezione come pipeline esistente
- **WHEN** l'utente esegue `compiler < fib.cpl > fib.pcd`
- **THEN** `fib.pcd` contiene il p-code valido del sorgente, identico a quello che la stessa invocazione produceva prima della change

#### Scenario: Nessun file viene scritto in questa modalita'
- **WHEN** l'utente esegue `compiler < fib.cpl`
- **THEN** il compilatore non crea alcun file `.pcd` nella directory corrente

### Requirement: Le diagnostiche vanno su stderr
Tutte le diagnostiche del compilatore SHALL essere scritte su stderr e SHALL NOT
finire nel contenuto del file p-code.

#### Scenario: Errore di sintassi non inquina il p-code
- **WHEN** un sorgente in input contiene un errore di sintassi e il compilatore viene eseguito in modalita' file
- **THEN** il messaggio di errore compare su stderr e il file p-code prodotto contiene solo p-code

#### Scenario: Errori dello scanner su stderr
- **WHEN** un sorgente in input contiene una stringa non terminata o un numero mal terminato
- **THEN** i relativi messaggi compaiono su stderr e non nel file p-code prodotto

### Requirement: Le diagnostiche identificano il file sorgente e la riga
In modalita' file, ogni diagnostica SHALL riportare il percorso del file sorgente come
prefisso e il numero di riga SHALL ripartire da 1 per ciascun file. In modalita' standard
input le diagnostiche SHALL essere emesse senza prefisso.

#### Scenario: Prefisso con il nome del file
- **WHEN** `b.cpl` contiene un errore di sintassi alla riga 3 e viene eseguito `compiler a.cpl b.cpl`
- **THEN** su stderr compare un messaggio che inizia con `b.cpl: ` e riporta `line 3`

#### Scenario: La numerazione riparte per ogni file
- **WHEN** due file diversi hanno ciascuno un errore di sintassi alla propria riga 1 e vengono compilati insieme
- **THEN** entrambi i messaggi riportano `line 1`

#### Scenario: Modalita' standard input senza prefisso
- **WHEN** l'utente esegue `compiler < a.cpl` e il sorgente ha un errore di sintassi
- **THEN** il messaggio su stderr ha la stessa forma di quello emesso prima della change, senza prefisso di file

### Requirement: Codice di uscita pari al totale degli errori
Il codice di uscita del compilatore SHALL essere il numero totale di errori di
compilazione riscontrati, sommato su tutti i file elaborati.

#### Scenario: Nessun errore
- **WHEN** tutti i sorgenti indicati compilano senza errori
- **THEN** il compilatore termina con codice di uscita 0

#### Scenario: Errori su piu' file
- **WHEN** due sorgenti indicati producono rispettivamente 1 e 2 errori di sintassi
- **THEN** il compilatore termina con codice di uscita 3 e produce comunque i due file p-code

### Requirement: Un file non apribile non interrompe gli altri
Se un file indicato in command line non esiste, non e' leggibile o il relativo file p-code
non puo' essere scritto, il compilatore SHALL riportare la condizione, SHALL contarla fra
gli errori e SHALL proseguire con i file rimanenti.

#### Scenario: File inesistente in mezzo alla lista
- **WHEN** l'utente esegue `compiler a.cpl mancante.cpl b.cpl`
- **THEN** il compilatore segnala l'impossibilita' di aprire `mancante.cpl`, produce il p-code di `a.cpl` e di `b.cpl`, e termina con codice di uscita 1

#### Scenario: Output non scrivibile
- **WHEN** il file p-code di un sorgente non puo' essere creato, perche' la directory non esiste o non e' scrivibile
- **THEN** il compilatore segnala la condizione su stderr, non produce p-code per quel sorgente, e continua con gli altri file