# Tasks

## 1. Implementare `sys::loadModule`

- [ ] 1.1 Aggiungere a `sys` i campi `unordered_map<string,contextObj*> moduleContexts` e
  `string currentDir`, dichiarare `~sys()`, e aggiungere gli helper per la directory di un
  percorso e per derivare il percorso del modulo (`a.b` → `a/b.pcd`). Verifica: `make` compila
  senza errori e `./runtest` resta verde (comportamento ancora invariato).
- [ ] 1.2 Definire `sys::loadModule` secondo il design: cache per nome, `loadPcd`, iscrizione in
  cache prima dell'esecuzione, contesto nuovo con superlivello builtin e `lock()`, esecuzione
  con `interp` dedicato, `assert(sp==-1)`, salvataggio/ripristino di `currentDir` anche in caso
  di eccezione, ritorno 0 su caricamento fallito. Definire `~sys()` che distrugge i
  `pcodeProgram` in cache. Verifica: `make` compila senza errori e `./runtest` resta verde.
- [ ] 1.3 Impostare `theSys.currentDir` in `runPcd()` e `benchPcd()` dal percorso caricato,
  prima dell'esecuzione, così l'import cerca nella directory del `.pcd` principale. Verifica:
  `make` compila e `./runtest` resta verde.

## 2. Collegare `IMPORT` e aggiornare test e documentazione

- [ ] 2.1 Riscrivere `pcodeImport::exec()`: mantenere la validazione di N, profondità e
  `strObj`, chiamare `theSys.loadModule()` per ogni nome dal basso verso la cima, sollevare
  `domain_error` sul primo caricamento fallito, rimuovere le N stringhe e non stampare più i
  nomi. Verifica: `make` compila senza errori.
- [ ] 2.2 Aggiungere `test/a_module.cpl` con `var int x:=1` e `? 'modulo a'` più
  `test/a_module.out` con `modulo a`; riscrivere `test/import_stack.cpl` perché importi due
  volte `a_module`, dichiari `var int x:=2` e stampi `x`, e `test/import_stack.out` con
  `modulo a` e `2`. Verifica: `./runtest` passa, l'output di `import_stack` contiene una sola
  riga del modulo e la seconda `var int x` non fallisce.
- [ ] 2.3 Verificare manualmente la mappatura puntata e gli import annidati: in una directory
  temporanea creare `mod/x.cpl` e un `main.cpl` con `import mod.x`; compilare entrambi e
  eseguire `./interpreter main.pcd`. Verifica: viene eseguito `mod/x.pcd` e il suo output
  compare una sola volta; ripetere l'import e verificare che non venga rieseguito.
- [ ] 2.4 Aggiornare `AGENTS.md`: nella sezione sui costrutti non eseguibili, sostituire il
  testo che dice che `import` non carica i moduli con la nuova regola (percorso `a.b` →
  `a/b.pcd` relativo al p-code che importa, esecuzione una sola volta in un contesto separato,
  cache in `sys`, nessuna esportazione dei nomi) e lasciare `from ... import ...` fra i non
  implementati. Verifica: rileggere la sezione e confrontarla con il comportamento osservato
  al punto 2.2.

## 3. Verifica d'integrazione

- [ ] 3.1 Eseguire `make && ./runtest` due volte di seguito. Verifica: entrambi i giri riportano
  lo stesso esito, exit 0 e nessun file nuovo in `test/`.
- [ ] 3.2 Provare il fallimento di caricamento: compilare in una directory temporanea un
  sorgente con `import mancante` ed eseguirlo. Verifica: il programma termina con codice
  diverso da 0, stderr nomina il percorso cercato e stdout non contiene il nome del modulo.
- [ ] 3.3 Eseguire un programma che importa un modulo con `./interpreter -g file.pcd`.
  Verifica: l'output del programma e del modulo è corretto, il rapporto GC viene stampato e
  non ci sono crash dovuti ai contesti dei moduli lockati.
- [ ] 3.4 Verificare che `from ... import ...` resti fuori perimetro: compilare
  `test/simple_import.cpl` ed eseguirne il p-code. Verifica: fallisce ancora con l'errore di
  p-code non implementato di `P_FROM`, non con un errore di caricamento di un modulo.
