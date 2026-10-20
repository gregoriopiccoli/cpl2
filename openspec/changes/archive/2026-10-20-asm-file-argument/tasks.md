# Tasks

## 1. Implementazione in `asm.c`

- [x] 1.1 Includere `cli.c` in `asm.c` e riscrivere `main()` come
      `main(int argc,char** argv)` con una tabella `cliOpzione` vuota e
      `cliScansiona(...,pos,1)`: zero posizionali → `cpl2asm(stdin)`, esattamente
      uno → `fopen` del percorso. Verifica: `make` compila `asm` senza errori e
      `./asm < test/simple_class.pcd | ./asm` (round trip) produce lo stesso output
      di prima della change.
- [x] 1.2 Gestire gli errori di command line: `errore != CLI_ERRORE_NESSUNO`
      (opzione sconosciuta) o piu' di un posizionale → messaggio su stderr che
      nomina l'argomento, nessuna traduzione, exit 1. Verifica:
      `./asm -b fib.pcd` nomina `-b` su stderr e termina con 1;
      `./asm a.pcd b.pcd` segnala i troppi percorsi e termina con 1;
      `./asm -- -file.pcd` traduce `-file.pcd`.
- [x] 1.3 Gestire il file non apribile: `fopen` fallito → messaggio su stderr che
      nomina il percorso, stdout vuoto, exit 1. Verifica:
      `./asm mancante.pcd` scrive solo su stderr e termina con 1;
      `./asm mancante.pcd 2>/dev/null | wc -c` produce 0.
- [x] 1.4 Verificare l'equivalenza delle due modalita' e i codici di uscita:
      `./asm fib.pcd > a.txt` e `./asm < fib.pcd > b.txt` con `diff a.txt b.txt`
      vuoto; `echo $?` vale 2 per binario→listing, 1 per listing→binario, 0 per
      input non p-code, in entrambe le modalita'; dopo `./asm fib.pcd` nella
      directory non compare alcun file nuovo.

## 2. Commenti e documentazione

- [x] 2.1 Aggiornare il commento di intestazione di `cli.c`: con `asm.c` il file
      e' incluso da tre binari, non piu' da due. Verifica: `make` e
      `cc -g -O2 cli_test.c -o cli_test && ./cli_test` passano ancora.
- [x] 2.2 Aggiornare la sezione "asm is a checker, not a pipeline stage" di
      `AGENTS.md` documentando la nuova forma d'uso (`./asm fib.pcd`, un solo
      percorso, output sempre su stdout, `./asm < fib.pcd` invariato).
      Verifica: i comandi scritti nel documento producono cio' che descrivono.

## 3. Verifica di integrazione

- [x] 3.1 `make && ./runtest` passa al completo (nessuna regressione in
      `compiler`, `interpreter` o nei test): exit code 0 e nessun test fallito.
- [x] 3.2 Controllare a mano gli scenari dello spec `asm-cli` uno per uno
      (`openspec/changes/asm-file-argument/specs/asm-cli/spec.md`), compresi
      `cat fib.pcd | ./asm -` e `./asm -- -file.pcd`.

## Workflow follow-up

- Archiviare la change (`openspec archive`) dopo la revisione.
