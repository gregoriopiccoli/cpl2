# Tasks

## 1. Implementare `sys::loadModule`

- [x] 1.1 Aggiungere a `sys` i campi `unordered_map<string,contextObj*> moduleContexts` e
  `string currentDir`, dichiarare `~sys()`, e aggiungere gli helper per la directory di un
  percorso e per derivare il percorso del modulo (`a.b` → `a/b.pcd`). Verifica: `make` compila
  senza errori e `./runtest` resta verde (comportamento ancora invariato).
- [x] 1.2 Definire `sys::loadModule` secondo il design: cache per nome, `loadPcd`, iscrizione in
  cache prima dell'esecuzione, contesto nuovo con superlivello builtin e `lock()`, esecuzione
  con `interp` dedicato, `assert(sp==-1)`, salvataggio/ripristino di `currentDir` anche in caso
  di eccezione, ritorno 0 su caricamento fallito. Definire `~sys()` che distrugge i
  `pcodeProgram` in cache. Verifica: `make` compila senza errori e `./runtest` resta verde.
- [x] 1.3 Impostare `theSys.currentDir` in `runPcd()` e `benchPcd()` dal percorso caricato,
  prima dell'esecuzione, così l'import cerca nella directory del `.pcd` principale. Verifica:
  `make` compila e `./runtest` resta verde.

## 2. Collegare `IMPORT` e aggiornare test e documentazione

- [x] 2.1 Riscrivere `pcodeImport::exec()`: mantenere la validazione di N, profondità e
  `strObj`, chiamare `theSys.loadModule()` per ogni nome dal basso verso la cima, sollevare
  `domain_error` sul primo caricamento fallito, rimuovere le N stringhe e non stampare più i
  nomi. Verifica: `make` compila senza errori.
- [x] 2.2 Aggiungere `test/a_module.cpl` con `var int x:=1` e `? 'modulo a'` più
  `test/a_module.out` con `modulo a`; riscrivere `test/import_stack.cpl` perché importi due
  volte `a_module`, dichiari `var int x:=2` e stampi `x`, e `test/import_stack.out` con
  `modulo a` e `2`. Verifica: `./runtest` passa, l'output di `import_stack` contiene una sola
  riga del modulo e la seconda `var int x` non fallisce.
- [x] 2.3 Verificare manualmente la mappatura puntata e gli import annidati: in una directory
  temporanea creare `pkg/x.cpl` e un `main.cpl` con `import pkg.x` (il nome `mod` non è
  utilizzabile perché è la parola chiave del modulo); compilare entrambi e
  eseguire `./interpreter main.pcd`. Verifica: viene eseguito `pkg/x.pcd` e il suo output
  compare una sola volta; ripetere l'import e verificare che non venga rieseguito.
- [x] 2.4 Aggiornare `AGENTS.md`: nella sezione sui costrutti non eseguibili, sostituire il
  testo che dice che `import` non carica i moduli con la nuova regola (percorso `a.b` →
  `a/b.pcd` relativo al p-code che importa, esecuzione una sola volta in un contesto separato,
  cache in `sys`, nessuna esportazione dei nomi) e lasciare `from ... import ...` fra i non
  implementati. Verifica: rileggere la sezione e confrontarla con il comportamento osservato
  al punto 2.2.

## 3. Verifica d'integrazione

- [x] 3.1 Eseguire `make && ./runtest` due volte di seguito. Verifica: entrambi i giri riportano
  lo stesso esito, exit 0 e nessun file nuovo in `test/`.
- [x] 3.2 Provare il fallimento di caricamento: compilare in una directory temporanea un
  sorgente con `import mancante` ed eseguirlo. Verifica: il programma termina con codice
  diverso da 0, stderr nomina il percorso cercato e stdout non contiene il nome del modulo.
- [x] 3.3 Eseguire un programma che importa un modulo con `./interpreter -g file.pcd`.
  Verifica: l'output del programma e del modulo è corretto, il rapporto GC viene stampato e
  non ci sono crash dovuti ai contesti dei moduli lockati.
- [x] 3.4 Verificare che `from ... import ...` resti fuori perimetro: compilare un sorgente
  temporaneo che contiene solo `from b import c` ed eseguirne il p-code (non
  `test/simple_import.cpl`, il cui primo `import a` si ferma prima sul modulo assente).
  Verifica: fallisce ancora con l'errore di p-code non implementato di `P_FROM`, non con un
  errore di caricamento di un modulo.

## 4. Accesso ai nomi del modulo

- [x] 4.1 Aggiungere a `contextObj` un helper `addIfAbsent(intern,type,value)` che guarda solo il
  livello locale, e in `pcodeImport::exec` creare, dopo ogni `loadModule` riuscito, il simbolo
  del modulo nel contesto dell'importatore: nome uguale all'ultimo componente del nome puntato,
  valore `theSys.moduleContexts[name]`, tipo `theNil`, first-wins. Verifica: `make` compila e
  `./runtest` resta verde.
- [x] 4.2 Implementare `pcodeLoadX` (sostituisce il contenitore in cima con
  `contenitore->load(intern)`) e `pcodeStoreX` (layout `[contenitore, valore]`, `store` e valore
  lasciato in cima) e aggiungere i casi `P_LOADX`/`P_STOREX` in `makePCode()`. Verifica: `make`
  compila senza errori e `./runtest` resta verde.
- [x] 4.3 Aggiungere `test/module_access.cpl` che importa `a_module`, stampa `a_module.x`,
  assegna `a_module.x:=5` e lo stampa di nuovo, con `test/module_access.out` contenente
  `modulo a`, `x:1` e `x:5`. Verifica: `./runtest` passa e il test è elencato fra i superati.
- [x] 4.4 Aggiornare `AGENTS.md`: togliere `LOADX`/`STOREX` dall'elenco dei p-code non
  implementati e descrivere il simbolo del modulo e l'accesso `modulo.nome`. Verifica:
  rileggere la sezione e confrontarla con `test/module_access`.
- [x] 4.5 Eseguire il test primario: `./compiler q1.cpl q.cpl && ./interpreter q.pcd`.
  Verifica: l'output è `q1.i:100` seguito dalla riga di fine, senza errori, ed eseguire
  `./runtest` due volte dà lo stesso esito con exit 0.
