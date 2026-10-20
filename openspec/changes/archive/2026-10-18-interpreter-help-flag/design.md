# Design

## Context

Vedi `proposal.md` per la motivazione. Lo stato attuale che condiziona l'approccio e' questo.

`cli.c` ( incluso in `parser.c` e in `interpreter.cpp`, valido in C e in C++) conosce la **forma**
di un'opzione, non il vocabolario di nessuno dei due strumenti: `cliTrova()` confronta il token
contro i campi `lungo`, `corto` e `alias` della tabella che ogni `main()` scrive, e `cliOpzioni()`
stampa le righe dell'uso **dagli stessi campi** (D8, gia' scritto sopra la funzione). Per questo
aggiungere un flag all'interprete non richiede di toccare `cli.c`: la riga mancante e' una riga
di tabella, e la sua riga nell'uso arriva gratis. Il campo `alias` copre la terza forma, una parola
intera preceduta da un solo trattino, che non e' ne' nome lungo ne' nome corto: e' la forma che
oggi regge `-bench` ed e' l'unico utente di quel campo in tutto il progetto.

La tabella dell'interprete (`interpreter.cpp`, `tabOpzioni[1]`) ha oggi una sola riga, `-bench`
con le tre grafie `--bench`, `-b`, `-bench`, e un commento `D7` sopra che spiega perche' la terza
esiste. La sua variabile `dest` e' `benchOpzione`, dichiarata `static` fuori da `main()` perche' il
campo `dest` deve puntare a qualcosa che sopravviva alla chiamata.

`main()` fa oggi **un controllo solo**, `if (sc.errore!=CLI_ERRORE_NESSUNO||nPercorsi!=1)`: gli
errori della scansione (opzione sconosciuta, valore mancante, valore non accettato) e la
condizione "non c'e' un solo percorso" condividono lo stesso ramo, e quel ramo chiama `uso()` e
restituisce 1. `uso()` scrive su `cerr` e poi su `stderr` (tramite `cliOpzioni(...,stderr)`), quindi
il testo dell'uso e' oggi **solo** un messaggio d'errore.

Un limite dello strumento che scrive questa change: in un requisito `MODIFIED` di una spec
principale, `openspec validate` **rifiuta di rinominare uno scenario**, perche' uno scenario e'
identificato dal titolo e il blocco modificato deve contenere per intero i titoli di quelli
esistenti. Il delta di questa change deve quindi correggere i **corpi** degli scenari che nominano
`-bench` e non i loro titoli, anche quando il titolo e' ormai falso. Lo stesso vale per il titolo
del requisito "Misura di un p-code con il flag `-bench`", che resta e viene aggiornato solo nel
corpo: e' il comportamento che ha gia' scelto `shared-cli-flags`, che ha portato le grafie da una
a tre lasciando il titolo com'e'.

Vincoli da `AGENTS.md`: nessun CI e nessun runner di test (la verifica e' `./compiler x.cpl` +
`./asm < x.pcd` + esecuzione), nessun formattatore, commenti e note in italiano, nessun
`git add -A` e nessuna rigenerazione della grammatica.

## Goals / Non-Goals

**Goals:**
- Un solo file da toccare per il comportamento: `interpreter.cpp` (piu' i commenti di `cli.c` e i
  documenti).
- Un vocabolario delle opzioni con due opzioni e due grafie per opzione: `-h`/`--help` e
  `-b`/`--bench`, senza forme che imitano quelle standard senza esserlo.
- Un solo testo di uso, stampato su due stream con due codici di uscita diversi, invece di due
  copie del testo da tenere allineate.
- `main()` con una sequenza di controlli leggibile, dove la precedenza fra errore, help e numero
  di percorsi e' esplicita nell'ordine delle righe.
- Il ciclo di scansione in `cli.c` intatto: resta generico sulle forme, perche' la forma `alias`
  continua a essere un caso legittimo di questo modulo anche se nessun binario la usa.

**Non-Goals:**
- `-h`/`--help` sul compilatore: `compiler-cli` deve continuare a descrivere `-h` come opzione
  sconosciuta, e la sua tabella vuota e' una scelta, non un buco (vedi proposal.md).
- L'alias `-help` con un solo trattino, che `-bench` aveva: l'help si dichiara con `-h` e
  `--help`.
- **Togliere il supporto all'alias da `cli.c`**, e rifare `cli_test.c` di conseguenza. Vedi D8.
- Riscrivere il testo delle istruzioni d'uso, aggiungere un `--version`, o un `-` che legga da
  stdin.
- Correggere il codice di uscita di `-bench` (`benchPcd()` restituisce l'esito inverso di
  `runPcd()`): difetto noto, change separata.
