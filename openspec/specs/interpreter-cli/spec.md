# interpreter-cli Specification

## Purpose

Definisce il contratto della command line dell'interprete cpl2: quale file p-code viene
eseguito e come il nome indicato viene trasformato in un percorso, cosa succede quando
l'argomento manca o sono in piu', e come vengono riportati i file che non si possono
caricare.

## Requirements

### Requirement: Selezione del file da eseguire
L'interprete SHALL eseguire il p-code derivato dal percorso indicato come unico percorso
della command line, e SHALL misurarlo quando l'opzione di misura e' presente. Le opzioni
possono comparire prima o dopo il percorso e non sono contate come percorsi. Senza percorsi,
con piu' di un percorso, o quando l'opzione di misura e' presente senza percorsi,
l'interprete SHALL produrre le istruzioni d'uso su stderr e terminare con codice di uscita
1, senza eseguire ne' misurare alcun programma, a meno che l'opzione di help sia presente: in
quel caso vale il requisito sull'opzione di help.

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
- **WHEN** l'utente esegue `./interpreter -b`
- **THEN** l'interprete stampa le istruzioni d'uso su stderr, non esegue e non misura nessun programma e termina con codice di uscita 1

#### Scenario: Flag `-bench` con piu' percorsi
- **WHEN** l'utente esegue `./interpreter -b primo.pcd fib.pcd`
- **THEN** l'interprete segnala la condizione su stderr, non misura nessun programma e termina con codice di uscita 1

#### Scenario: Argomento che sembra un'opzione
- **WHEN** l'utente esegue `./interpreter -w fib.pcd`
- **THEN** l'interprete segnala `-w` come opzione sconosciuta su stderr e termina con codice di uscita 1, senza cercare il file di nome `-w`

#### Scenario: Flag in posizione non riconosciuta
- **WHEN** l'utente esegue `./interpreter primo.pcd -b`
- **THEN** l'interprete riconosce `-b` anche in coda alla riga e misura `primo.pcd`, invece di segnalare che accetta un solo argomento

#### Scenario: Flag scritto con due trattini
- **WHEN** l'utente esegue `./interpreter --bench fib.pcd`
- **THEN** l'interprete riconosce `--bench` come opzione e misura `fib.pcd`

### Requirement: Le istruzioni d'uso elencano le opzioni dichiarate
Le istruzioni d'uso dell'interprete SHALL riportare, per ogni opzione che l'interprete
riconosce, tutte le grafie con cui e' accettata e che cosa fa, piu' le due righe che spiegano
la derivazione del percorso e il terminatore `--`. Le istruzioni d'uso vanno su stderr quando
accompagnano un errore di command line, con codice di uscita 1, e sullo standard output quando
sono richieste con l'opzione di help, con codice di uscita 0.

#### Scenario: L'uso nomina le tre grafie dell'opzione di misura
- **WHEN** l'utente esegue `./interpreter` senza argomenti e legge stderr
- **THEN** le istruzioni d'uso riportano che il file si misura con `--bench` o `-b`, e non riportano `bench` come terza grafia accettata

#### Scenario: L'uso resta su stderr
- **WHEN** l'utente esegue `./interpreter -z fib.pcd` ridirigendo stderr su `/dev/null`
- **THEN** lo standard output non contiene le istruzioni d'uso ne' la segnalazione dell'opzione sconosciuta

#### Scenario: L'help elenca se stesso
- **WHEN** l'utente esegue `./interpreter --help` e legge lo standard output
- **THEN** le istruzioni d'uso riportano che l'help si chiede con `--help` o `-h`

#### Scenario: L'help esplicito va sullo standard output
- **WHEN** l'utente esegue `./interpreter --help` ridirigendo stderr su `/dev/null`
- **THEN** lo standard output contiene le istruzioni d'uso per intero, comprese le righe sulla derivazione `.pcd` e sul terminatore `--`

### Requirement: Opzione di help `-h` e `--help`
L'interprete SHALL riconoscere l'opzione di help con le due grafie `--help` e `-h`, in
qualunque posizione della command line. Quando l'opzione e' presente SHALL stampare le
istruzioni d'uso sullo standard output, SHALL NOT eseguire ne' misurare alcun programma e
SHALL terminare con codice di uscita 0, qualunque cosa contenga il resto della riga. Le
diagnostiche di command line sbagliata hanno la precedenza sull'help. Dopo il terminatore
`--` l'opzione non e' riconosciuta e vale come percorso.

#### Scenario: Help con il nome corto
- **WHEN** l'utente esegue `./interpreter -h`
- **THEN** l'interprete stampa le istruzioni d'uso sullo standard output, non esegue ne' misura nessun programma e il codice di uscita e' 0

#### Scenario: Help con il nome lungo
- **WHEN** l'utente esegue `./interpreter --help`
- **THEN** l'interprete stampa le istruzioni d'uso sullo standard output e il codice di uscita e' 0, come con la grafia `-h`

