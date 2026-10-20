# Tasks

## 1. Instradamento dell'emissione su file

- [x] 1.1 In `emit.c` aggiungere il globale `FILE* emit_out;` e sostituire tutte le `printf`
      con `fprintf(emit_out, ...)` in `emit`, `emit_i`, `emit_s`, `emit_line`, `emit_ver`.
      Il globale **non** puo' essere inizializzato a scope di file con `stdout`: in C non e'
      un'espressione costante e `cc` rifiuta l'inizializzatore (D1 in `design.md`). L'assegnazione
      `emit_out=stdout;` va quindi fatta in cima al `main()` di `parser.c` (ramo `argc < 2`,
      vedi 4.2) e in cima al `main()` di `asm.c`.
      Verifica: `grep -nP '(?<!f)printf' emit.c` non restituisce nulla, e `make compiler asm`
      compila senza warning.
- [x] 1.2 Compilare ed eseguire la pipeline esistente `./compiler < test/simple_func.cpl | ./asm`
      prima e dopo la modifica e verificare che l'output testuale sia identico, dimostrando
      che `asm.c` non e' stato disturbato.

## 2. Diagnostiche su stderr con il nome del file

- [x] 2.1 In `parser.c` aggiungere `#include <stdarg.h>`, il globale
      `const char* srcFile = NULL;` e l'helper `void diag(const char* fmt, ...)` che
      scrive su stderr anteponendo `"<srcFile>: "` quando `srcFile != NULL`. Posizionarlo
      prima di `tokenize()` e prima di `#include "cpl2.c"`.
      Verifica: `make compiler` compila e un file con un errore produce il messaggio su stderr.
- [x] 2.2 Sostituire le due `printf` di `tokenize()` (`stringa non terminata`,
      `numero terminato male`) con `diag()`, senza cambiare il testo dei messaggi.
      Verifica: un sorgente con una stringa aperta e uno con `123abc` riportano i due
      messaggi su stderr e il `.pcd` prodotto non li contiene.
- [x] 2.3 In `cpl2.gram` cambiare l'azione `%syntax_error` (riga 26) da
      `printf("syntax error at line %d!\n",sState->lineNo)` a `diag("syntax error at line %d!\n",sState->lineNo)`,
      lasciando invariato l'incremento di `sState->errors`.
      Verifica: il diff di `cpl2.gram` e' di una sola riga.
- [x] 2.4 Rigenerare la grammatica con `make cpl2.c` (o `lemon cpl2.gram`), verificando che
      `lemon` termini con codice 0 e che `cpl2.c` e `cpl2.h` contengano la nuova azione.
      Verifica: `grep -c 'diag("syntax error' cpl2.c` restituisce 1 e il Makefile continua a
      produrre il binario senza conflitti.
- [x] 2.5 Committare `cpl2.c` e `cpl2.h` insieme a `cpl2.gram` (add espliciti, mai `git add -A`),
      accettando il rumore di template generato dalla versione di lemon installata.
      Verifica: `git status` mostra solo i tre file previsti e il diff di `cpl2.c` contiene
      la modifica attesa piu' il boilerplate gia' noto.

## 3. Correzioni a `parse()` per la lettura da file

- [x] 3.1 Sostituire `feof(stdin)` con `feof(f)` nel loop di `parse()`.
      Verifica: compilando un file reale il compilatore termina invece di entrare in loop
      infinito (prima della modifica il processo va terminato con `timeout`).
- [x] 3.2 Controllare il valore di ritorno della prima `fgets` in `parse()`: con file vuoto
      inizializzare `line[0]='\0'` e saltare il loop, in modo che venga comunque emesso
      header e `PCODEEND`. Verifica: `compiler vuoto.cpl` produce un `vuoto.pcd` di due righe
      (`#cpl20.0.1` e `PCODEEND 0`) e termina con codice 0.

## 4. Selezione dei sorgenti in `main()`

