# Tasks

Directory di scratch per tutte le verifiche: `/tmp/opencode/cpl2-cli-check` (il progetto ha
`.gitignore` con una sola riga, quindi i `.pcd` prodotti in verifica non devono accumularsi
nella root del repository). Il compilatore in uso e' quello della root: `make compiler`.

## 1. Regola «il punto e' nel nome di base» in un posto solo

- [x] 1.1 Prima di modificare `parser.c`, salvare il riferimento: `mkdir -p
      /tmp/opencode/cpl2-cli-check/baseline`, poi `cp fib.pcd primo.pcd
      /tmp/opencode/cpl2-cli-check/baseline/` e `cp test/simple_class.pcd
      /tmp/opencode/cpl2-cli-check/baseline/simple_class.pcd` (i `.pcd` gia' presenti nella
      root, prodotti dal binario attuale).
      Verifica: `ls /tmp/opencode/cpl2-cli-check/baseline` elenca i tre file.
- [x] 1.2 Estrarre in `parser.c` l'helper `const char* baseDot(const char* path)` che
      restituisce l'ultimo `.` del solo nome di base, `NULL` se il nome di base non ne contiene,
      senza considerare la directory (D5). Commento obbligatorio: `dir.d/x` non ha
      estensione.
      Verifica: `make compiler` compila senza warning e il file contiene l'helper una volta
      sola.
- [x] 1.3 Riscrivere `pcodePath()` usando `baseDot()` al posto dei tre calcoli inline
      (`slash`, `base`, `dot`), lasciando invariati troncamento e `".pcd"` finale. Non cambiare
      la firma ne' l'ordine delle operazioni sul buffer `out`.
      Verifica: in `/tmp/opencode/cpl2-cli-check/matrix` preparare `a.cpl`, `b.txt`, `noext` e
      `dir.d/x.cpl`, poi eseguire `./compiler a.cpl b.txt noext dir.d/x.cpl` da quella directory
      e controllare che i `.pcd` siano `a.pcd`, `b.pcd`, `noext.pcd`, `dir.d/x.pcd` — in
      particolare che `dir.d/x.pcd` sia dentro `dir.d/` e non nella directory corrente.
- [x] 1.4 Verificare che `pcodePath()` produca output identici a prima della modifica: da
      `/tmp/opencode/cpl2-cli-check/baseline` non basta il nome, quindi rieseguire
      `./compiler fib.cpl primo.cpl test/simple_class.cpl` nella root del progetto e confrontare
      `cmp fib.pcd baseline/fib.pcd`, `cmp primo.pcd baseline/primo.pcd` e
      `cmp test/simple_class.pcd baseline/simple_class.pcd`.
      Verifica: i tre `cmp` non riportano differenze.

## 2. Risoluzione del sorgente in command line

