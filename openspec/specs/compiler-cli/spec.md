# compiler-cli Specification

## Purpose

Definisce il contratto della command line del compilatore cpl2: da dove arrivano i
sorgenti da tradurre, dove viene scritto il p-code risultante, come vengono riportate
le diagnostiche e con quale codice di uscita termina il processo.

## Requirements

### Requirement: Selezione dei sorgenti
Il compilatore SHALL leggere i sorgenti dagli argomenti della command line che sono percorsi,
e dallo standard input quando non e' presente alcun percorso. Un argomento che inizia con `-`
e non e' il solo `-` e' un'opzione e non un sorgente; `--` termina il riconoscimento delle
opzioni e ogni argomento successivo e' un sorgente; il solo `-` indica lo standard input.

#### Scenario: Compilazione di un file indicato in command line
- **WHEN** l'utente esegue `compiler fib.cpl`
- **THEN** il compilatore traduce il contenuto di `fib.cpl` e non legge lo standard input

#### Scenario: Compilazione di piu' file indicati in command line
- **WHEN** l'utente esegue `compiler fib.cpl primo.cpl test/simple_class.cpl`
- **THEN** il compilatore traduce ciascuno dei tre file, nell'ordine in cui sono stati indicati

#### Scenario: Nessun file in command line
- **WHEN** l'utente esegue `compiler` senza argomenti, anche con lo standard input reindirizzato da un file
- **THEN** il compilatore traduce lo standard input, come ha sempre fatto

#### Scenario: Il solo trattino indica lo standard input
- **WHEN** l'utente esegue `cat fib.cpl | compiler -`
- **THEN** il compilatore traduce lo standard input e scrive il p-code su stdout, come nella modalita' senza argomenti

#### Scenario: Sorgente il cui nome comincia con un trattino
- **WHEN** l'utente esegue `compiler fib.cpl ./-sorgente.cpl` e il file `./-sorgente.cpl` esiste
- **THEN** il compilatore traduce anche `./-sorgente.cpl`, perche' il percorso digitato non comincia con `-`

#### Scenario: Terminatore `--`
- **WHEN** l'utente esegue `compiler -- -sorgente.cpl` e il file `-sorgente.cpl` esiste
- **THEN** il compilatore traduce il contenuto di `-sorgente.cpl`, perche' dopo `--` gli argomenti sono percorsi

### Requirement: Opzione sconosciuta in command line
Se la command line contiene un'opzione che il compilatore non dichiara, il compilatore SHALL
segnalarla su stderr nominandola, SHALL NOT tradurre alcun sorgente e SHALL terminare con
codice di uscita 1. Questa condizione e' diversa da un sorgente che non puo' essere aperto:
un'opzione sconosciuta riguarda tutta l'invocazione, quindi gli altri sorgenti non vengono
tradotti.

#### Scenario: Opzione sconosciuta corta
- **WHEN** l'utente esegue `compiler -x fib.cpl`
- **THEN** su stderr compare un messaggio che nomina `-x` come opzione sconosciuta, non viene prodotto alcun p-code e il codice di uscita e' 1

#### Scenario: Opzione sconosciuta lunga
- **WHEN** l'utente esegue `compiler --verbose fib.cpl`
- **THEN** su stderr compare un messaggio che nomina `--verbose` come opzione sconosciuta, non viene prodotto alcun p-code e il codice di uscita e' 1

#### Scenario: Argomento che sembrava un sorgente
- **WHEN** l'utente esegue `compiler -h fib.cpl`
- **THEN** il compilatore non tenta di aprire il file di nome `-h`, segnala `-h` come opzione sconosciuta e termina con codice di uscita 1

#### Scenario: L'opzione sconosciuta interrompe gli altri sorgenti
- **WHEN** l'utente esegue `compiler a.cpl -x b.cpl` e `a.cpl` e `b.cpl` esistono
- **THEN** il compilatore segnala `-x` come opzione sconosciuta, non produce ne' `a.pcd` ne' `b.pcd` e termina con codice di uscita 1

#### Scenario: La segnalazione non finisce su stdout
- **WHEN** l'utente esegue `compiler -x fib.cpl` ridirigendo stderr su `/dev/null`
- **THEN** lo standard output non contiene la segnalazione dell'opzione sconosciuta

#### Scenario: Le opzioni dichiarate non sono ancora accettate dal compilatore
- **WHEN** l'utente esegue `compiler --bench fib.cpl`
- **THEN** il compilatore segnala `--bench` come opzione sconosciuta, perche' l'opzione di misura appartiene all'interprete e il compilatore non ne dichiara nessuna

