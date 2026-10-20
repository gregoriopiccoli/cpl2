# Proposal

## Why

L'interprete non ha modo di chiedere le sue istruzioni d'uso: `./interpreter -h` viene
scansionato come opzione, non la trova in tabella e finisce nel ramo "opzione sconosciuta",
cioe' `opzione sconosciuta: -h` piu' l'uso su stderr e codice 1. L'elenco delle opzioni esiste
gia' (`cliOpzioni()` costruisce le righe dalla stessa tabella che usa la scansione) ed e' stampato
in due casi soltanto, entrambi d'errore: senza percorsi, o con piu' di uno. L'unica occasione in
cui l'utente vorrebbe leggerlo di proposito e' proprio quella che oggi produce una segnalazione
d'errore.

Il vocabolario delle opzioni ha anche un eccesso: l'opzione di misura si scrive in **tre** grafie,
`--bench`, `-b` e `-bench`, e la terza non serve a niente. `-bench` non e' un nome lungo (non ha
due trattini) ne' un nome corto (non e' un solo carattere): e' rimasta perche' `main()` confrontava
`argv[1]` con la stringa esatta `-bench` quando non c'era ancora la tabella delle opzioni, e la
change `shared-cli-flags` l'ha dovuta conservare come alias perche' il modulo la tratta come
parola intera preceduta da un solo trattino. Oggi che la tabella c'e', `-bench` e' la terza grafia
di un flag che ha gia' `--bench` e `-b`: costa una riga in tabella, una riga in piu' nell'uso
stampato a ogni `-h`, e un documento (`AGENTS.md`) che la promuove come forma ufficiale. Le
opzioni della CLI dell'interprete sono due, con due grafie ciascuna: `-h`/`--help` e
`-b`/`--bench`.

## What Changes

- **BREAKING** `-h` e `--help` diventano l'opzione di help dell'interprete. `-h` non e' piu'
  un'opzione sconosciuta: la tabella `tabOpzioni` di `interpreter.cpp` cresce da una riga a due e
  `-h` smette di essere segnalata come sconosciuta dalla spec `interpreter-cli`.
- **BREAKING** `./interpreter -h` e `./interpreter --help` stampano le istruzioni d'uso **su
  stdout** e terminano con **codice 0**, senza eseguire ne' misurare alcun programma. Non e' il
  ramo d'errore: su stderr restano le diagnostiche delle command line sbagliate, con codice 1.
  L'help esplicito e l'uso su stderr diventano due uscite distinte dello stesso testo, quindi
  `uso()` si parametrizza sul `FILE*` di destinazione.
- `-h` e `--help` vengono riconosciute in qualunque posizione della riga e valgono anche senza
  percorso. `-h` con un percorso presente (`./interpreter -h fib.pcd`) **stampa l'help e esce
  0**: il percorso viene ignorato, non viene eseguito ne' misurato e non viene derivato in
  `.pcd`.
- `-h` non e' un alias di un nome lungo e non accetta un valore: `./interpreter -h=x` e
  `./interpreter --help=x` restano l'errore "non accetta un valore" con uso su stderr e codice 1,
  come gia' `-b=on`.
- La combinazione con l'opzione di misura (`./interpreter -h -b fib.pcd`) stampa l'help ed esce
  0 senza misurare: l'help e' richiesto esplicitamente e vince su tutto il resto della riga.
- **Decisione, non richiesta esplicitamente**: gli **errori di command line hanno la precedenza
  sull'help**. Il numero di percorsi cede davanti a `-h`, un errore della scansione no: se la riga
  contiene anche `-z` o un `-h=x`, l'interprete segnala l'errore come oggi, con uso su stderr e
  codice 1, e non stampa l'help su stdout. Motivo: l'help e' l'unica cosa che l'utente chiede
  quando lo chiede, ma un'opzione sconosciuta o un valore inaccettabile e' un errore di sintassi
  della riga, che va segnalato e non nascosto. Il prezzo e' una regola in piu' da ricordare,
  `./interpreter -h` resta comunque il caso normale e resta quello che la scelta dell'utente
  descrive.
- Le due righe dell'uso che descrivono la derivazione `.pcd` e il terminatore `--` vengono
  stampate anche nell'help esplicito, perche' `cliOpzioni()` non le genera e l'elenco da solo
  non basta a capire `./interpreter fib`.
- **BREAKING** `-bench` sparisce dall'opzione di misura, che resta dichiarata con le due sole
  grafie `--bench` e `-b`. `alias` diventa `NULL` nella riga di tabella: `./interpreter -bench
  fib.pcd` non e' piu' una forma valida e viene segnalato come **opzione sconosciuta** su stderr
  con l'uso e codice 1, esattamente come `-z`. Non resta un alias silenzioso ne' una
  deprecazione: l'utente ha deciso che le grafie sono due.
- L'uso stampato passa da una riga di opzioni a due: `--bench, -b, misura il file invece di
  eseguirlo una volta` perde `-bench` dalla propria lista e compare `--help, -h, stampa queste
  istruzioni d'uso`. Le righe sono generate da `cliOpzioni()` dagli stessi campi della tabella,
  quindi non serve scrivere nulla a mano.
- Fuori perimetro: la coppia `-h`/`--help` sul **compilatore**. `compiler-cli` continua a
  dichiarare zero opzioni e `./compiler -h fib.cpl` resta "opzione sconosciuta" con codice 1, che
  e' il comportamento che la sua spec descrive oggi. Aggiungerlo e' una change a parte.