- Un test runner. Il progetto non ne ha uno: la verifica di questa change e' esecuzione dei
  comandi e osservazione di stream e codice di ritorno.

## Decisions

### D1: la precedenza fra errore, help e numero di percorsi

`main()` passa da un `if` a tre controlli in sequenza:

1. `sc.errore!=CLI_ERRORE_NESSUNO` → messaggio dell'errore, `uso(stderr)`, `return 1`.
2. variabile dell'help impostata → `uso(stdout)`, `return 0`.
3. `nPercorsi!=1` → `uso(stderr)`, `return 1`.

Alternativa considerata: mettere il controllo dell'help **primo**, davanti a tutto, cosi'
`-h` vince su qualunque altra cosa (`./interpreter -h -z` stamperebbe l'help ed uscirebbe 0, e
`-z` non verrebbe mai segnalato). Semplice da scrivere e coerente con "l'help vince sul
percorso", ma nasconde un errore di sintassi della riga: un utente che scrive `-z` per sbaglio
conclude che l'opzione esista. **Scelta: gli errori vincono.** Il numero di percorsi cede invece
all'help, perche' in quel caso l'utente ha chiesto l'help e non voleva eseguire nulla: e' la
decisione presa con l'utente.

Il costo e' che il ramo d'errore non e' piu' condiviso con quello dei percorsi, quindi il `switch`
sui `CLI_ERRORE_*` va tenuto intatto nel primo controllo e il terzo diventa un `if` separato con
la stessa `uso(stderr)` e lo stesso `return 1`.

### D2: `uso()` prende il `FILE*` invece di scrivere su `cerr`

