# Design

## Context

Vedi `proposal.md` per la motivazione. Qui i fatti del codice che condizionano
l'approccio.

`parser.c` e' l'unica translation unit del compilatore: include `pcodes.c`, poi
`emit.c`, poi il `cpl2.c` generato da lemon. Le azioni della grammatica scrivono il
p-code chiamando `emit()`, `emit_i()`, `emit_s()`, `emit_line()` e `emit_ver()`, che oggi
scrivono tutte con `printf` su stdout. Non esiste alcun modo di redirigere l'output.

`parse(FILE* f)` ha gia' la firma giusta ma presenta tre difetti che la rendono
inutilizzabile con un file reale:

1. il loop usa `feof(stdin)` invece di `feof(f)`;
2. la prima `fgets` non controlla il valore di ritorno, quindi con un file vuoto `line`
   resta non inizializzata e `initScannerLine()` la tratta come stringa;
3. le diagnostiche (`printf` in `tokenize()` e in `%syntax_error`) vanno su stdout e
   finiscono dentro il p-code.

`emit.c` e' condiviso anche con `asm.c`, che lo include a sua volta e scrive su stdout:
qualsiasi modifica a `emit.c` deve lasciare `./asm` invariato.

`%syntax_error` vive dentro `cpl2.gram`, quindi ogni modifica al suo testo obbliga a
rigenerare `cpl2.c` e `cpl2.h`. Il rumore di template (~300 righe) prodotto dalla versione
di lemon installata e' atteso e va committato.

Contratto del formato `.pcd`, verificato da `pcodeProgram::loadPcd` in
`interpreter.cpp`: header esattamente `#cpl20.0.1` in prima riga e ultimo p-code
`PCODEEND` con operando `0`. La grammatica emette gia' `emit_i(P_PCODEEND, errs)`, quindi
un file con errori produce `PCODEEND` diverso da zero e l'interprete lo rifiuta correttamente.

## Goals / Non-Goals

**Goals:**
- Riutilizzare `parse()` esistente senza riscriverla: cambiare il minimo necessario per
  poterla alimentare con un `FILE*` e scrivere su un `FILE*` di uscita.
- Non introdurre dipendenze esterne ne' un parser di command line: il progetto e' C puro
  senza librerie oltre lo standard.
- Lasciare `./asm` byte-identico nel comportamento.
- Non cambiare il formato del `.pcd` ne' l'interprete.

**Non-Goals:**
- Nessun flag di opzione: nessun `-o`, `-d`, `-c`, nessun `--help`, nessuna gestione di
  opzioni che inizino con `-`. Un nome che inizia con `-` e' trattato come un percorso.
- Nessun `-` come sinonimo esplicito di stdin.
- Nessun supporto a include/import: e' una questione separata, e `import` non e' neppure
  implementato nell'interprete.
- Nessuna modifica all'interprete. Ad `asm.c` si tocca **una sola riga**
  (`emit_out=stdout;` in cima al `main()`), resa necessaria dalla D1: il suo comportamento
  resta identico, `cpl2asm()` non cambia e le pipe esistenti continuano a funzionare.
