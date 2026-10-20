# Proposal

## Why

Il compilatore usa il percorso digitato in command line **verbatim** come sorgente: in
`main()` (`parser.c`) l'unica cosa che ricava dall'argomento e' `pcodePath()`, cioe' il
nome del `.pcd` di output. Quindi `./compiler fib.cpl` funziona, ma `./compiler fib` termina
con `impossibile aprire il sorgente: fib` anche quando `fib.cpl` e' li' accanto.

L'interprete ha gia' la convenzione opposta e piu' comoda: `pcodePathFor()` in
`interpreter.cpp` accoda `.pcd` quando il nome di base non ha alcun punto, cosi'
`./interpreter fib` esegue `fib.pcd`. I due strumenti hanno quindi convenzioni opposte sullo
stesso nome — uno sostituisce l'estensione per ricavare l'output, l'altro la completa per
trovare l'input — e il ciclo `compile -> run` si puo' fare con `fib` da un lato solo.

La richiesta e' di completare il nome anche sul lato del compilatore, cosi' `fib` sta per lo
stesso file nei due strumenti.

Nota sulla formulazione della richiesta: «se il file non ha estensione ed esiste il file con
estensione `.pcd`, aggiungere l'estensione e compilare il file» letta alla lettera farebbe
aprire `fib.pcd` e tradurlo come sorgente cpl2, cioe' passerebbe un p-code binario allo
scanner producendo solo errori di sintassi. Il `.pcd` e' il formato di **output**, non
quello dei sorgenti: la condizione di esistenza va quindi posta su `.cpl`, e il file da
compilare e' il sorgente. La forma finale e' simmetrica con l'interprete solo nell'ergonomia,
non nella regola: l'interprete accoda `.pcd` incondizionatamente, il compilatore accoda
`.cpl` **solo come ripiego** quando il percorso digitato non si apre.

## What Changes

- `./compiler fib` risolve il sorgente in due passi: prova `fib` e, solo se non e' apribile,
  prova `fib.cpl`. Se `fib.cpl` si apre e' quello il file tradotto, e il p-code finisce in
  `fib.pcd`. Se nessuno dei due si apre, la diagnostica li **nomi entrambi** ed il codice di
  uscita sale di uno.
- Il percorso digitato **ha la precedenza**: se `fib` e `fib.cpl` esistono entrambi viene
  tradotto `fib`, cioe' il comportamento di oggi non cambia per nessuna invocazione gia'
  funzionante. La precedenza e' il motivo per cui il ripiego non e' una sostituzione.
- Il ripiego vale solo se il **nome di base** non contiene alcun punto, con la stessa regola
  di `pcodePath()` e di `pcodePathFor()`: `dir.d/x` cerca `dir.d/x` e poi `dir.d/x.cpl`, il
  punto della directory resta parte della directory.
- Un nome **che ha gia' un'estensione** non viene mai riscritto: `./compiler sorg.txt` prova
  `sorg.txt` e basta, non cerca `sorg.txt.cpl` ne' `sorg.cpl`.
- Il **percorso risolto** (non quello digitato) diventa la sorgente delle tre cose che oggi
  derivano dall'argomento: il prefisso delle diagnostiche (`srcFile`), la derivazione del
  nome del `.pcd` e il testo del messaggio di sorgente non apribile. In particolare
  `./compiler fib` che compila `fib.cpl` produce diagnostiche con prefisso `fib.cpl: `, cosi'
  il messaggio dice sempre quale file e' stato davvero tradotto.
- Il `.pcd` di output non cambia nome: `pcodePath("fib")` e `pcodePath("fib.cpl")` danno
  entrambi `fib.pcd`, perche' lo stem e' lo stesso. La derivazione passa comunque dal
  percorso risolto perche' `main()` abbia una sola nozione di "percorso del sorgente".
- Il ripiego scatta solo su `ENOENT`: se `fib` esiste ma non e' leggibile, non si passa a
  `fib.cpl`, si riporta l'errore sul percorso che l'utente ha digitato. Tradurre un file
  diverso da quello nominato e' peggio che fallire.
- Fuori perimetro: nessun flag e nessuna nuova opzione (`-h` resta un percorso, come gia'
  e'); la modalita' standard input resta identica; il formato del `.pcd`, l'header
  `#cpl20.0.1` e `pcodePath()` come contratto non cambiano; la grammatica non e' toccata,
  quindi nessuna rigenerazione di `cpl2.c`/`cpl2.h`.
- Fuori perimetro: il ripiego `.cpl` per un nome con un'estensione **diversa** da `.cpl`
  (`sorg.txt` -> `sorg.cpl`). Sarebbe la generalizzazione naturale, ma cambia il contratto
  di `./compiler sorg.txt`, che oggi e' documentato come «il p-code viene scritto in
  `sorg.pcd`» compilando `sorg.txt`. Resta aperta per una change separata.

## Capabilities

### New Capabilities
- Nessuna.

### Modified Capabilities
- `compiler-cli`: la selezione del sorgente indicato in command line acquisisce la
  risoluzione a `.cpl` per i nomi senza estensione, con precedenza del percorso digitato; le
  diagnostiche di sorgente non apribibile devono nominare tutti i percorsi tentati e
  identificare il file risolto.

## Impact

- `parser.c`, `main()`: il ramo multi-file oggi calcola `pcodePath(argv[i])`, apre
  `argv[i]` e usa `argv[i]` come `srcFile`. Serve un passo di risoluzione fra questi tre
  punti, con un buffer per il candidato `.cpl` (la casa e' `char pcdPath[2000]`, quindi
  `2000` anche qui) e una regola di precedenza esplicita. Il ramo `argc < 2` non e' toccato.
- `parser.c`: la regola «il punto si cerca solo nel nome di base» esiste oggi duplicata fra
  `pcodePath()` e `pcodePathFor()` (in due binari diversi, in due linguaggi diversi, quindi
  non condivisibile) e con la nuova change serve anche al controllo dell'estensione in
  `parser.c`. Conviene estrarre un helper unico in `parser.c` usato sia da `pcodePath()` sia
  dalla risoluzione, cosi' la regola ha un posto solo nel file.
- `parser.c`, `diag()` e `srcFile`: `srcFile` passa dal percorso digitato al percorso
  risolto. Il formato del messaggio (`"<percorso>: <testo>"`) non cambia, cambia il
  percorso che ci finisce dentro. Il testo esatto del messaggio «impossibile aprire il
  sorgente» e' libero, ma deve citare i percorsi tentati.
- `parser.c`, `errno`: il ramo di apertura deve ricordare `errno` del primo tentato prima di
  tentare il secondo, altrimenti il messaggio finale riporta l'errore del tentativo sbagliato.
- `AGENTS.md`: la sezione «Compiler — sources in argv, one `.pcd` per source» descrive la
  derivazione del nome di output ma non dice nulla della risoluzione dell'input: va
  aggiunta la regola `fib` -> `fib.cpl` come ripiego, con la precedenza del nome digitato.
- `todo.md`: nessun punto da spuntare. Il punto 2 («specificare i file da eseguire in cli»)
  riguarda l'interprete ed e' stato chiuso dal commit `e616f62`, non da questa change; questa
  e' un affinamento del contratto gia' scritto, non una voce di `todo.md`.
- `interpreter.cpp`, `./asm`, `emit.c`, `pcodes.*`, `cpl2.gram`, `test/`: non toccati.