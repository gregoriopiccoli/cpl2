# Spec Delta

## MODIFIED Requirements

### Requirement: Selezione del file da eseguire
L'interprete SHALL eseguire il p-code derivato dal percorso indicato come unico percorso
della command line, e SHALL misurarlo quando l'opzione di misura e' presente. Le opzioni
possono comparire prima o dopo il percorso e non sono contate come percorsi. Senza
percorsi, con piu' di un percorso, o quando l'opzione di misura e' presente senza
percorsi, l'interprete SHALL produrre le istruzioni d'uso su stderr e terminare con codice
di uscita 1, senza eseguire ne' misurare alcun programma.

#### Scenario: Esecuzione del file indicato
- **WHEN** l'utente esegue `./interpreter fib.pcd` e `fib.pcd` e' un p-code valido
- **THEN** l'interprete carica ed esegue quel programma, come faceva quando il nome era scritto dentro il sorgente

#### Scenario: Nessun argomento
- **WHEN** l'utente esegue `./interpreter` senza argomenti
- **THEN** l'interprete stampa le istruzioni d'uso su stderr, non esegue nessun programma e termina con codice di uscita 1

#### Scenario: Piu' di un argomento
- **WHEN** l'utente esegue `./interpreter primo.pcd fib.pcd`
- **THEN** l'interprete segnala che accetta un solo argomento su stderr, non esegue nessun programma e termina con codice di uscita 1

#### Scenario: Flag `-bench` senza percorso
- **WHEN** l'utente esegue `./interpreter -bench`
- **THEN** l'interprete stampa le istruzioni d'uso su stderr, non esegue e non misura nessun programma e termina con codice di uscita 1

#### Scenario: Flag `-bench` con piu' percorsi
- **WHEN** l'utente esegue `./interpreter -bench primo.pcd fib.pcd`
- **THEN** l'interprete segnala la condizione su stderr, non misura nessun programma e termina con codice di uscita 1

#### Scenario: Argomento che sembra un'opzione
- **WHEN** l'utente esegue `./interpreter -h`
- **THEN** l'interprete segnala `-h` come opzione sconosciuta su stderr e termina con codice di uscita 1, senza cercare il file di nome `-h`

#### Scenario: Flag in posizione non riconosciuta
- **WHEN** l'utente esegue `./interpreter primo.pcd -bench`
- **THEN** l'interprete riconosce `-bench` anche in coda alla riga e misura `primo.pcd`, invece di segnalare che accetta un solo argomento

#### Scenario: Flag scritto con due trattini
- **WHEN** l'utente esegue `./interpreter --bench fib.pcd`
- **THEN** l'interprete riconosce `--bench` come opzione e misura `fib.pcd`

### Requirement: Derivazione del percorso da caricare
Il percorso caricato SHALL essere derivato dal nome di base del solo percorso indicato: se il
nome di base non contiene alcun punto, ad esso SHALL essere accodato `.pcd`; se ne contiene
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

### Requirement: Misura di un p-code con il flag `-bench`
Quando l'opzione di misura e' presente, l'interprete SHALL caricare il p-code derivato
dall'unico percorso indicato e SHALL eseguirlo ripetutamente dentro il ciclo di misura,
riportando su stdout la tabella di nanobench invece della singola esecuzione. L'opzione di
misura SHALL essere riconosciuta con le tre grafie `--bench`, `-b` e `-bench`. La derivazione
del percorso e' quella descritta nel requisito sulla derivazione.

#### Scenario: Misura del file indicato
- **WHEN** l'utente esegue `./interpreter -bench fib.pcd` e `fib.pcd` e' un p-code valido
- **THEN** l'interprete carica `fib.pcd`, lo esegue piu' volte dentro il ciclo di misura e stampa su stdout la tabella dei tempi

#### Scenario: Misura con il nome lungo dell'opzione
- **WHEN** l'utente esegue `./interpreter --bench fib.pcd` e `fib.pcd` e' un p-code valido
- **THEN** l'interprete misura `fib.pcd`, come con la grafia `-bench`

