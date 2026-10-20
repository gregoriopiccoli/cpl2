# Tasks

## 1. Percorso da caricare e diagnostiche di `loadPcd()`

- [x] 1.1 Aggiungere in `interpreter.cpp` l'helper `pcodePathFor(const string&)` con la regola di D2: nome di base = parte dopo l'ultimo `'/'`, ultimo `.` cercato **solo** nel nome di base, `.pcd` accodato quando non c'e' alcun punto; la directory resta intatta. Verifica: `make interpreter` compila e l'helper e' definito prima di `main()`.
- [x] 1.2 Applicare D4 in `pcodeProgram::loadPcd()`: passare i due messaggi esistenti (`not a compiled cpl2 file ...`, `<fn> has errors!`) da `cout` a `cerr` e aggiungere il messaggio, oggi assente, del ramo in cui il file non si puo' aprire, che nomina il percorso tentato. Verifica: `grep -n 'cout' interpreter.cpp` non trova piu' occorrenze dentro `loadPcd()` e il ramo del file non apribile contiene una scrittura su `cerr`.

## 2. `main()` con gli argomenti

- [x] 2.1 Applicare D3: `runPcd()` passa da `void` a `int` e restituisce 0 se il caricamento e' riuscito, 1 altrimenti, senza alterarne il corpo. Verifica: `make interpreter` compila e `bench()`, che continua a chiamarla ignorando il ritorno, compila senza warning.
- [x] 2.2 Applicare D1 e D5: `main()` acquisisce `(int argc, char** argv)`, stampa su stderr le istruzioni d'uso e restituisce 1 quando `argc != 2`, altrimenti esegue `return runPcd(pcodePathFor(argv[1]))`; rimuovere il blocco hardcoded (`runPcd("primo.pcd")`, i `bench(...)`, `bench_fib_cc()`) mantenendo le funzioni `bench*()`/`test*_cc()` nel sorgente. Verifica: `make` completo senza errori e `./interpreter` esce con codice 1 stampando l'uso su stderr senza eseguire nulla.
- [x] 2.3 Aggiornare `AGENTS.md`, sezione "Running a programma": `./interpreter <file>` al posto di "nessun argomento", la regola del completamento `.pcd`, il fatto che non serve piu' modificare `main()` per eseguire un programma e il fatto che `./interpreter` da solo non e' piu' uno smoke test. Verifica: le due righe di comando riportate in `AGENTS.md`, eseguite cosi' come sono scritte, funzionano.

## 3. Verifica degli scenari della spec

Non c'e' un runner di test (vedi `AGENTS.md`): la verifica e' esecuzione degli scenari e
confronto fra uscita, stream e codice di ritorno.

- [x] 3.1 Scenari di naming (requisito "Derivazione del percorso da caricare"): `./interpreter primo.pcd` e `./interpreter primo` caricano ed eseguono lo stesso programma; `./compiler test/simple_class.cpl && ./interpreter test/simple_class` esegue il p-code prodotto dal compilatore nella sottodirectory. Verifica: tutti e tre i comandi producono l'output del programma e codice di uscita 0.
- [x] 3.2 Scenari di selezione (requisito "Selezione del file da eseguire"): `./interpreter` e `./interpreter primo.pcd fib.pcd` non eseguono nulla e finiscono con codice 1; `./interpreter -h` tenta di aprire il file `-h` invece di trattarlo come opzione. Verifica: i tre comandi scrivono su stderr e hanno codice di uscita 1.
- [x] 3.3 Scenari di file non caricabile (requisito "Diagnostica e codice di uscita per un file non caricabile"): `./interpreter non_esiste`, `./interpreter primo.cpl` e un p-code prodotto da un sorgente con errore di sintassi (compilato in `tmp/`, che e' gia' scratch) riportano ciascuno la propria condizione su stderr, non eseguono nulla e finiscono con codice 1. Verifica: i tre messaggi distinguono "non apribile", "non e' un p-code" e "ha errori", e nessuno finisce su stdout (`./interpreter non_esiste 2>/dev/null` non produce il messaggio).
- [x] 3.4 Scenari di uscita e di integrita' dell'output (requisiti "Codice di uscita in caso di esecuzione" e "Diagnostica ... "): `./interpreter primo.pcd` termina con codice 0 e l'output del programma resta su stdout; il rapporto del GC continua a essere stampato come oggi. Verifica: `echo $?` vale 0 e il redirect di stdout contiene l'output del programma.

## 4. Controllo finale del diff

- [x] 4.1 Rileggere `git diff` e verificare che tocchi solo `interpreter.cpp` e `AGENTS.md`, che la rinomina non committata `test()` -> `runPcd()` sia conservata, che nessuna funzione `bench*()`/`test*_cc()` sia stata cancellata e che nessun file della grammatica (`cpl2.gram`, `cpl2.c`, `cpl2.h`) sia stato toccato. Verifica: `git diff --stat` mostra solo i file attesi e `git status` non mostra nuovi file tracciati (i `.pcd` generati dalla verifica sono build output e non vanno committati, vedi `AGENTS.md`).