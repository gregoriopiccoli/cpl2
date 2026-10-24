# Design

## Context

Vedi `proposal.md` per la motivazione e le delta spec `interpreter-modules` e
`interpreter-pcodes` per il comportamento richiesto.

Lo stato attuale rilevante:

- `sys` è dichiarata in `interpreter.cpp` con `unordered_map<string,pcodeProgram*> modules` e
  `int loadModule(string name);`, ma il metodo non ha definizione e la mappa non è mai usata.
- `pcodeProgram::loadPcd(fn)` carica un `.pcd`, verifica intestazione `#cpl20.0.1` e terminazione
  con `PCODEEND 0`, e su errore stampa la diagnosi su stderr e restituisce 0 lasciando un
  programma con il solo `PCODEEND`. Non memorizza il percorso da cui è stato caricato.
- `pcodeImport::exec()` (p-code `IMPORT`) valida le N stringhe in cima allo stack, le stampa e le
  rimuove. Non ha accesso al percorso del programma corrente.
- La grammatica emette `LOAD` + `LOADX` per un nome puntato (`a.b`) e `STOREX` per l'assegnamento
  a un nome puntato (`a.b := ...`); `LOADX` e `STOREX` non hanno un caso in `makePCode()` e
  finiscono in `pcodeNotImpl`.
- `interp` esegue un `pcodeProgram&` con un `contextObj*`; `runPCode()` crea un contesto nuovo
  (superlivello di default = builtin) e verifica `sp==-1` al termine. `runPcd()`/`benchPcd()`
  sono gli unici chiamanti di `runPCode()`.
- I contesti sono oggetti GC: `contextObj*` non referenziato e non lockato può essere raccolto.
  `lockgc_ptr` non ha un costruttore di copia sicuro (il distruttore implicito copiato
  sbloccherebbe due volte), quindi non è adatto come *value* di una mappa.

## Goals / Non-Goals

**Goals:**

- Dare una definizione unica di `sys::loadModule` che risolva il nome, carichi il `.pcd`,
  esegua il modulo una sola volta e ne conservi programma e contesto.
- Rendere `IMPORT` un vero comando di inclusione, mantenendo la validazione delle N stringhe e
  la pulizia dello stack, e collegare il contesto del modulo all'importatore con un simbolo
  esplicito.
- Rendere utilizzabili i nomi del modulo attraverso quel simbolo, implementando `LOADX` e
  `STOREX`.
- Rendere il comportamento verificabile con `./runtest` senza modificare il runner.

**Non-Goals:**

- Nessuna esportazione implicita dei nomi del modulo: restano nel contesto del modulo e si
  raggiungono solo attraverso il simbolo del modulo, non con un nome nudo.
- Nessuna ricerca in una lista di directory (include path) o tramite variabili d'ambiente.
- Nessun supporto a `from ... import ...`: `P_FROM` resta non implementato.
- Nessuna modifica al formato `.pcd`, a `pcodes.h`/`pcodes.c`, a `cpl2.gram`, ad `asm.c` o alla
  command line dell'interprete.

## Decisions

### 1. La directory di ricerca è quella del p-code che importa

Il nome puntato diventa un percorso relativo: ogni `.` diventa `/`, si accoda `.pcd`, e il
risultato si cerca nella directory del programma che contiene l'`import`. Per esempio
`dir/mio.pcd` che importa `a.b` cerca `dir/a/b.pcd`; un modulo `dir/x/mod.pcd` che importa `a`
cerca `dir/x/a.pcd`.

La directory del programma corrente è tenuta in `sys` come `string currentDir`. `runPcd()` e
`benchPcd()` la impostano dal percorso passato a `loadPcd()` prima di eseguire; `loadModule()`
la salva, la porta alla directory del modulo caricato, esegue e la ripristina anche in caso di
eccezione (try/catch con ripristino e rethrow).

Alternative considerate:

- *Directory corrente del processo*: rifiutata perché `./interpreter dir/mio.pcd` lanciato da
  un'altra directory cercherebbe i moduli nel posto sbagliato, e perché i test di `runtest`
  girano con cwd nella radice del repo mentre il `.pcd` sta nella directory di appoggio.
- *Passare la directory come secondo argomento di `loadModule`*: più esplicito, ma cambia la
  firma già dichiarata e costringerebbe `pcodeImport::exec` a conoscere il percorso del
  programma; `interp::prg` non lo espone. La variabile in `sys` mantiene la firma e un solo
  punto di verità.
- *Aggiungere il percorso a `pcodeProgram`*: utile in prospettiva, ma non necessario qui e
  tocca l'assegnamento di `pcodeProgram` usato da `procObj::call`; rinviato.

### 2. `loadModule` carica, registra in cache e poi esegue

