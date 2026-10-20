# Spec Delta

## Purpose

Definisce il contratto della command line di `asm`: da dove legge il p-code (file o
standard input), dove viene scritto il risultato, come riporta gli errori di uso e
con quale codice di uscita termina il processo.

## ADDED Requirements

### Requirement: Lettura dall'argomento file
Quando sulla command line e' indicato un percorso, `asm` SHALL leggere il p-code da
quel file e SHALL NON leggere lo standard input.

#### Scenario: File indicato in command line
- **WHEN** l'utente esegue `./asm fib.pcd`
- **THEN** `asm` traduce il contenuto di `fib.pcd` e non legge lo standard input

#### Scenario: Il risultato finisce su stdout
- **WHEN** l'utente esegue `./asm fib.pcd > out.txt`
- **THEN** `out.txt` contiene la listing prodotta dalla traduzione di `fib.pcd`

### Requirement: Compatibilita' della modalita' standard input
Senza argomenti `asm` SHALL leggere dallo standard input e SHALL produrre lo stesso
risultato e gli stessi codici di uscita prodotti prima di questa change.

#### Scenario: Redirezione come pipeline esistente
- **WHEN** l'utente esegue `./asm < fib.pcd`
- **THEN** lo standard output contiene la listing identica a quella che la stessa
  invocazione produceva prima della change

#### Scenario: Solo `-` indica lo standard input
- **WHEN** l'utente esegue `cat fib.pcd | ./asm -`
- **THEN** `asm` traduce lo standard input, come nella modalita' senza argomenti

### Requirement: Un solo percorso
`asm` SHALL accettare al massimo un percorso. Se gli argomenti posizionali sono piu'
di uno, SHALL riportare la condizione su stderr, SHALL NON tradurre nulla e SHALL
terminare con codice di uscita 1.

#### Scenario: Due percorsi in command line
- **WHEN** l'utente esegue `./asm a.pcd b.pcd`
- **THEN** su stderr compare un messaggio che segnala il troppi percorsi, non viene
  prodotta alcuna listing e il codice di uscita e' 1

### Requirement: L'output resta su stdout
In entrambe le modalità `asm` SHALL scrivere il risultato solo sullo standard output e
SHALL NON creare alcun file.

#### Scenario: Nessun file prodotto in modalita' file
- **WHEN** l'utente esegue `./asm fib.pcd` in una directory
- **THEN** lo standard output contiene la listing e nella directory non viene creato
  alcun file

### Requirement: File non apribile
Se il percorso indicato non puo' essere aperto, `asm` SHALL riportare la condizione
nominando il percorso su stderr, SHALL NON produrre output e SHALL terminare con
codice di uscita 1.

#### Scenario: Percorso inesistente
- **WHEN** l'utente esegue `./asm mancante.pcd` e il file non esiste
- **THEN** su stderr compare un messaggio che nomina `mancante.pcd`, lo standard output
  resta vuoto e il codice di uscita e' 1

#### Scenario: Il percorso non compare su stdout
- **WHEN** l'utente esegue `./asm mancante.pcd` ridirigendo stderr su `/dev/null`
- **THEN** lo standard output non contiene alcun messaggio di errore

### Requirement: Nessuna opzione dichiara asm
`asm` SHALL non dichiarare opzioni: un argomento che comincia con `-` e non e' il solo
`-` e' un'opzione sconosciuta, SHALL essere segnalato su stderr nominandolo e `asm`
SHALL terminare con codice di uscita 1 senza tradurre nulla; `--` SHALL terminare il
riconoscimento delle opzioni e ogni argomento successivo sara' un percorso.

#### Scenario: Opzione sconosciuta corta
- **WHEN** l'utente esegue `./asm -b fib.pcd`
- **THEN** su stderr compare un messaggio che nomina `-b` come opzione sconosciuta,
  non viene prodotta alcuna listing e il codice di uscita e' 1

#### Scenario: Terminatore `--`
- **WHEN** l'utente esegue `./asm -- -file.pcd` e il file `-file.pcd` esiste
- **THEN** `asm` traduce il contenuto di `-file.pcd`, perche' dopo `--` gli argomenti
  sono percorsi

### Requirement: Codici di uscita della conversione invariati
Il codice di uscita che `asm` restituisce quando la traduzione riesce o quando
l'input non e' p-code SHALL restare identico a quello di prima della change, sia in
modalita' file sia in modalita' standard input.

#### Scenario: Binario verso listing
- **WHEN** l'input contiene p-code binario (`#cpl2...`) e la traduzione riesce
- **THEN** il codice di uscita e' 2

#### Scenario: Listing verso binario
- **WHEN** l'input contiene una listing leggibile (`CPL2 PCODE VER:...`) e la
  traduzione riesce
- **THEN** il codice di uscita e' 1

#### Scenario: Input che non e' p-code
- **WHEN** la prima riga dell'input non e' ne' p-code binario ne' listing leggibile
- **THEN** il codice di uscita e' 0
