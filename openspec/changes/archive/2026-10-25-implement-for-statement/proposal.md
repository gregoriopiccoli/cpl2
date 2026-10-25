# Proposal

## Why

La grammatica riconosce già le quattro forme del `for` numerico (`to`/`downto`, con e senza
`step`), ma ognuna emette soltanto `COMMENT "da fare for ..."` alla riduzione finale: il p-code
prodotto contiene le espressioni di inizio, fine e passo e il corpo una sola volta, senza salti, e
poi un `COMMENT` che a runtime finisce in `pcodeNotImpl`. Dopo `if`, `while`, `repeat` e
`switch`, il `for` è l'ultimo statement di ciclo "semplice" non eseguibile, e `q.cpl` ne contiene
già i quattro esempi.

## What Changes

- Nuovi p-code per il ciclo numerico:
  - `FOR_TO <nome>` / `FOR_DOWNTO <nome>` (inizializzazione): consumano inizio, fine e passo dallo
    stack, verificano che la variabile di ciclo sia **locale** (dichiarata nel contesto corrente),
    che i tre valori siano `int` e che il segno del passo sia coerente con la direzione, assegnano
    l'inizio alla variabile e lasciano fine e passo sullo stack per tutta la durata del ciclo;
  - `FOR_TEST <label>`: confronta il valore corrente della variabile con la fine e salta fuori dal
    ciclo quando il limite è superato;
  - `FOR_NEXT <nome>`: somma il passo alla variabile.
- La grammatica del `for` in `cpl2.gram` viene riscritta con non-terminali intermedi (come lo
  `switch`) per emettere il p-code nei punti giusti; il passo assente diventa `INT_CONST 1` per
  `to` e `INT_CONST -1` per `downto`. Sparisce il `COMMENT "da fare for ..."`.
- Semantica scelta (vedi design): fine e passo valutati **una sola volta**; estremi inclusi;
  il passo è un incremento con segno (`downto 1 step -2` come in `q.cpl`); passo nullo o di segno
  incoerente con `to`/`downto` è un errore a runtime; zero iterazioni se l'inizio è già oltre la
  fine; la variabile viene riletta dal contesto a ogni giro, quindi un'assegnazione nel corpo ha
  effetto sul ciclo.
- **Requisito dell'utente:** la variabile di ciclo deve essere già presente nel contesto corrente
  (`var int i` nello stesso modulo/funzione); il `for` non la dichiara e non usa una variabile
  trovata solo in un livello superiore (modulo visto da una funzione, builtin): in quel caso
  errore a runtime prima della prima iterazione.
- `cpl2.c`/`cpl2.h` rigenerati con `lemon`; `pcodes.h`, `pcodes.c` e `interpreter.cpp` aggiornati
  per i quattro p-code (`MAX_PCODE` a 111).
- Nuovo test completo `test/for.cpl` + `test/for.out`; nuovo test di errore compile-only non
  necessario (gli errori sono a runtime, vedi design).
- `AGENTS.md`: conteggio dei p-code e forma del p-code del `for`.

Fuori scope: `for each` (resta `COMMENT "da fare for-each"`), `break`/`continue` (non esistono
nella grammatica), variabili di ciclo non `int` (float/date non sono implementati), dichiarazione
implicita della variabile da parte del `for`, controllo a compile time che la variabile sia
locale (il compilatore non ha tabella dei simboli).

## Capabilities

### New Capabilities

- `for-statement`: semantica dello statement `for ... to/downto ... [step ...]` di cpl2, requisito
  della variabile di ciclo locale e forma del p-code che il compilatore genera.

### Modified Capabilities

- `interpreter-pcodes`: nuovi requisiti per l'esecuzione dei p-code `FOR_TO`, `FOR_DOWNTO`,
  `FOR_TEST`, `FOR_NEXT` (effetto sullo stack, controlli, salti) e per il loro riconoscimento in
  `makePCode()` e in `./asm`.

## Impact

- `pcodes.h` (`P_FOR_TO 108`, `P_FOR_DOWNTO 109`, `P_FOR_TEST 110`, `P_FOR_NEXT 111`,
  `MAX_PCODE 111`), `pcodes.c` (`pcodetxt`).
- `parser.c`: nuova struct per portare label e nome della variabile fra le riduzioni.
- `cpl2.gram` (sezione `--- for statement ---`), `cpl2.c`/`cpl2.h` rigenerati.
- `interpreter.cpp`: quattro classi `pcodeFor*`, i `case` in `makePCode()`, un metodo di
  `contextObj` per sapere se un nome è nel contesto corrente.
- `asm.c`: nessuna modifica al codice, legge `MAX_PCODE`.
- Formato del `.pcd` invariato (header `#cpl20.0.1`): un `.pcd` vecchio con un `for` conteneva
  comunque un `COMMENT` non eseguibile.
- `q.cpl` (già pronto), `test/`, `AGENTS.md`.