Sequenza:

1. se il nome è già in `modules`, restituisce successo senza rieseguire;
2. deriva il percorso con un helper `modulePathFor(name, currentDir)`;
3. crea un `pcodeProgram` con `new` e chiama `loadPcd(path)`; se fallisce, distrugge il
   programma e restituisce fallimento (la diagnosi è già su stderr);
4. registra **subito** programma e contesto nella cache;
5. crea `contextObj* ctx = new contextObj()` (superlivello = builtin), lo blocca con `lock()` e
   lo conserva in `moduleContexts[name]`;
6. salva `currentDir`, lo porta alla directory del modulo, esegue con un `interp` dedicato,
   verifica `sp==-1` con `assert` come `runPCode()`, ripristina `currentDir` e restituisce
   successo.
7. nel modulo che ha chiamato la importa viene creata un nuovo simbolo che punta al contesto appena creato.

L'iscrizione in cache prima dell'esecuzione è ciò che rende finiti gli import ciclici: un
modulo che importa sé stesso, o due moduli che si importano a vicenda, trovano la voce già
presente e non rieseguono. Alternative: registrare dopo l'esecuzione (ricorsione infinita) o
usare un insieme separato di "in caricamento" (più stato per lo stesso risultato).

Il contesto è lockato e resta vivo per tutta la durata del processo. Non si usa
`lockgc_ptr<contextObj>` come valore della mappa perché la copia implicita di `lockgc_ptr`
copia il puntatore senza incrementare il lock: il temporaneo distruggerebbe il lock lasciando
la mappa con un puntatore sbloccato. La mappa contiene quindi `contextObj*` grezzi, lockati a
mano; il distruttore del GC li elimina comunque a fine processo. Un `~sys()` distrugge i
`pcodeProgram` accodati in `modules` (non sono oggetti GC), evitando il leak dei loro p-code.

### 3. `IMPORT` valida, carica in ordine e non stampa più

`pcodeImport::exec()` mantiene la validazione esistente (N dall'operando, profondità
sufficiente, N `strObj` in cima) e la esegue **prima** di ogni effetto collaterale. Poi, per
ogni nome dal basso verso la cima del gruppo, chiama `theSys.loadModule(nome)`; se una chiamata
restituisce 0 solleva `domain_error` con il nome del modulo (dopo che `loadPcd` ha già scritto
la causa su stderr), lasciando il programma terminare come per gli altri errori a runtime.
Infine rimuove le N stringhe con `pop_back()` e `sp--`.

Non stampa più i nomi: l'output di un `import` è quello prodotto dal modulo. Il vecchio test
`test/import_stack` che si aspettava i nomi viene riscritto, e il requisito "IMPORT non include
ancora i moduli" viene rimosso dalla delta spec.

Alternativa considerata: stampare i nomi e poi caricare. Rifiutata perché confonderebbe
l'output del programma con una diagnostica temporanea e perché il test dovrebbe conoscere
comunque i moduli.

### 4. Test: modulo reale, cache e contesto separato con i fixture esistenti