- [x] 2.1 In `parser.c` implementare `FILE* openSource(const char* arg, const char** resolved)`
      (D1): `fopen(arg,"r")` e, se riesce, `*resolved=arg` e ritorno del `FILE*`; altrimenti
      salva `errno` in una variabile locale **prima** di ogni tentativo successivo (D3), e se
      `errno==ENOENT` e `baseDot(arg)==NULL` costruisci il candidato `snprintf(cand,
      sizeof(cand), "%s.cpl", arg)` in un buffer `static char cand[2000]` dichiarato nel
      corpo dell'helper (D6), provando `fopen(cand,"r")`: se si apre, `*resolved=cand`.
      Il vincolo di durata del puntatore restituito (valido almeno fino alla chiamata
      successiva, perche' alimenta `srcFile` e `pcodePath()`) va scritto nel commento.
      Verifica: `make compiler` compila senza warning; il buffer e' `static` e non in pilota.
- [x] 2.2 Nel ramo `argc >= 2` di `main()` sostituire la sequenza attuale
      `pcodePath(argv[i],...)` / `srcFile=argv[i]` / `fopen(argv[i],...)` con: chiamare
      `openSource(argv[i], &resolved)`; se il `FILE*` e' `NULL` emettere **una sola**
      diagnostica con `diag()` che nomini i percorsi tentati (D7), incrementare `errors`,
      saltare al file successivo; altrimenti impostare `srcFile = resolved`, chiamare
      `pcodePath(resolved, pcdPath)` **dopo** la risoluzione (D4), aprire il `.pcd`, `parse()` e
      chiudere. Il ramo `argc < 2` non si tocca.
      Verifica: `make compiler` compila; `grep -n "argv\[i\]" parser.c` non mostra piu' nessun
      uso di `argv[i]` come sorgente o come `srcFile` nel ramo multi-file.
- [x] 2.3 Verificare gli scenari centrali della spec in `/tmp/opencode/cpl2-cli-check/solo`
      (che contiene solo `fib.cpl`): `./compiler fib` deve tradurre `fib.cpl` e scrivere
      `fib.pcd` lì, con codice di uscita 0 e stdout vuoto.
      Verifica: `ls` mostra `fib.pcd`; `./asm < fib.pcd` mostra il listing del programma e non
      contiene `has errors`; il codice di uscita e' 0.
- [x] 2.4 Verificare la precedenza del percorso digitato in
      `/tmp/opencode/cpl2-cli-check/entrambi`, che contiene un `fib` e un `fib.cpl` con
      contenuti **diversi**: `./compiler fib` deve tradurre `fib`, e il `.pcd` prodotto deve
      corrispondere a `fib`, non a `fib.cpl`. distinguere i due contenuti con un
      `print "..."` diverso in ciascun sorgente.
      Verifica: `./asm < fib.pcd` contiene la stringa del file `fib` e non quella di `fib.cpl`.
- [x] 2.5 Verificare che un sorgente irraggiungibile produca una diagnostica che nomina tutti i
      percorsi tentati, in una directory vuota: `./compiler fib` deve citare sia `fib` sia
      `fib.cpl` su stderr, non produrre alcun `.pcd` e uscire con codice 1.
      Verifica: stderr contiene entrambe le stringhe; `ls` non mostra `fib.pcd`; `$?` e' 1.
- [x] 2.6 Verificare lo scenario misto in `/tmp/opencode/cpl2-cli-check/misto`, che contiene
      solo `fib.cpl`: `./compiler fib mancante` deve produrre `fib.pcd`, segnalare
      l'irraggiungibilita' di `mancante` e `mancante.cpl`, e uscire con codice 1.
      Verifica: `fib.pcd` esiste ed e' valido; stderr cita `mancante` e `mancante.cpl`; `$?` e' 1.
- [x] 2.7 Documentare il comportamento in `AGENTS.md`, sezione «Compiler — sources in argv, one
      `.pcd` per source`: aggiungere la regola di risoluzione dell'input (`fib` -> tenta `fib.cpl`
      come ripiego, con precedenza del percorso digitato e con il punto cercato solo nel nome di
      base) accanto alla derivazione del nome di output, e ricordare che un nome con
      l'estensione gia' presente non viene riscritto.
      Verifica: `grep -n "fib.cpl" AGENTS.md` trova la nuova frase e il testo non contraddice la
      sezione «Running a programma» sull'interprete.

## 3. Diagnostiche e forma finale degli argomenti

- [x] 3.1 Verificare che il prefisso delle diagnostiche sia il percorso **risolto**: in
      `/tmp/opencode/cpl2-cli-check/solo` preparare `fib.cpl` con un errore di sintassi alla
      riga 3 e `./compiler fib` deve emettere su stderr un messaggio che inizia con `fib.cpl: `
      e riporta `line 3`.
      Verifica: il primo carattere del messaggio e' `f` e il percorso riportato e' `fib.cpl`,
      non `fib`.
- [x] 3.2 Verificare che anche le diagnostiche dello scanner portino il percorso risolto: stesso
      `fib.cpl`, con una stringa non terminata; il messaggio `stringa non terminata` deve
      arrivare su stderr con prefisso `fib.cpl: ` e non comparire nel `.pcd`.
      Verifica: `grep -c "stringa non terminata" fib.pcd` restituisce 0.
- [x] 3.3 Verificare che un nome **con** estensione non venga riscritto: in
      `/tmp/opencode/cpl2-cli-check/txt`, che contiene `sorg.cpl` e **non** `sorg.txt`,
      eseguire `./compiler sorg.txt` e verificare che venga segnalata l'impossibilita' di aprire
      `sorg.txt` e che `sorg.pcd` non venga prodotto dal contenuto di `sorg.cpl`.
      Verifica: stderr nomina `sorg.txt`; il comando esce con codice 1; `sorg.pcd` assente.
- [x] 3.4 Verificare la regola del punto solo nel nome di base: in
      `/tmp/opencode/cpl2-cli-check/dir.d` con il solo file `x.cpl`, eseguire `../compiler x`
      (che non vede `dir.d/x.cpl` dalla sua directory di lavoro) e poi, dalla directory
      superiore, `compiler dir.d/x`, che deve tradurre `dir.d/x.cpl` e scrivere
      `dir.d/x.pcd`.
      Verifica: il `.pcd` e' `dir.d/x.pcd` dentro `dir.d/`, e non un percorso derivato dal punto
      della directory.
- [x] 3.5 Verificare che il ripiego scatti solo su assenza del file: in
      `/tmp/opencode/cpl2-cli-check/noexec`, con `fib` a permessi 000 e `fib.cpl` leggibile,
      `./compiler fib` deve segnalare che `fib` non e' apribile e **non** tradurre `fib.cpl`
      (D2). Attenzione: la verifica vale solo se il processo non gira come root con
      `CAP_DAC_OVERRIDE`; in quel caso `fopen` riesce comunque e lo scenario e' irraggiungibile
      — segnalarlo e passare, senza modificare il codice.
      Verifica: stderr nomina `fib` e il suo errore di permessi; `fib.pcd` assente o identico a
      quello prodotto prima dell'esecuzione.

## 4. Verifica d'integrazione

- [x] 4.1 Verificare che tutte le invocazioni gia' funzionanti siano invariate: da
      `/tmp/opencode/cpl2-cli-check/matrix` rieseguire `./compiler a.cpl b.txt noext
      dir.d/x.cpl` e confrontare i quattro `.pcd` con i nomi attesi (vedi 1.3), e verificare
      che `./compiler fib.cpl` produca un `fib.pcd` identico a quello salvato in `baseline`.
      Verifica: stessi nomi di file, stessi contenuti di `baseline/fib.pcd`.
- [x] 4.2 Verificare la modalita' standard input invariata: `./compiler < fib.cpl > fib.pcd` deve
      produrre lo stesso file di `./compiler fib.cpl`, e `cmp` deve confermarlo. Il ramo e'
      quello di D8 e non deve essere stato toccato.
      Verifica: `cmp fib.pcd baseline/fib.pcd` (dopo la redirezione) non riporta differenze.
- [x] 4.3 Verificare i codici di uscita aggregati: due sorgenti con 1 e 2 errori di sintassi
      nella stessa invocazione devono far uscire il compilatore con codice 3 producendo
      comunque entrambi i `.pcd`.
      Verifica: `$?` vale 3 e i due file p-code esistono.
- [x] 4.4 Verificare la compilazione completa con `make` (interpreter, compiler, asm) e che
      `cpl2.gram`, `cpl2.c`, `cpl2.h`, `emit.c`, `asm.c`, `pcodes.*` e `interpreter.cpp` non
      siano stati modificati da questa change: `git status --short` deve elencare solo
      `parser.c` e `AGENTS.md` oltre agli artefatti di build gia' non tracciati e alla directory
      `openspec/`.
      Verifica: il diff tocca solo i due file attesi e `make` termina senza errori.
      Nota 2026-10-17: `git status` elenca anche `fib.cpl` (`if n<=2` -> `if n<2`), modifica
      del sorgente fatta in parallelo e non da questa change, che l'utente ha deciso di
      tenere. Perci' la prova di neutralita' non e' piu' il confronto con `baseline/`, che
      era stato preso sul sorgente pre-modifica: e' il confronto diretto fra il compilatore
      di riferimento costruito da `HEAD:parser.c` e quello nuovo, eseguito sugli stessi
      sorgenti correnti. Risultato: `a.pcd`, `b.pcd`, `noext.pcd`, `dir.d/x.pcd`, `fib.pcd`
      e il `.pcd` da standard input sono tutti identici, e l'albero dei file prodotti
      coincide. `make` termina senza errori e i sette file protetti sono intatti.
- [x] 4.5 Verificare end-to-end il ciclo `compile -> run` con il nome senza estensione:
      compilare un fixture in `/tmp/opencode/cpl2-cli-check/solo` e lanciare
      `../interpreter fib`, che deve eseguire il p-code appena prodotto.
      Verifica: l'interprete stampa l'output del programma e termina con codice 0.
- [x] 4.6 Ripulire gli artefatti di verifica generati dentro il repository: i `.pcd` prodotti
      nella root e in `test/` durante le verifiche non vanno committati. Confrontare
      `git status --short` con lo stato di partenza e rimuovere solo i file generati da questa
      change, senza toccare `tmp/`, `cpl2.out` e i binari gia' presenti prima.
      Verifica: `git status --short` non mostra nuovi file non tracciati rispetto allo stato
      iniziale della change.

## Workflow follow-up

- Archiviare la change una volta soddisfatti i requisiti di revisione del progetto.
- Verificare il risultato archiviato: la spec `compiler-cli` in
  `openspec/specs/compiler-cli/spec.md` deve contenere il requirement «Risoluzione implicita di
  `.cpl` per un nome senza estensione», il requirement «Le diagnostiche nominano il percorso
  risolto» e il testo aggiornato di «Un file non apribile non interrompe gli altri».
- `todo.md`: nessun punto da spuntare per questa change. Spuntare il punto 2
  («specificare i file da eseguire in cli») e' igiene del file e riguarda il commit `e616f62`,
  non questa change.