#### Scenario: Help con un percorso presente
- **WHEN** l'utente esegue `./interpreter -h fib.pcd` e `fib.pcd` e' un p-code valido
- **THEN** l'interprete stampa l'help sullo standard output e termina con codice 0, senza eseguire `fib.pcd` e senza produrre alcuna tabella di misura

#### Scenario: Help in coda alla riga
- **WHEN** l'utente esegue `./interpreter fib.pcd -h` e `fib.pcd` e' un p-code valido
- **THEN** l'interprete stampa l'help sullo standard output e termina con codice 0, senza eseguire `fib.pcd`

#### Scenario: Help insieme all'opzione di misura
- **WHEN** l'utente esegue `./interpreter -h -b fib.pcd` e `fib.pcd` e' un p-code valido
- **THEN** l'interprete stampa l'help sullo standard output e termina con codice 0, senza misurare `fib.pcd`

#### Scenario: Stessa opzione ripetuta
- **WHEN** l'utente esegue `./interpreter -h --help`
- **THEN** l'interprete stampa l'help una sola volta e non segnala alcun errore per la ripetizione

#### Scenario: Help che non accetta un valore
- **WHEN** l'utente esegue `./interpreter -h=x`
- **THEN** l'interprete segnala su stderr che `-h` non accetta un valore, con le istruzioni d'uso su stderr e codice di uscita 1, e non stampa alcun help sullo standard output

#### Scenario: Opzione sconosciuta insieme all'help
- **WHEN** l'utente esegue `./interpreter -h -z fib.pcd`
- **THEN** l'interprete segnala su stderr `-z` come opzione sconosciuta insieme alle istruzioni d'uso, non stampa l'help sullo standard output e termina con codice di uscita 1

#### Scenario: Help dopo il terminatore
- **WHEN** l'utente esegue `./interpreter -- -h` e il file `-h.pcd` esiste
- **THEN** l'interprete tratta `-h` come un percorso e carica `-h.pcd`, perche' dopo `--` gli argomenti sono percorsi

#### Scenario: L'help non viene cercato come file
- **WHEN** l'utente esegue `./interpreter --help` e non esiste alcun file di nome `--help.pcd` ne' `-h.pcd`
- **THEN** l'interprete stampa l'help ed esce con 0, senza tentare di aprire quei file

### Requirement: Riconoscimento delle opzioni
Un argomento che inizia con `-` e non e' il solo `-` SHALL essere trattato come opzione, non
come percorso, in qualunque posizione della command line. Un'opzione SHALL essere riconosciuta
dal nome lungo (`--bench`, `--help`) o dal nome corto (`-b`, `-h`), e `--` SHALL terminare il
riconoscimento: ogni argomento successivo a `--` e' un percorso. `-` da solo non e' un'opzione.

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
- **WHEN** l'utente esegue `./interpreter ./-b` e il file `./-b.pcd` esiste
- **THEN** l'interprete carica `./-b.pcd`, perche' il percorso e' `./-b` e non inizia con `-`

#### Scenario: Terminatore `--`
- **WHEN** l'utente esegue `./interpreter -- --bench` e il file `--bench.pcd` esiste
- **THEN** l'interprete carica il file di nome `--bench.pcd`, perche' dopo `--` gli argomenti sono percorsi

#### Scenario: Percorso dopo il terminatore
- **WHEN** l'utente esegue `./interpreter --bench -- fib.pcd`
- **THEN** l'interprete misura `fib.pcd`, perche' `fib.pcd` segue il terminatore e resta un percorso

#### Scenario: Il solo trattino non e' un'opzione
- **WHEN** l'utente esegue `./interpreter -`
- **THEN** l'interprete non cerca alcuna opzione `-` e tratta `-` come un percorso

### Requirement: Derivazione del percorso da caricare
Il percorso caricato SHALL essere derivato dal nome di base del solo percorso indicato: se il
nome di base non contiene alcun punto, ad esso SHALL essere accodato `.pcd`; se ne contiene
uno, il percorso indicato SHALL essere usato invariato. Il punto SHALL essere cercato
solo nel nome di base, mai nella directory, e la directory indicata SHALL essere
conservata. Per l'esecuzione normale, l'interprete SHALL utilizzare la stessa regola di derivazione del percorso (`pcodePathFor`) e SHALL caricare/eseguire il file derivato usando il meccanismo di esecuzione del p-code (come `runPcd`), assicurando il corretto funzionamento dei comandi come `./interpreter q` anche quando il file `q.pcd` si trova accanto al sorgente.

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

#### Scenario: Nome senza estensione per file accanto al sorgente
- **WHEN** l'utente esegue `./interpreter q` e il file `q.pcd` esiste nella directory corrente
- **THEN** l'interprete lo carica ed esegue correttamente

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