Si aggiunge `test/a_module.cpl` (un test completo, con `.out`) che dichiara `var int x:=1` e
stampa `modulo a`. `test/import_stack.cpl` viene riscritto per importare `a_module` due volte,
dichiarare a sua volta `var int x:=2` e stampare `x`; l'output atteso è `modulo a` seguito da
`2`. Il test verifica insieme: esecuzione del modulo (la riga del modulo compare), cache
(compare una sola volta) e contesto separato (la seconda `var int x` non solleva "name already
in context", cosa che accadrebbe se il modulo fosse eseguito nel contesto dell'importatore).
`test/module_access.cpl` + `.out` importa `a_module`, stampa `a_module.x`, assegna
`a_module.x:=5` e lo stampa di nuovo: verifica il simbolo del modulo e `LOADX`/`STOREX`
sull'output atteso `modulo a`, `x:1`, `x:5`.

Il runner non cambia. `runtest` usa una sola directory di appoggio per l'intero giro e compila
i test completi in ordine di nome: `a_module` (che inizia per `a`) viene compilato prima che
`import_stack` venga eseguito, quindi `a_module.pcd` è già nella directory di appoggio e la
risoluzione relativa al p-code dell'importatore lo trova. È una dipendenza voluta e visibile
nei nomi dei fixture; se in futuro `runtest` isolasse i `.pcd` per test, servirà una copia dei
moduli helper (fuori da questo change).

La mappatura puntata `a.b → a/b.pcd` non è coperta da `runtest`, che copia i sorgenti in modo
piatto: viene verificata manualmente con un modulo in una sottodirectory, e la delta spec la
descrive con scenari propri.

### 5. Il simbolo del modulo e l'accesso con LOADX/STOREX

`IMPORT`, dopo ogni `loadModule` riuscito (anche quando il modulo era già in cache), crea nel
contesto dell'importatore un simbolo con il **nome locale** del modulo, cioè l'ultimo componente
del nome puntato: `q1` per `q1`, `x` per `a.x`. Il valore è il `contextObj*` del modulo già
conservato in `sys::moduleContexts`, il tipo è `theNil`. Un nome puntato completo non è un
identificatore valido per il linguaggio, quindi l'ultimo componente è l'unico nome con cui il
simbolo può essere usato nella sintassi `nome.variabile`.

L'inserimento è **first-wins**: se il contesto dell'importatore ha già un simbolo con quel nome
lo lascia invariato, senza errore. Serve perché un secondo `import` dello stesso nome non deve
fallire, e perché due moduli con lo stesso nome locale (`a.x` e `b.x`) restano un caso definito
invece che un'eccezione a runtime. Per questo `contextObj` acquisisce un piccolo helper
`addIfAbsent(intern,type,value)` che guarda solo il livello locale (non il superlivello), così un
modulo con lo stesso nome di un builtin può comunque essere importato.

`LOADX` (operando stringa, nome internato) prende il contenitore in cima allo stack e lo
sostituisce con il valore che quel contenitore ha per quel nome:
`stack[sp] = stack[sp]->load(intern)`. Funziona perché il simbolo del modulo vale un
`contextObj`, la cui `load()` è già quella usata da `LOAD`; per un contenitore che non sa fare
`load` l'errore esistente ("load not implemented") resta invariato.

`STOREX` (stesso operando) usa il layout `[..., contenitore(sp-1), valore(sp)]`:
`contenitore->store(intern,valore)`, poi lascia il valore in cima come fa `STORE`
(`stack[sp-1]=valore`, `pop_back`, `sp--`). L'assegnamento a un nome inesistente segue la
`store()` del contenitore, quindi un modulo la cui variabile non esiste produce l'errore di nome
non trovato dei builtin.

Nessuno dei due p-code è caldo: non entrano in `pcodejump[]`. Non serve toccare `pcodes.h` né
`pcodes.c`: `P_LOADX` (64) e `P_STOREX` (68) esistono già, manca solo il caso in `makePCode()`.

## Risks / Trade-offs

- [La ricerca relativa al p-code differisce dalla regola CLI relativa alla cwd] → è il
  comportamento specificato per i moduli; `pcodePathFor` resta invariato per il file da
  eseguire.
- [`currentDir` è stato globale dell'interprete] → viene salvato e ripristinato con try/catch;
  gli import avvengono solo a livello superiore del programma o del modulo, non dentro proc in
  esecuzione.
- [Il contesto del modulo resta lockato per sempre] → è voluto: dura quanto il processo, e il
  distruttore del GC lo elimina a fine esecuzione.
- [Nessun nome esportato con un nome nudo: importare un modulo non rende visibili i suoi nomi
  senza il simbolo del modulo] → limite dichiarato; il simbolo del modulo evita la copia e
  lascia aperta l'aggiunta di `from` senza cambiare la cache né la risoluzione dei percorsi.
- [Due moduli con lo stesso nome locale rendono raggiungibile solo il primo] → è la regola
  first-wins, scelta per non far fallire un doppio import; i nomi puntati distinti restano
  caricati e memorizzati anche se il secondo simbolo non viene creato.
- [Un modulo mancante termina il programma con `domain_error` non catturata] → coerente con gli
  altri errori a runtime (`pcode not implemented` compreso); la causa del caricamento è già su
  stderr.
- [Il test di import dipende dall'ordine dei nomi e dalla directory di appoggio condivisa] →
  i nomi dei fixture lo rendono deterministico; due giri consecutivi di `./runtest` devono
  dare lo stesso esito, ed è un passo di verifica esplicito.
- [`~sys()` distrugge i programmi mentre i contesti dei moduli sono ancora vivi nel GC] → la
  distruzione avviene a fine processo, dopo che nessun p-code può più essere eseguito; il GC
  elimina poi i contesti senza usare i programmi.

## Migration Plan

Nessuna migrazione di dati o formati: i `.pcd` esistenti restano validi e non vanno ricompilati.
Rollback: rimuovere la definizione di `loadModule`, i campi aggiunti a `sys` e il collegamento
in `pcodeImport::exec` (compresa la creazione del simbolo), rimuovere `pcodeLoadX`/`pcodeStoreX`
e i loro casi in `makePCode()`, ripristinando stampa e rimozione dei nomi; rimuovere i fixture
nuovi e ripristinare `test/import_stack` e le righe di `AGENTS.md`.