- Fuori perimetro: l'alias `-help` con un solo trattino. L'help si dichiara con `-h` e `--help` e
  nient'altro, anche se `-bench` aveva un alias: la forma con un solo trattino e una parola
  intera era un artefatto del confronto con `argv[1]`, non un requisito.
- Fuori perimetro: il testo dell'uso. Cambiano stream e codice di uscita, non le frasi: `uso:`
  resta `uso: interpreter [opzioni] <file.pcd>`.

## Capabilities

### New Capabilities
- Nessuna.

### Modified Capabilities
- `interpreter-cli`: nuovo requisito per l'opzione di help `-h`/`--help` (cosa stampa, su
  quale stream, con quale codice di uscita, e cosa succede quando un percorso e' presente), e
  cinque requisiti esistenti corretti: "Selezione del file da eseguire" (lo scenario che oggi
  fissa `-h` come opzione sconosciuta e i suoi scenari sulla misura senza percorso),
  "Le istruzioni d'uso elencano le opzioni dichiarate" (due grafie per l'opzione di misura, e
  l'help esplicito su stdout che elenca anche se stesso), "Riconoscimento delle opzioni" (le
  grafie lunghe e corte di entrambe le opzioni), "Misura di un p-code con il flag `-bench`" (le
  due grafie e `-bench` come opzione sconosciuta) e "Esito del caricamento in modalita' bench"
  (i suoi scenari passano da `-bench` a `-b`).
- Nessuna capacita' nuova e nessuna capacita' del compilatore toccata.

## Impact

- `interpreter.cpp`: `tabOpzioni` resta a due righe, ma entrambe cambiano. La riga della misura
  perde `alias` (`"bench"` diventa `NULL`) e il commento `D7` che la descrive va riscritto, perche'
  elenca tre grafie e spiega perche' la terza esiste; la riga nuova e' `--help`/`-h`. `uso()`
  prende il `FILE*` su cui scrivere invece di scrivere fisso su `cerr`. `main()` si spezza in tre
  controlli in sequenza: prima quello su `sc.errore` (che chiama `uso(stderr)` e resta identico a
  oggi), poi quello sulla variabile dell'help (che chiama `uso(stdout)` ed esce con 0), infine
  quello sul numero di percorsi. Oggi i due ultimi condividono un unico `if`, quindi lo
  smembramento e' parte della change, non un optional. Nessuna funzione di misura o di esecuzione
  viene toccata: `runPcd()`, `benchPcd()`, `pcodePathFor()` restano quelle di oggi.
- `cli.c`: nessuna modifica di codice. La forma dell'opzione (`lungo`, `corto`, `alias`, tipo) e'
  gia' supportata e `cliOpzioni()` stampa gia' `--help, -h` e `--bench, -b` senza bisogno di
  righe nuove. Cambiano **solo due commenti**, che usano `-bench` come esempio di alias e che
  diventano fuorivanti quando l'alias sparisce: quello dentro `cliTrova()` e quello nella testata
  del file. Il ramo `alias` di `cliTrova()` e la sua stampa in `cliOpzioni()` restano nel codice
  anche se nessun binario li usa piu': `cli.c` conosce la **forma** delle opzioni, non il
  vocabolario di nessuno dei due strumenti, e togliere una forma dal modulo perche' uno dei due
  non la usa e' ridurre il modulo per un motivo che non e' suo.
- `cli_test.c` non cambia: ha una tabella propria che dichiara ancora un flag con alias, quindi
  continua a provare la forma alias senza dipendere dall'interprete, e la sua verifica sulla riga
  `--bench, -b, -bench` resta vera perche' riguarda la sua tabella e non quella di `main()`.
- `parser.c` non cambia: la tabella delle opzioni del compilatore resta intenzionalmente vuota e
  `compiler-cli` resta com'e'.
- `AGENTS.md`: la sezione "Running a program — one path, plus options" va riscritta su tre punti:
  "The only flag is the benchmark one, written `-bench`, `-b` or `--bench`" diventa due opzioni con
  due grafie ciascuna, l'esempio `./interpreter -bench mio.pcd` diventa `./interpreter -b mio.pcd`,
  e la frase "`./interpreter -h` says so rather than opening a file named `-h`" (che descrive `-h`
  come opzione sconosciuta) diventa la descrizione dell'help. Nella sezione "Wiring" la lista
  "`interpreter.cpp` has `--bench`/`-b`/`-bench`" va corretta, e nella sezione dei debug switch il
  paragrafo sul difetto di `benchPcd()` che nomina `-bench` va allineato alla grafia che resta.
- `todo.md`: non ha punti su questo tema; i DA FARE sono 3, 4 e 5, nessuno legato alla command
  line. Nessuna modifica prevista.
- Formato del `.pcd`, header `#cpl20.0.1`, `emit.c`, `pcodes.c`, `asm.c` e la grammatica non sono
  toccati: nessuna rigenerazione di `cpl2.c`/`cpl2.h`, nessun `.pcd` da rifare, nessun p-code
  nuovo.
- Tempi e memoria: nessuna. `-h` non carica nessun p-code e non entra in nanobench, quindi non
  costa i 30-60s del bench e non aggiunge un caso al test runner che non esiste. La rimozione di
  `-bench` non cambia il costo di `--bench` e `-b`.