### Requirement: Misura di un p-code con il flag `-bench`
Quando l'opzione di misura e' presente, l'interprete SHALL caricare il p-code derivato
dall'unico percorso indicato e SHALL eseguirlo ripetutamente dentro il ciclo di misura,
riportando su stdout la tabella di nanobench invece della singola esecuzione. L'opzione di
misura SHALL essere riconosciuta con le due grafie `--bench` e `-b`; `-bench`, con un solo
trattino, SHALL essere trattata come opzione sconosciuta. La derivazione del percorso e' quella
descritta nel requisito sulla derivazione.

#### Scenario: Misura del file indicato
- **WHEN** l'utente esegue `./interpreter -b fib.pcd` e `fib.pcd` e' un p-code valido
- **THEN** l'interprete carica `fib.pcd`, lo esegue piu' volte dentro il ciclo di misura e stampa su stdout la tabella dei tempi

#### Scenario: Grafia con un solo trattino
- **WHEN** l'utente esegue `./interpreter -bench fib.pcd` e `fib.pcd` e' un p-code valido
- **THEN** l'interprete segnala `-bench` come opzione sconosciuta su stderr con le istruzioni d'uso e il codice di uscita e' 1, e non misura `fib.pcd`

#### Scenario: Misura con il nome lungo dell'opzione
- **WHEN** l'utente esegue `./interpreter --bench fib.pcd` e `fib.pcd` e' un p-code valido
- **THEN** l'interprete misura `fib.pcd`

#### Scenario: Misura con il nome corto dell'opzione
- **WHEN** l'utente esegue `./interpreter -b fib.pcd` e `fib.pcd` e' un p-code valido
- **THEN** l'interprete misura `fib.pcd`, come con la grafia `--bench`

#### Scenario: Misura con l'opzione dopo il percorso
- **WHEN** l'utente esegue `./interpreter fib.pcd -b` e `fib.pcd` e' un p-code valido
- **THEN** l'interprete misura `fib.pcd`

#### Scenario: Stessa opzione ripetuta
- **WHEN** l'utente esegue `./interpreter --bench -b fib.pcd`
- **THEN** l'interprete misura `fib.pcd` una sola volta, e non segnala alcun errore per la ripetizione

#### Scenario: Misura di un nome senza estensione
- **WHEN** l'utente esegue `./interpreter -b fib` e il file `fib.pcd` esiste
- **THEN** l'interprete misura `fib.pcd`, non un file di nome `fib`

#### Scenario: Misura di un percorso con punto nella directory
- **WHEN** l'utente esegue `./interpreter -b dir.d/x` e il file `dir.d/x.pcd` esiste
- **THEN** l'interprete misura `dir.d/x.pcd`, considerando il punto della directory come parte della directory

#### Scenario: Misura di un p-code prodotto dal compilatore
- **WHEN** il compilatore traduce un sorgente in una sottodirectory, per esempio `tmp/sub/mio.cpl` in `tmp/sub/mio.pcd`, e l'utente esegue `./interpreter -b tmp/sub/mio`
- **THEN** l'interprete misura `tmp/sub/mio.pcd`, il file appena prodotto dal compilatore nella sottodirectory

#### Scenario: La tabella di misura resta su stdout
- **WHEN** la misura di `./interpreter -b primo.pcd` termina
- **THEN** l'output del programma misurato e la tabella dei tempi compaiono sullo standard output, insieme al rapporto del GC che segue come nelle esecuzioni normali

### Requirement: Esito del caricamento in modalita' bench
In modalita' bench il codice di uscita SHALL dipendere dal caricamento del file e non
dalla misura: se il file e' caricabile l'interprete esegue la misura e termina con codice
0, se non e' caricabile SHALL riportare su stderr la stessa diagnostica della modalita'
normale, nominando il percorso derivato, SHALL NOT eseguire alcuna misura e SHALL terminare
con codice di uscita 1.

#### Scenario: File inesistente
- **WHEN** l'utente esegue `./interpreter -b non_esiste`
- **THEN** su stderr compare il messaggio che indica che `non_esiste.pcd` non puo' essere aperto, non viene prodotta alcuna tabella di misura e il codice di uscita e' 1

#### Scenario: File che non e' un p-code
- **WHEN** l'utente esegue `./interpreter -b primo.cpl`
- **THEN** su stderr compare il messaggio che indica che il file non e' un p-code compilato, non viene prodotta alcuna tabella di misura e il codice di uscita e' 1

#### Scenario: File caricabile
- **WHEN** l'utente esegue `./interpreter -b primo.pcd` e il file e' un p-code valido
- **THEN** su stdout compare la tabella di misura e il codice di uscita e' 0

#### Scenario: La diagnostica non finisce su stdout
- **WHEN** l'utente esegue `./interpreter -b non_esiste` e ridirige stderr su `/dev/null`
- **THEN** lo standard output non contiene la diagnostica e non contiene alcuna tabella di misura
