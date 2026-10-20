# Proposal

## Why

Oggi `./compiler` non accetta argomenti: `main()` chiama `parse(stdin)` e non ha un
modo per dire al programma *quale* file tradurre. L'unica forma di uso possibile è la
redirezione da shell (`./compiler < fib.cpl > fib.pcd`), con la parte destra della
pipeline scritta a mano ogni volta. Questo rende impossibile compilare un gruppo di
file e obbliga a eseguire il compilatore una volta per file, con il costo di gestire
a mano l'output di ogni invocazione. E' anche il punto 1 di `todo.md`.

La funzione `parse(FILE* f)` esiste gia' ed accetterebbe un `FILE*`, ma non e' mai
stato cablato: il suo loop di lettura controlla `feof(stdin)` invece di `feof(f)`, quindi
passarle un file reale oggi produrrebbe un loop infinito.

## What Changes

- **BREAKING** `compiler` accetta una lista di sorgenti in command line. Per ogni file
  sorgente produce **un p-code separato** scritto su file: il nome di output e' quello
  dell'input con l'estensione sostituita da `.pcd`, nella stessa directory del sorgente
  (`fib.cpl` -> `fib.pcd`, `test/simple_class.cpl` -> `test/simple_class.pcd`).
- **BREAKING** Quando sono indicati dei file, il p-code non viene piu' scritto su stdout:
  stdout non riceve piu' nulla. Senza argomenti il comportamento resta esattamente quello
  attuale (legge stdin, scrive il p-code su stdout), per non rompere le pipeline esistenti.
- Le diagnostiche (`syntax error at line N!`, `stringa non terminata`, `numero terminato
  male`, piu' il nuovo errore di file non apribile) vanno su **stderr** con il nome del
  file sorgente come prefisso. Finora finivano su stdout e venivano therefore cotte dentro
  il `.pcd`, con l'output su file questo difetto si rimuove naturalmente.
- Il codice di uscita resta il numero totale di errori di compilazione, ora sommato su
  tutti i file elaborati invece che sull'unico stream.
- Correzione del difetto `feof(stdin)` in `parse()`, necessario perche' il compilatore
  possa leggere da file senza entrare in loop infinito.
- Correzione della lettura della prima riga in `parse()`, che oggi lascia `line` non
  inizializzata se il file e' vuoto: un file sorgente vuoto era impossibile prima, con i
  file da command line diventa un caso ordinario da gestire.

## Capabilities

### New Capabilities
- `compiler-cli`: contratto della command line del compilatore cpl2: selezione dei
  sorgenti (argumenti vs stdin), destinazione del p-code (file `.pcd` vs stdout),
  regola di derivazione del nome di output, emissione delle diagnostiche su stderr con
  prefisso del file e conteggio errori, e aggregazione del codice di uscita.

### Modified Capabilities
- Nessuna: non esistono ancora spec in `openspec/specs/`.

## Impact

- `parser.c`: `main()` (nuova logica di selezione dei file e naming degli output),
  `parse()` (loop su `feof(f)`, gestione del file vuoto, apertura dell'output),
  `tokenize()` (diagnostiche su stderr con il nome del file).
- `emit.c`: le quattro funzioni `emit*()`/`emit_line()`/`emit_ver()` emettono su un
  `FILE*` globale invece che su `stdout`, cosi' il p-code puo' andare su file.
- `cpl2.gram`: l'azione `%syntax_error` deve passare da `printf` a stderr con il prefisso
  del file. **Implica la rigenerazione di `cpl2.c` e `cpl2.h`** con `lemon` e il commit dei
  due file generati (con il rumore di template ~300 righe atteso dalla versione di lemon
  installata, gia' documentato in `AGENTS.md`).
- `asm.c`: include `emit.c` e cambia di una sola riga, per l'assegnazione
  `emit_out=stdout;` in cima al `main()` resa necessaria dal fatto che in C `stdout` non e'
  un inizializzatore costante (D1 in `design.md`). `cpl2asm()` non cambia: `./asm` continua
  a funzionare identico sulle pipe esistenti.
- Non cambia l'interprete: `loadPcd` accetta gia' un percorso e i file prodotti sono
  gia' validi (header `#cpl20.0.1` e `PCODEEND 0` quando non ci sono errori). Il formato
  del `.pcd` e' invariato.
- Fuori perimetro: nessun flag (`-o`, `-d`, `-c`...) e nessun supporto a `-` come
  sinonimo di stdin; il secondo punto di `todo.md` (accettare file in command line
  anche nell'interprete) resta aperto.