#### Scenario: Misura con il nome corto dell'opzione
- **WHEN** l'utente esegue `./interpreter -b fib.pcd` e `fib.pcd` e' un p-code valido
- **THEN** l'interprete misura `fib.pcd`, come con la grafia `-bench`

#### Scenario: Misura con l'opzione dopo il percorso
- **WHEN** l'utente esegue `./interpreter fib.pcd -b` e `fib.pcd` e' un p-code valido
- **THEN** l'interprete misura `fib.pcd`

#### Scenario: Stessa opzione ripetuta
- **WHEN** l'utente esegue `./interpreter --bench -b fib.pcd`
- **THEN** l'interprete misura `fib.pcd` una sola volta, e non segnala alcun errore per la ripetizione

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

## ADDED Requirements

### Requirement: Riconoscimento delle opzioni
Un argomento che inizia con `-` e non e' il solo `-` SHALL essere trattato come opzione, non
come percorso, in qualunque posizione della command line. Un'opzione SHALL essere riconosciuta
dal nome lungo (`--bench`) o dal nome corto (`-b`), e `--` SHALL terminare il riconoscimento:
ogni argomento successivo a `--` e' un percorso. `-` da solo non e' un'opzione.

#### Scenario: Nome lungo dell'opzione
- **WHEN** l'utente esegue `./interpreter --bench fib.pcd`
- **THEN** l'interprete riconosce `--bench` come opzione e misura `fib.pcd`

#### Scenario: Opzione corta
- **WHEN** l'utente esegue `./interpreter -b fib.pcd`
- **THEN** l'interprete riconosce `-b` come la stessa opzione di `--bench` e misura `fib.pcd`

#### Scenario: Opzione corta unita al valore
- **WHEN** l'utente esegue `./interpreter -b=on fib.pcd`
- **THEN** l'interprete segnala che `-b` non accetta un valore, su stderr, con le istruzioni d'uso e codice di uscita 1

#### Scenario: Opzione sconosciuta corta
- **WHEN** l'utente esegue `./interpreter -z fib.pcd`
- **THEN** su stderr compare un messaggio che segnala `-z` come opzione sconosciuta insieme alle istruzioni d'uso, non viene eseguito ne' misurato nulla e il codice di uscita e' 1

#### Scenario: Percorso con directory esplicita
- **WHEN** l'utente esegue `./interpreter ./-bench` e il file `./-bench.pcd` esiste
- **THEN** l'interprete carica `./-bench.pcd`, perche' il percorso e' `./-bench` e non inizia con `-`

#### Scenario: Terminatore `--`
- **WHEN** l'utente esegue `./interpreter -- --bench` e il file `--bench.pcd` esiste
- **THEN** l'interprete carica il file di nome `--bench.pcd`, perche' dopo `--` gli argomenti sono percorsi

#### Scenario: Percorso dopo il terminatore
- **WHEN** l'utente esegue `./interpreter --bench -- fib.pcd`
- **THEN** l'interprete misura `fib.pcd`, perche' `fib.pcd` segue il terminatore e resta un percorso

#### Scenario: Il solo trattino non e' un'opzione
- **WHEN** l'utente esegue `./interpreter -`
- **THEN** l'interprete non cerca alcuna opzione `-` e tratta `-` come un percorso

### Requirement: Le istruzioni d'uso elencano le opzioni dichiarate
Le istruzioni d'uso dell'interprete SHALL riportare, per ogni opzione che l'interprete
riconosce, tutte le grafie con cui e' accettata e che cosa fa. Le istruzioni d'uso vanno su
stderr quando accompagnano un errore di command line.

#### Scenario: L'uso nomina le tre grafie dell'opzione di misura
- **WHEN** l'utente esegue `./interpreter` senza argomenti e legge stderr
- **THEN** le istruzioni d'uso riportano che il file si misura con `--bench`, `-b` o `-bench`

#### Scenario: L'uso resta su stderr
- **WHEN** l'utente esegue `./interpreter -z fib.pcd` ridirigendo stderr su `/dev/null`
- **THEN** lo standard output non contiene le istruzioni d'uso ne' la segnalazione dell'opzione sconosciuta