- Secondo punto di `todo.md` (file in command line per l'interprete): resta aperto, e
  comunque richiederebbe prima di toccare il `main()` hardcoded dell'interprete.

## Decisions

### D1. Un puntatore di uscita globale in `emit.c`, impostato dai `main()`
`emit.c` ottiene un globale `FILE* emit_out;` e tutte le funzioni di emissione passano
da `printf` a `fprintf(emit_out, ...)`. Il puntatore **non** puo' essere inizializzato a
scope di file con `stdout`: in ISO C `stdout` e' un oggetto (`extern FILE *stdout`), non
un'espressione costante, e `cc` (C, non C++) rifiuta l'inizializzatore:

```
emit.c:7:16: error: initializer element is not constant
    7 | FILE* emit_out=stdout;
```

L'assegnazione avviene quindi all'avvio di ciascun programma che include `emit.c`:
`emit_out=stdout;` in cima al `main()` di `parser.c` (nel ramo `argc < 2`) e in cima al
`main()` di `asm.c`. Il globale parte a `NULL` (inizializzazione implicita dei globali a
durata di storage statica) e nessuna emissione avviene prima che il `main()` lo abbia
impostato.

Perche' un globale e non un parametro: le funzioni sono chiamate da ~100 azioni della
grammatica, tutte con firme fissate da lemon (`%type` dichiara solo un `int`). Passare
il `FILE*` richiederebbe di aggiungerlo a ogni `%type` o di sporcare `%extra_argument`,
che pero' oggi contiene `parserState *sState` ed e' il posto naturale per lo stato di
compilazione. Un globale e' l'opzione meno invasiva e resta coerente con lo stile
esistente, dove `emit_asm` e' gia' un globale analogo che smista lo stesso flusso.

L'impostazione di `emit_out` in ogni entry point e' obbligatoria, non opzionale: `emit.c`
e' compilato anche dentro `asm.c`, che emette su stdout e non ha bisogno del ramo multi-file.
Senza l'assegnazione in `main()` `asm.c` avrebbe comportamento indefinito.

Alternativa scartata: `freopen("a.pcd","w",stdout)` per ogni file. Sembra piu' rapida da
scrivere, ma redirige un flusso di processo: in modalita' file lascerebbe stdout
irraggiungibile per il resto del programma ed e' fragile con piu' file.

Alternativa scartata: inizializzazione pigra con un accessor (`emitStream()` che cade su
`stdout` quando `emit_out` e' `NULL`). Evita la riga nei due `main()`, ma introduce una
funzione e un ramo in ogni emissione, e sposta il punto di garanzia dentro `emit.c`
invece che all'avvio del programma.

### D2. Il nome del file di output lo calcola `main()`, non `parse()`
`parse()` resta focalizzata su "leggi questo `FILE*`, produci questo p-code". Il
`FILE*` di uscita viene impostato dal chiamante prima dell'invocazione.

Perche': il nome dell'output dipende dal percorso passato in command line, che `parse()`
non conosce. Derivarlo dentro `parse()` richiederebbe di passare il percorso come
parametro aggiuntivo solo per ricavarne un nome che il chiamante deve comunque conoscere
per gestire `fopen` ed errori.

### D3. Un `srcFile` globale per il prefisso delle diagnostiche
Il nome del sorgente corrente vive in un globale `const char* srcFile`, inizializzato a
`NULL`. Tutte le diagnostiche passano da un helper unico in `parser.c`:

```c
void diag(const char* fmt, ...);   // scrive su stderr, antepone "srcFile: " se srcFile != NULL
```

Perche' un helper invece di ripetere il prefisso: le diagnostiche sono emesse da tre
punti distanti e in momenti diversi della compilazione — `tokenize()` (stringa non
terminata, numero mal terminato), l'azione `%syntax_error` della grammatica, e il
nuovo codice di gestione dei file in `main()`. Un helper unico garantisce che le tre
famiglie restino coerenti e da' un posto unico dove cambiare formato in futuro.

`diag` deve essere definito prima di `tokenize()` e prima di `#include "cpl2.c"`, quindi
in cima a `parser.c`: e' una sola translation unit, l'ordine di dichiarazione basta.

Alternativa scartata: mettere il prefisso in `parserState`. Non funziona per `tokenize()`,
che riceve solo `scannerStatus*` e non ha accesso a `sState`.

### D4. Una sola `emit_ver("0.0.1")` per file, dentro `parse()`
`emit_ver()` resta dove e', all'inizio di `parse()`, ma `main()` imposta `emit_out` e
`srcFile` *prima* di chiamare `parse()`. Ogni invocazione di `parse()` produce quindi un
programma completo: header, corpo, `PCODEEND`. Non serve nessun header aggiuntivo.

La stringa di versione resta hardcoded in un solo posto, come richiede `AGENTS.md`, che
richiede di bumpare in modo coordinato il valore qui e in `pcodeProgram::loadPcd`.

### D5. La modalita' standard input non viene toccata
`main()` con `argc < 2` esegue esattamente il percorso di oggi: `srcFile = NULL`,
`emit_out = stdout`, `parse(stdin)`, `return parse(...)`.

La conseguenza e' voluta e accettata: in questa modalita' le diagnostiche escono senza
prefisso e su stderr invece che su stdout. Cambiare stream e' una conseguenza del passaggio
alle diagnostiche su stderr, che e' la scelta giusta perche' i file p-code non devono
contenere testo di errore. Il testo dei messaggi resta identico, quindi chi legge le
diagnostiche non perde nulla; cambia solo il flusso su cui arrivano.

### D6. Il sentinella `eof`/`EOF` resta, applicato per file
Il loop attuale termina anche quando la riga letta e'`eof\n` o `EOF\n`. Oggi questo e'
osservabile per qualsiasi sorgente, perche' anche i file arrivano da stdin. Il codice
verra' applicato per file senza modifiche, cosi' il comportamento di quel caso limite non
cambia: un sorgente che contiene una riga `eof` viene troncato esattamente come oggi.

Alternativa scartata: rimuovere il sentinella in modalita' file, perche' troncare un file
"normale" e' surprising. Sarebbe pero' una modifica di comportamento non richiesta e non
coperta dalla spec; e' una modifica separata, da fare insieme a una regola chiara su
come terminare l'input in modo esplicito.

### D7. Il conteggio errori si somma in `main()`
`parse()` continua a restituire il numero di errori del singolo file (`sState.errors`).
`main()` li somma su tutti i file, e aggiunge 1 per ogni file non apribile o non
scrivibile. Il contratto "exit code == numero di errori" resta quindi vero, ma da
per-stream diventa per-invocazione.

## Risks / Trade-offs

- **La rigenerazione della grammatica produce ~300 righe di rumore non correlate.**
  → Il `Makefile` ha gia' la regola `cpl2.c: cpl2.gram`; si esegue `make cpl2.c` e si
  committano `cpl2.c` e `cpl2.h` insieme alla modifica di `cpl2.gram`. Il rumore e'
  documentato in `AGENTS.md`. In task: verificare che `lemon` esca 0 e che il diff della
  grammatica sia di una sola riga.

- **L'emissione su un globale puo' essere dimenticata in un punto futuro.** Una nuova
  funzione in `emit.c` che usa `printf` invece di `fprintf(emit_out, ...)` romperebbe
  in silenzio solo la modalita' file.
  → In task: verificare con `grep` che in `emit.c` non resti alcuna `printf`.

- **`initpcodetxt()` viene chiamata una volta per file.** Oggi e' gia' cosi' perche' e'
  dentro `parse()`, ed e' idempotente (riempie un array fisso), quindi il costo e' trascurabile.
  → Nessuna azione, ma da sapere se in futuro `initpcodetxt()` diventasse non idempotente.

- **`fgets` con buffer da 2000 caratteri tronca le righe piu' lunghi.** Difetto gia'
  presente, non introdotto qui; i file in command line non lo peggiorano.
  → Fuori perimetro, da non correggere insieme a questa change per non mescolare due
  modifiche di formato del file di input.

- **Sovrascrittura silenziosa: `compiler fib.cpl` scrive `fib.pcd` senza avvisare se il
  file esiste gia'.**
  → Accettato per ora: e' il comportamento atteso da un compilatore e l'opzione `-o`,
  deliberatamente esclusa, e' quella che lo renderebbe esplicito.

- **`diag()` in un progetto C senza `<stdarg.h>`**: va incluso. Poco rischioso ma facile
  da dimenticare in fase di implementazione.
  → Esplicitato nei task.

## Migration Plan

Non c'e' migrazione di dati ne' di formato. La sola cosa che cambia per chi usa gli
strumenti e' il modo di invocare il compilatore in modo file:

- `compiler fib.cpl > fib.pcd` continua a funzionare: la redirezione crea e tronca
  `fib.pcd` prima che il compilatore lo apra in scrittura, quindi l'apertura successiva
  scrive su un file gia' vuoto e il risultato e' corretto. La redirezione e' solo
  diventata inutile. Da consigliare comunque `compiler fib.cpl` senza redirezione, per non
  avere due scritture sullo stesso file nella stessa esecuzione.

Rollback: il cambio e' confinato a `parser.c`, `emit.c` e una riga di `cpl2.gram`. Per
tornare indietro basta ripristinare quei tre file e rimuovere i `.pcd` generati. Non
c'e' nessun dato persistito dal nuovo formato.

## Open Questions

- Il sentinella `eof`/`EOF` (D6) resta un comportamento strano per un file. Una volta
  questa change applicata, ha senso chiedersi se l'input da file debba avere un modo
  esplicito di fine, e se il sentinella debba valere solo per stdin. Non blocca questa
  change: non ne cambia specs, approccio ne task.
- Se in futuro si vorrebbe `-` come sinonimo di stdin, o `-o`, la D1 (globale in
  `emit.c`) resta valida: sono ortogonali.