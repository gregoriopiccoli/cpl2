# Spec Delta

## Purpose

Definisce il contratto della command line dell'interprete cpl2: quale file p-code viene
eseguito e come il nome indicato viene trasformato in un percorso, cosa succede quando
l'argomento manca o sono in piu', e come vengono riportati i file che non si possono
caricare.

## ADDED Requirements

### Requirement: Selezione del file da eseguire
L'interprete SHALL eseguire il file p-code indicato come unico argomento della command
line. Senza argomenti, o con piu' di un argomento, l'interprete SHALL NOT eseguire alcun
programma, SHALL riportare la condizione su stderr e SHALL terminare con codice di uscita
1. Un argomento che inizia con `-` SHALL essere trattato come un percorso, non come
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

### Requirement: Derivazione del percorso da caricare
Il percorso caricato SHALL essere derivato dal nome di base del solo argomento: se il nome
di base non contiene alcun punto, ad esso SHALL essere accodato `.pcd`; se ne contiene
uno, il percorso indicato SHALL essere usato invariato. Il punto SHALL essere cercato
solo nel nome di base, mai nella directory, e la directory indicata SHALL essere
conservata.

#### Scenario: Nome senza estensione
- **WHEN** l'utente esegue `./interpreter fib` e il file `fib.pcd` esiste
- **THEN** l'interprete carica ed esegue `fib.pcd`

#### Scenario: Nome gia' con estensione
- **WHEN** l'utente esegue `./interpreter fib.pcd`
- **THEN** l'interprete usa il percorso `fib.pcd` come indicato, senza appendere nulla

#### Scenario: Estensione diversa da `.pcd`
- **WHEN** l'utente esegue `./interpreter fib.cpl`
- **THEN** l'interprete tenta di caricare `fib.cpl` e segnala che il file non e' un p-code compilato, senza cercare `fib.pcd` al suo posto

#### Scenario: Punto nella directory e non nel nome di base
- **WHEN** l'utente esegue `./interpreter dir.d/x`
- **THEN** l'interprete carica `dir.d/x.pcd`, considerando il punto della directory come parte della directory

#### Scenario: File in una sottodirectory
- **WHEN** l'utente esegue `./interpreter test/simple_class` e `test/simple_class.pcd` esiste
- **THEN** l'interprete carica ed esegue `test/simple_class.pcd`

### Requirement: Diagnostica e codice di uscita per un file non caricabile
Se il file derivato non puo' essere aperto, non e' un p-code compilato, oppure contiene
errori di compilazione, l'interprete SHALL riportare la condizione su stderr identificando
il percorso del file, SHALL NOT eseguire il programma e SHALL terminare con codice di uscita
1. Tali diagnostiche SHALL NOT essere scritte sullo standard output.

#### Scenario: File inesistente
- **WHEN** l'utente esegue `./interpreter non_esiste`
- **THEN** su stderr compare un messaggio che indica che `non_esiste.pcd` non puo' essere aperto, non viene eseguito nulla e il codice di uscita e' 1

#### Scenario: File che non e' un p-code
- **WHEN** l'utente esegue `./interpreter primo.cpl`
- **THEN** su stderr compare un messaggio che indica che il file non e' un p-code compilato e il codice di uscita e' 1

#### Scenario: P-code con errori di compilazione
- **WHEN** l'utente esegue `./interpreter rotto.pcd`, prodotto da un sorgente con errori di sintassi
- **THEN** su stderr compare un messaggio che indica che il file ha errori, il programma non viene eseguito e il codice di uscita e' 1

#### Scenario: L'output del programma resta su stdout
- **WHEN** un p-code valido viene eseguito con successo e il programma stampa qualcosa
- **THEN** l'output del programma compare sullo standard output, non viene dirottato e non viene mescolato alle diagnostiche

### Requirement: Codice di uscita in caso di esecuzione
Quando il file caricato e' un p-code valido e il programma arriva in fondo, l'interprete
SHALL terminare con codice di uscita 0.

#### Scenario: Programma eseguito fino in fondo
- **WHEN** l'utente esegue `./interpreter primo.pcd` e il programma termina normalmente
- **THEN** il codice di uscita dell'interprete e' 0