`uso(FILE* out)` scrive la riga `uso: interpreter [opzioni] <file.pcd>` con `fprintf(out,...)`
invece che con `cerr <<`, e passa `out` a `cliOpzioni()`, che ha gia' un parametro `FILE* out`
(D8). Le due righe finali ("un nome senza estensione viene completato con .pcd", "dopo `--` ogni
argomento e' un percorso") seguono lo stesso stream.

Alternativa considerata: tenere `uso()` su `cerr` e aggiungare una `usoHelp()` su `cout` che
duplica le cinque righe. Rifiutata: sono due copie di un testo che cambia per meta' delle change
di CLI, e il giorno in cui l'uso guadagna un'opzione le due copie divergono.

Nota: `cli.c` resta in C puro e `fprintf` su `FILE*` e' gia' quello che usa `cliOpzioni()`,
quindi il parametro non crea problemi in `interpreter.cpp`, che e' C++ ma include `cli.c` senza
modifiche.

### D3: una riga in tabella, con `lungo="help"` e `corto='h'`, senza alias

La riga e' `{"help",'h',NULL,"stampa queste istruzioni d'uso",CLI_OPZIONE,&helpOpzione}`.

`alias` resta `NULL`: l'utente ha scelto `-h` e `--help`, e `-help` non e' richiesto. Con
`alias=NULL` `cliOpzioni()` stampa gia' `--help, -h`, quindi non serve nessuna riga di testo
nuova e nessuna modifica a `cli.c`. La descrizione e' l'unica cosa nuova da scrivere, ed e' un
testo per l'utente, non codice.

`dest` punta a una `static int helpOpzione=0` fuori da `main()`, come `benchOpzione`: e' un
interruttore (`CLI_OPZIONE`), quindi `cliScansiona()` scrive `1` e non un valore, e la variabile
non ha bisogno di essere inizializzata a mano ma viene comunque azzerata all'inizio di `main()`
perche' `main()` puo' essere chiamato una volta sola e l'azzeramento costa niente ed e' in
linea con `benchOpzione=0`.

Alternativa considerata: riusare un solo interruttore per help e bench con una tabella a due
destinazioni diverse per la stessa opzione. Rifiutata: sono due opzioni distinte con due
significati distinti, e un solo bit non basta.

### D4: dopo `-h` non si entra in nessun ramo che tocchi il filesystem

Il ramo dell'help sta **prima** del controllo sul numero di percorsi e **prima** di
`pcodePathFor()`, quindi `-h fib.pcd` non deriva `fib.pcd`, non chiama `loadPcd()`, non chiama
`runPcd()` ne' `benchPcd()`, e non stampa il rapporto del GC che `main()` stampa a fine
esecuzione. Non serve un flag "gia' stampato" ne' una copia di `main()`: l'help e' un `return`
primo.

### D5: le due righe finali dell'uso stampate anche dall'help esplicito

`cliOpzioni()` genera solo le righe delle opzioni dichiarate; le due righe sulla derivazione
`.pcd` e sul terminatore `--` sono scritte a mano in `uso()`. Il testo non cambia, cambia solo
lo stream, quindi passano da sole senza codice. Vanno tenute anche nell'help esplicito: sono la
risposta alla domanda "come scrivo il nome del file", cioe' la stessa domanda a cui `-h` risponde.

### D6: la verifica resta manuale, con i comandi della spec

Non c'e' un runner (`AGENTS.md`). Ogni scenario della spec si verifica con `./interpreter ...` e
l'osservazione di stream e codice di uscita. Per gli scenari che non devono eseguire nulla si
usa `primo.pcd` e non `fib.pcd`: il bench di `fib(34)` costa 30-60s, mentre `primo.pcd` dimostra
lo stesso contratto in pochi millisecondi. Le fixture di `test/` non si usano: abortiscono con
`not impl!` (rc 134) gia' in modalita' normale, quindi produrrebbero rumore invece di una
distinzione fra i due codici di uscita.

`cli_test.c` non va esteso: la tabella di prova dichiara gia' un flag con le tre forme e
un'opzione con valore, quindi copre tutto quello che `cliScansiona()` sa riconoscere, e nessuna
delle due modifiche aggiunge una forma nuova. Extenderlo significherebbe riscrivere la tabella di
prova per coprire un caso che il modulo non sa ancora fare (un'opzione con valore su uno strumento
che non ne ha).

### D7: `-bench` si rimuove mettendo `alias` a `NULL`, non togliendo il campo

La riga della misura diventa `{"bench",'b',NULL,"misura il file invece di eseguirlo una volta",CLI_OPZIONE,&benchOpzione}`:
un nome lungo, un nome corto, nessun alias. `-bench` smette di essere riconosciuto e il ramo
`CLI_ERRORE_SCONOSCIUTA` lo tratta come `-z`, senza codice nuovo: il messaggio, l'uso su stderr e
l'uscita 1 sono gia' quelli descritti dalla spec.

Alternativa considerata: **lasciare `alias="bench"` in tabella** e limitarsi a non stamparlo
nell'uso, cosi' gli script con `-bench` continuano a funzionare. Rifiutata: l'utente ha deciso
che le grafie sono due, e un alias invisibile nell'uso e' il peggiore dei due mondi — la forma
`-bench` resta accettata ma non e' documentata da nessuna parte, e il prossimo maintainer non sa
se debba toglierla. Meglio un errore esplicito, che si puo' citare in un bug report.

Alternativa considerata: tenere `-bench` come opzione deprecata con un avviso. Rifiutata per lo
stesso motivo e perche' il progetto non ha un meccanismo di avvisi.

Il commento `D7` sopra la tabella, che oggi elenca tre grafie e spiega il ruolo dell'alias, va
riscritto: dopo il cambiamento dice una cosa falsa.

### D8: `cli.c` perde l'alias solo nei commenti, non nel codice

`cliTrova()` confronta l'alias solo per i token con un solo trattino e `cliOpzioni()` stampa
l'alias quando c'e'. Dopo D7 nessun binario lo usa, ma il modulo resta quello che e': conosce le
**forme** delle opzioni, e `alias` e' una forma legittima (una parola intera con un solo trattino,
che ne' `getopt` ne' `argparse` prevedono). Cancellarlo significherebbe togliere al modulo una
forma perche' uno dei due strumenti non la usa, e cancellare anche il suo unico test
(`cli_test.c` verifica la forma alias con la propria tabella, e continuerebbe a verificarlo senza
l'interprete).

Cambiano solo i due commenti che usano `-bench` come esempio di alias — quello dentro
`cliTrova()` e quello nella testata del file — perche' diventano fuorivianti: `-bench` non e' piu'
un alias dichiarato da nessuno. Un esempio neutro (`-verbose`, una parola che non somiglia a nessun
flag di questo progetto) evita che il commento torni a mentire alla prossima modifica.

Se un domani servisse davvero un alias in un binario, la riga tornerebbe in tabella senza toccare
`cli.c`: e' la prova che il modulo sta dalla parte giusta.

### D9: nel delta della spec si correggono i corpi degli scenari, non i titoli

`openspec validate` tratta uno scenario come identificato dal **titolo**: rinominarlo dentro un
requisito `MODIFIED` fa fallire la validazione ("omits scenario(s) the current spec still has"),
perche' l'archivio sostituisce il blocco per intero e si rifiuta di far perdere uno scenario. In
pratica la stessa cosa vale per i titoli dei requisiti: rinominarli richiederebbe una sezione
`## RENAMED Requirements` che questo progetto non ha mai usato e il cui ordine di applicazione
rispetto a un `MODIFIED` non e' verificabile qui, perche' `openspec archive` non ha un dry-run.

Quindi i titoli che nominano la grafia eliminata restano, e i corpi dicono la verita':

- "Flag `-bench` senza percorso" e "Flag `-bench` con piu' percorsi" descrivono `./interpreter -b`
  e `./interpreter -b primo.pcd fib.pcd`: il titolo e' un'etichetta storica, il corpo e' il
  contratto.
- "L'uso nomina le tre grafie dell'opzione di misura" dice nell'uso `--bench` e `-b`, e che `-bench`
  non e' riportata come terza grafia accettata.
- "Misura di un p-code con il flag `-bench`" resta il titolo del requisito, come ha gia' fatto
  `shared-cli-flags` quando le grafie sono passate da una a tre.

Alternativa considerata: `REMOVED` + `ADDED` del requisito, che avrebbe permesso titoli puliti.
Rifiutata perche' fa perdere l'identita' del requisito — che il corpo del delta e le altre
specifiche citano per nome — per un motivo puramente cosmetico, e perche' mostrerebbe in un colpo
solo un requisito rimosso e uno aggiunto con lo stesso nome, che e' il tipo di delta che vale la
pena evitare in una spec che si legge a mano.

Se i titoli congelati disturbano, la ripulizia si fa in una change successiva che rimuove e
ricrea i tre requisiti interessati per intero, non in questa.

## Risks / Trade-offs

- **Il testo dell'uso compare su due stream diversi** → le due righe `.pcd` e `--` passano
  insieme a tutto il resto perche' `uso()` scrive su un solo `FILE*`; non resta nessuna riga
  scritta a mano su `cerr`. La verifica e' `./interpreter --help 2>/dev/null` che deve mostrare
  l'uso **per intero**, e `./interpreter -z fib.pcd 2>/dev/null` che non deve mostrare nulla.
- **Uno spec o una pagina che dice ancora "`-h` e' un'opzione sconosciuta"** → la spec
  `interpreter-cli` ha uno scenario dedicato (`Argomento che sembra un'opzione`) che la change
  modifica, ma `AGENTS.md` ripete la frase due volte (`"./interpreter -h" says so rather than
  opening a file named -h`) e `todo.md` non ne parla. Il task di allineamento include la
  verifica con `grep -n '\-h' AGENTS.md` che non deve piu' trovare la frase.
- **`compiler -h` resta sconosciuto e qualcuno lo legge come un buco** → e' una scelta, non un
  limite: la sua tabella vuota e' intenzionale e `compiler-cli` lo descrive. Diventa una change
  separata se serve.
- **Precedenza degli errori (D1) non intuitiva per chi scrive `./interpreter -h -z`** → mitigato
  dallo scenario esplicito nella spec ("Opzione sconosciuta insieme all'help"), che fissa il
  comportamento senza lasciarlo alla bisogna.
- **`uso(stderr)` in un ramo che oggi non lo chiama** → oggi i due errori condividono il ramo e
  chiamano `uso()` una volta sola; separarli duplica la chiamata in due punti. Accettato: e' il
  prezzo di D1 e sono due righe, non due copie di un testo.
- **Un titolo di scenario che nomina una grafia eliminata** → e' il prezzo di D9, e vale per tre
  casi: "Flag `-bench` senza percorso", "Flag `-bench` con piu' percorsi" e "L'uso nomina le tre
  grafie dell'opzione di misura". Il corpo di tutti e tre dice il comportamento giusto, quindi il
  contratto non e' ambiguo, ma il titolo richiede un secondo sguardo. Mitigazione: la verifica
  del diff rilegge i tre corpi uno per uno, e `## Workflow follow-up` in `tasks.md` lascia la
  ripulizia completa (rimuovere e ricreare i requisiti) a una change dedicata.
- **Qualcuno ha uno script con `-bench`** → dopo questa change riceve `opzione sconosciuta: -bench`
  e codice 1, non un avviso. E' la scelta dell'utente (rimuovere, non deprecare) e il messaggio e'
  esplicito, quindi la diagnosi e' immediata. `AGENTS.md` e' l'unico posto del progetto che
  documenta `-bench` come forma d'uso e va aggiornato nella stessa change.
- **Il ramo `alias` di `cli.c` non e' piu' raggiunto da nessun binario** → resta perche' il modulo
  conosce le forme delle opzioni e non il vocabolario dei due strumenti (D8), e resta coperto da
  `cli_test.c`, che ha una tabella propria. Se in futuro un binario dichiarera un alias, la riga
  torna in tabella senza modifiche al modulo.
- **Una riga di tabella in meno non e' rilevata da nessun controllo automatico** → non c'e' un
  runner (`AGENTS.md`), quindi la verifica e' il task 3.3 della change: `./interpreter
  -bench primo.pcd` deve uscire con 1 e dire "opzione sconosciuta: -bench", e l'uso deve mostrare
  due righe di opzioni, non tre.