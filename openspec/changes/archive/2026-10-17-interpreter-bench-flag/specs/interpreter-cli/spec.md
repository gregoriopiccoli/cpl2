# Spec Delta

## MODIFIED Requirements

### Requirement: Selezione del file da eseguire
L'interprete SHALL riconoscere come valide soltanto due forme della command line: un solo
percorso, che esegue il file, e il flag `-bench` seguito da un solo percorso, che lo misura.
Ogni altra forma SHALL produrre le istruzioni d'uso su stderr e un codice di uscita 1, senza
eseguire ne' misurare alcun programma. Un argomento che inizia con `-`, diverso dalla stringa
esatta `-bench` in prima posizione, SHALL essere trattato come un percorso, non come
un'opzione.

#### Scenario: Esecuzione del file indicato
- **WHEN** l'utente esegue `./interpreter fib.pcd` e `fib.pcd` e' un p-code valido
- **THEN** l'interprete carica ed esegue quel programma, come faceva quando il nome era scritto dentro il sorgente

#### Scenario: Nessun argomento
- **WHEN** l'utente esegue `./interpreter` senza argomenti
- **THEN** l'interprete stampa le istruzioni d'uso su stderr, non esegue nessun programma e termina con codice di uscita 1

#### Scenario: Piu' di un argomento
- **WHEN** l'utente esegue `./interpreter primo.pcd fib.pcd`
- **THEN** l'interprete segnala che accetta un solo argomento su stderr, non esegue nessun programma e termina con codice di uscita 1

#### Scenario: Argomento che sembra un'opzione
- **WHEN** l'utente esegue `./interpreter -h`
- **THEN** l'interprete non cerca un'opzione `-h`, ma tenta di caricare il file di nome `-h` e segnala che non puo' aprirlo

#### Scenario: Flag `-bench` senza percorso
- **WHEN** l'utente esegue `./interpreter -bench`
- **THEN** l'interprete stampa le istruzioni d'uso su stderr, non esegue e non misura nessun programma e termina con codice di uscita 1

#### Scenario: Flag `-bench` con piu' percorsi
- **WHEN** l'utente esegue `./interpreter -bench primo.pcd fib.pcd`
- **THEN** l'interprete segnala la condizione su stderr, non misura nessun programma e termina con codice di uscita 1

#### Scenario: Flag in posizione non riconosciuta
- **WHEN** l'utente esegue `./interpreter primo.pcd -bench`
- **THEN** l'interprete non cerca il flag e segnala che accetta un solo argomento su stderr, senza misurare nulla e con codice di uscita 1

#### Scenario: Flag scritto con due trattini
- **WHEN** l'utente esegue `./interpreter --bench fib.pcd`
- **THEN** l'interprete non cerca l'opzione e segnala su stderr che accetta un solo argomento, senza eseguire ne' misurare nulla, con codice di uscita 1

## ADDED Requirements

### Requirement: Misura di un p-code con il flag `-bench`
Quando `-bench` precede il percorso, l'interprete SHALL caricare il p-code derivato da quel
percorso e SHALL eseguirlo ripetutamente dentro il ciclo di misura, riportando su stdout la
tabella di nanobench invece della singola esecuzione. La derivazione del percorso SHALL
essere quella della modalita' normale: accodare `.pcd` quando il nome di base non contiene
alcun punto, usare il percorso invariato altrimenti.

#### Scenario: Misura del file indicato
- **WHEN** l'utente esegue `./interpreter -bench fib.pcd` e `fib.pcd` e' un p-code valido
- **THEN** l'interprete carica `fib.pcd`, lo esegue piu' volte dentro il ciclo di misura e stampa su stdout la tabella dei tempi

#### Scenario: Misura di un nome senza estensione
- **WHEN** l'utente esegue `./interpreter -bench fib` e il file `fib.pcd` esiste
- **THEN** l'interprete misura `fib.pcd`, non un file di nome `fib`

#### Scenario: Misura di un percorso con punto nella directory
- **WHEN** l'utente esegue `./interpreter -bench dir.d/x` e il file `dir.d/x.pcd` esiste
- **THEN** l'interprete misura `dir.d/x.pcd`, considerando il punto della directory come parte della directory

#### Scenario: Misura di un p-code prodotto dal compilatore
- **WHEN** il compilatore traduce un sorgente in una sottodirectory, per esempio `tmp/sub/mio.cpl` in `tmp/sub/mio.pcd`, e l'utente esegue `./interpreter -bench tmp/sub/mio`
- **THEN** l'interprete misura `tmp/sub/mio.pcd`, il file appena prodotto dal compilatore nella sottodirectory

#### Scenario: La tabella di misura resta su stdout
- **WHEN** la misura di `./interpreter -bench primo.pcd` termina
- **THEN** l'output del programma misurato e la tabella dei tempi compaiono sullo standard output, insieme al rapporto del GC che segue come nelle esecuzioni normali

### Requirement: Esito del caricamento in modalita' bench
In modalita' bench il codice di uscita SHALL dipendere dal caricamento del file e non
dalla misura: se il file e' caricabile l'interprete esegue la misura e termina con codice
0, se non e' caricabile SHALL riportare su stderr la stessa diagnostica della modalita'
normale, nominando il percorso derivato, SHALL NOT eseguire alcuna misura e SHALL terminare
con codice di uscita 1.

#### Scenario: File inesistente
- **WHEN** l'utente esegue `./interpreter -bench non_esiste`
- **THEN** su stderr compare il messaggio che indica che `non_esiste.pcd` non puo' essere aperto, non viene prodotta alcuna tabella di misura e il codice di uscita e' 1

#### Scenario: File che non e' un p-code
- **WHEN** l'utente esegue `./interpreter -bench primo.cpl`
- **THEN** su stderr compare il messaggio che indica che il file non e' un p-code compilato, non viene prodotta alcuna tabella di misura e il codice di uscita e' 1

#### Scenario: File caricabile
- **WHEN** l'utente esegue `./interpreter -bench primo.pcd` e il file e' un p-code valido
- **THEN** su stdout compare la tabella di misura e il codice di uscita e' 0

#### Scenario: La diagnostica non finisce su stdout
- **WHEN** l'utente esegue `./interpreter -bench non_esiste` e ridirige stderr su `/dev/null`
- **THEN** lo standard output non contiene la diagnostica e non contiene alcuna tabella di misura