- [x] 4.1 Implementare in `parser.c` la derivazione del nome di output dal percorso del
      sorgente: individuare l'ultimo `.` **nel solo nome di base** (non nel percorso),
      troncare da li' e accodare `.pcd`; se il nome di base non contiene `.`, accodare
      `.pcd`. La directory del sorgente e' preservata per costruzione.
      Verifica: `fib.cpl` -> `fib.pcd`, `test/simple_class.cpl` -> `test/simple_class.pcd`,
      `fib` -> `fib.pcd`, `sorg.txt` -> `sorg.pcd`, `dir.d/senza_ext` -> `dir.d/senza_ext.pcd`.
- [x] 4.2 Riscrivere `main()` con i due rami: `argc < 2` esegue il percorso invariato
      (`srcFile = NULL`, `emit_out = stdout`, `parse(stdin)`, `return` del suo conteggio errori);
      `argc >= 2` itera su `argv[1..argc-1]` aprendo sorgente e output, impostando `srcFile`
      ed `emit_out` prima di `parse()`, e chiudendo entrambi gli stream.
      Verifica: `echo "..." | ./compiler | ./asm` produce lo stesso p-code di prima la change.
- [x] 4.3 Nel ramo multi-file, su sorgente non apribile o su output non apribile: emettere
      `diag()` con un messaggio che nomini il percorso, incrementare il totale degli errori,
      chiudere gli stream gia' aperti e proseguire con il file successivo.
      Verifica: `compiler a.cpl mancante.cpl b.cpl` segnala `mancante.cpl` su stderr,
      produce `a.pcd` e `b.pcd`, ed esce con codice 1.
- [x] 4.4 Sommare su tutti i file i conteggi errori restituiti da `parse()`, preservando il
      contratto "exit code == numero di errori".
      Verifica: due sorgenti con 1 e 2 errori di sintassi fanno uscire il compilatore con
      codice 3.

## 5. Verifica di fine change

- [x] 5.1 Verificare la modalita' file end-to-end su tutti i fixture di `test/`
      (`simple_class.cpl`, `simple_func.cpl`, `simple_import.cpl`, `test_expr1.cpl`,
      `test_stm1.cpl`, `test_var1.cpl`), confrontando `./asm < <fixture>.pcd` con il blocco
      di p-code atteso commentato dentro `test/simple_class.cpl`.
      Verifica: ogni `.pcd` generato supera il confronto e ha ultima riga `PCODEEND 0`.
- [x] 5.2 Verificare che i `.pcd` generati siano caricabili dall'interprete: il file prodotto
      da `compiler fib.cpl` ha header esattamente `#cpl20.0.1` ed è accettato da
      `pcodeProgram::loadPcd`. Verifica: ispezionare il file con `head -1` e
      `tail -1`, senza modificare il `main()` hardcoded dell'interprete.
- [x] 5.3 Verificare la compilazione completa con `make` (interpreter, compiler, asm) e
      controllare che `./asm < fib.pcd | ./asm` torni ancora fedelmente al p-code binario.
      Verifica: il round-trip produce un file identico a quello di partenza.
- [x] 5.4 Ripulire gli artefatti generati durante le verifiche: i `.pcd` di prova vanno
      prodotti in una directory di scratch fuori dal repository (per esempio
      `/tmp/opencode/cpl2-cli-check`), perche' il progetto non ha `.gitignore` e non
      devono accumularsi file non tracciati nella root. Verifica: `git status` pulito
      tranne le modifiche previste.

## Workflow follow-up

- Archiviare la change una volta soddisfatti i requisiti di revisione del progetto.
- Verificare il risultato archiviato: la spec `compiler-cli` deve esistere in
  `openspec/specs/compiler-cli/spec.md` con i suoi scenari.
- Il punto 1 di `todo.md` puo' essere spuntato una volta archiviata; il punto 2
  (file in command line per l'interprete) resta fuori perimetro e aperto.