### Requirement: Risoluzione implicita di `.cpl` per un nome senza estensione
Quando il nome di base del percorso indicato in command line non contiene alcun punto e quel
percorso non puo' essere aperto perche' non esiste, il compilatore SHALL tentare di aprire
anche il percorso ottenuto accodando `.cpl` e, se si apre, SHALL tradurre il suo contenuto.

#### Scenario: Nome senza estensione risolto come `.cpl`
- **WHEN** l'utente esegue `compiler fib` in una directory dove `fib` non esiste e `fib.cpl` esiste
- **THEN** il compilatore traduce il contenuto di `fib.cpl` e scrive il p-code in `fib.pcd`

#### Scenario: Il percorso digitato ha la precedenza
- **WHEN** l'utente esegue `compiler fib` in una directory dove esistono sia `fib` sia `fib.cpl`
- **THEN** il compilatore traduce il contenuto di `fib` e non di `fib.cpl`

#### Scenario: Nome con estensione mai riscritto
- **WHEN** l'utente esegue `compiler sorg.txt` e `sorg.txt` non esiste, in una directory dove esiste `sorg.cpl`
- **THEN** il compilatore segnala che non puo' aprire `sorg.txt` e non traduce `sorg.cpl`

#### Scenario: Il punto si cerca solo nel nome di base
- **WHEN** l'utente esegue `compiler dir.d/x` in una directory dove `dir.d/x` non esiste e `dir.d/x.cpl` esiste
- **THEN** il compilatore traduce il contenuto di `dir.d/x.cpl` e scrive il p-code in `dir.d/x.pcd`

#### Scenario: Il ripiego scatta solo se il file non esiste
- **WHEN** l'utente esegue `compiler fib` in una directory dove `fib` esiste ma non e' leggibile e `fib.cpl` esiste ed e' leggibile
- **THEN** il compilatore segnala che non puo' aprire `fib` e non traduce `fib.cpl`

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

### Requirement: Le diagnostiche nominano il percorso risolto
Ogni diagnostica emessa durante la traduzione di un sorgente indicato in command line SHALL
essere preceduta dal percorso del file realmente tradotto, cioe' quello aperto, e non dal
percorso originariamente digitato.

#### Scenario: Prefisso con il percorso risolto
- **WHEN** l'utente esegue `compiler fib`, esiste solo `fib.cpl` e `fib.cpl` ha un errore di sintassi alla riga 3
- **THEN** su stderr compare un messaggio che inizia con `fib.cpl: ` e riporta `line 3`

#### Scenario: Scanner sul percorso risolto
- **WHEN** l'utente esegue `compiler fib`, esiste solo `fib.cpl` e `fib.cpl` contiene una stringa non terminata
- **THEN** su stderr compare il messaggio dello scanner con il prefisso `fib.cpl: `

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
non puo' essere scritto, il compilatore SHALL riportare la condizione nominando tutti i
percorsi che ha tentato di aprire per quel sorgente, SHALL contarla fra gli errori e SHALL
proseguire con i file rimanenti.

#### Scenario: File inesistente in mezzo alla lista
- **WHEN** l'utente esegue `compiler a.cpl mancante.cpl b.cpl`
- **THEN** il compilatore segnala l'impossibilita' di aprire `mancante.cpl`, produce il p-code di `a.pcd` e di `b.pcd`, e termina con codice di uscita 1

#### Scenario: Output non scrivibile
- **WHEN** il file p-code di un sorgente non puo' essere creato, perche' la directory non esiste o non e' scrivibile
- **THEN** il compilatore segnala la condizione su stderr, non produce p-code per quel sorgente, e continua con gli altri file

#### Scenario: Percorsi tentati nella diagnostica del sorgente non apribile
- **WHEN** l'utente esegue `compiler fib` in una directory dove non esistono ne' `fib` ne' `fib.cpl`
- **THEN** su stderr compare una diagnostica che cita sia `fib` sia `fib.cpl`, non viene prodotto alcun p-code e il codice di uscita e' 1

#### Scenario: Un risolto e uno irraggiungibile nella stessa invocazione
- **WHEN** l'utente esegue `compiler fib mancante` in una directory dove esiste solo `fib.cpl` e non esistono `mancante` ne' `mancante.cpl`
- **THEN** il compilatore produce il p-code di `fib.cpl`, segnala l'irraggiungibilita' di `mancante` e `mancante.cpl` e termina con codice di uscita 1
