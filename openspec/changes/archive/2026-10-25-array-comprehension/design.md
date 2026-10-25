# Design

## Context

Motivazione in `proposal.md`, requisiti in `specs/array-comprehension/spec.md` e
`specs/interpreter-pcodes/spec.md`. Stato attuale rilevante:

- `cpl2.gram`, sezione `--- costanti ARRAY, DIZIONARI e TUPLE`: quattro regole monolitiche
  `array_compr ::= LSQ expr FOR EACH ID [COMMA ID] IN expr [WITH expr] RSQ.` con la sola azione
  `COMMENT "da fare array comprehension ..."`. `array_const ::= LSQ [array_list] RSQ` comincia con
  lo stesso token.
- Il `for each` (archiviato in `2026-10-25-implement-foreach-statement`) ha già lo schema di ciclo
  `FOREACH_INIT` / `FOREACH_VAR` / `[FOREACH_KEY]` / `LABEL Ltest` / `FOREACH_NEXT Lend` /
  `[<cond> IF_FALSE Ltest]` / corpo / `GOTO Ltest` / `LABEL Lend` / `POP`, con l'iteratore sullo
  stack per tutto il ciclo, e i non-terminali `foreach_in` (porta i due nomi) e `foreach_src`
  (emette la testa del ciclo) con la struct `foreachloop` in `parser.c`.
- Lemon è LALR senza azioni a metà regola e il p-code esce **in ordine di riduzione**: in
  `[e for each x in c]` il codice di `e` viene emesso prima che il parser sappia che si tratta di
  una comprehension, e prima del codice di `c`.
- `emit()/emit_i()/emit_s()/emit_line()/emit_ver()` in `emit.c` scrivono subito con `fprintf` su
  `emit_out`, che in modalità stdin è `stdout` (spesso una pipe, non riposizionabile). `emit.c` non è
  usato da `interpreter.cpp` oggi, ma è un file condiviso: deve restare C e C++ validi.
- `emit_line()` viene chiamata dal ciclo di `parse()` all'inizio di ogni riga, *prima* che i token
  di quella riga provochino le riduzioni in sospeso: una `LINE` può quindi cadere in mezzo al codice
  di un'espressione su più righe.
- Le label sono simboliche: `pcodeProgram` ne ricava la posizione al caricamento, scorrendo i
  `LABEL n`. Spostare blocchi di p-code non invalida i salti.
- Nessun p-code esistente aggiunge un elemento a un array: `ARRAY n` costruisce un array da `n`
  valori sullo stack (n noto a compile time), `STORESLICE` scrive su un indice esistente
  (`a[pos]=v` senza controllo dei limiti) e vuole il contenitore sotto indice e valore; non ci sono
  `DUP`/`SWAP`. `arrayObj` ha già un metodo `append()`.

## Goals / Non-Goals

**Goals:**
- Riusare tali e quali i p-code e la grammatica del `for each` (anche la stessa azione per la testa
  del ciclo), aggiungendo un solo p-code.
- Nessun costo per le costanti array e per il resto del programma: niente `GOTO` o `LABEL` in più
  fuori dalle comprehension.
- Un meccanismo di riordino del p-code riusabile dalle dict comprehension.

**Non-Goals:**
- Dict comprehension, scope propri, generatori (vedi proposal).
- Inserire `COMPR_APPEND` in `pcodejump[]`.
- Pre-allocare l'array risultato (la lunghezza non è nota: il filtro scarta elementi).

## Decisions

### D1. Forma del p-code

```
    ARRAY 0                 ; risultato vuoto                       stack: ... r
    <collezione>            ; valutata una volta                    stack: ... r coll
    FOREACH_INIT            ;                                       stack: ... r it
    FOREACH_VAR v
    [FOREACH_KEY k]
    LABEL Ltest
    FOREACH_NEXT Lend       ; finito: salta; altrimenti v:=valore (k:=chiave)
    [<condizione>
     IF_FALSE Ltest]        ; elemento scartato: e non viene valutata
    <e>                     ;                                       stack: ... r it val
    COMPR_APPEND            ; r.append(val)                         stack: ... r it
    GOTO Ltest
    LABEL Lend
    POP                     ; l'iteratore                           stack: ... r
```

L'array risultato resta **sotto** l'iteratore, così `FOREACH_NEXT` trova l'iteratore in cima come
nel `for each`, e al `POP` finale in cima resta proprio il valore dell'espressione. Lo stato è tutto
sullo stack: annidamento e ricorsione funzionano senza altro.

**Esempio** — `var int x` / `? [x for each x in [1,2,3] with x>1]` (righe `LINE` omesse):

```
ARRAY 0
INT_CONST 1
INT_CONST 2
INT_CONST 3
ARRAY 3
FOREACH_INIT
FOREACH_VAR x
LABEL 0
FOREACH_NEXT 1
LOAD x
INT_CONST 1
GT
IF_FALSE 0
LOAD x
COMPR_APPEND
GOTO 0
LABEL 1
POP
PRINT 1
```

### D2. Un p-code nuovo, `COMPR_APPEND` (116)

Layout `[..., r(sp-2), it(sp-1), val(sp)] -> [..., r, it]`: `r->append(val)` su `arrayObj`, poi
`pop` del valore. Senza operando (`pcode`, come `FOREACH_INIT`). Il cast di `r` può essere
`static_cast`: solo il compilatore emette questa sequenza e `r` viene sempre da `ARRAY 0`.

Alternative scartate, tutte senza p-code nuovi:
- *valori accumulati sullo stack e `ARRAY n` finale*: `n` non è noto a compile time e i valori
  finirebbero sopra l'iteratore, che `FOREACH_NEXT` vuole in cima.
- *`STORESLICE` con indice = lunghezza*: serve l'array in cima sotto indice e valore (mancano
  `DUP`/`SWAP`) e `storeslice` non fa crescere l'array.
- *variabile nascosta e concatenazione*: gli array non hanno `PLUS`, e una variabile nascosta
  andrebbe dichiarata nel contesto dell'utente (visibile, con nome riservato da inventare).

Il nome è legato alla comprehension apposta: il layout con l'iteratore in mezzo non ha senso
altrove. Una futura dict comprehension avrà il suo p-code gemello.

### D3. Riordino del p-code: buffer in memoria in `emit.c`

Il codice di `e` va spostato **dopo** la testa del ciclo. Il compilatore smette di scrivere subito:

- `emit.c` accumula tutto in un buffer `char*` che cresce (`realloc`); le cinque funzioni di
  emissione formattano lì (`vsnprintf` in un helper variadico) invece che su `emit_out`;
- `int emit_pos(void)`: lunghezza attuale del buffer, cioè la posizione del prossimo p-code;
- `void emit_rotate(int from, int mid)`: porta il blocco `[from,mid)` in fondo, dopo `[mid,fine)`
  (copia temporanea di `[from,mid)`, `memmove` del resto, copia in coda);
- `void emit_flush(void)`: scrive il buffer su `emit_out` e lo azzera. `parse()` la chiama alla
  fine, prima di restituire gli errori, quindi ogni sorgente esce intero nel suo `.pcd` come oggi.

Alternative scartate:
- *`GOTO`/`LABEL` intorno a `e`*: bisognerebbe emetterli alla `[`, quando il parser non sa ancora
  se arriverà una costante o una comprehension: o un conflitto LALR, o un salto in più per ogni
  costante array del programma.
- *`ftell`/`fseek` su `emit_out`*: non funziona con `stdout` in modalità stdin (pipe).
- *`open_memstream`*: solo POSIX, e modificare in place il buffer di uno stream aperto non è un
  comportamento documentato.

Le posizioni restano valide con l'annidamento: `emit_rotate` permuta byte solo dentro
`[from,fine)` e non cambia la lunghezza; i segni di una comprehension esterna sono tutti o prima
dell'inizio di quella interna (interna nella collezione o nella condizione) o dopo la sua fine
(interna in `e`: il segno di fine di `e` viene preso quando `e` è già ridotta del tutto).
Le `LINE` che cadono dentro `e` (comprehension su più righe) si spostano con lei: dentro il corpo
indicano la riga dell'espressione, che è quella giusta per gli errori.

### D4. Grammatica

```
%type lsq        {int}
%type compr_in   {foreachloop}
%type compr_src  {foreachloop}
%type compr_head {foreachloop}

lsq(N) ::= LSQ.                                  {N=emit_pos();}
array_const ::= lsq RSQ.                         {emit_i(P_ARRAY,0);}
array_const ::= lsq array_list(N) RSQ.           {emit_i(P_ARRAY,N);}

array_compr ::= lsq(S) expr compr_head(H) RSQ.   {emit_rotate(S,H.mark);emit(P_COMPR_APPEND);
                                                  emit_i(P_GOTO,H.start);emit_i(P_LABEL,H.last);emit(P_POP);}
compr_head(N) ::= compr_src(P).                  {N=P;}
compr_head(N) ::= compr_src(P) WITH expr.        {N=P;emit_i(P_IF_FALSE,N.start);}
compr_src(N) ::= compr_in(P) expr.               {N=P;<testa del ciclo, come foreach_src>}
compr_in(N) ::= foreach_in(P).                   {N=P;N.mark=emit_pos();emit_i(P_ARRAY,0);}
```

- `lsq` sostituisce `LSQ` in **entrambe** le alternative che iniziano un'espressione con `[`, così
  dopo la `[` il parser non deve scegliere nulla; `lsq` si riduce quando arriva il primo token
  dell'interno, prima che ne esca qualunque p-code. Per la costante array il valore è ignorato.
- Le quattro regole `array_compr` diventano una sola: `foreach_in` porta già uno o due nomi e
  `compr_head` il `with` facoltativo.
- `compr_in` si riduce quando arriva il primo token della collezione: `e` è già tutta emessa,
  quindi `N.mark` è la fine di `e`, e `ARRAY 0` esce subito dopo. Al momento di `array_compr` il
  buffer da `S` in poi è `e | ARRAY 0 | coll | testa | [cond IF_FALSE]` e `emit_rotate(S,H.mark)` lo
  rende `ARRAY 0 | coll | testa | [cond IF_FALSE] | e`.
- La testa del ciclo (`FOREACH_INIT`, `FOREACH_VAR`, `FOREACH_KEY`, le due label, `LABEL Ltest`,
  `FOREACH_NEXT Lend`, liberazione dei nomi) diventa una funzione C in `parser.c`, chiamata sia
  da `foreach_src` sia da `compr_src`: nessuna duplicazione, stesso p-code per lo statement.
- `foreachloop` riceve un campo `int mark` (usato solo dalla comprehension).
- Dopo `lsq expr`, `FOR` non è nel seguito di `array_list` (che vuole `COMMA` o `RSQ`), quindi la
  scelta fra costante e comprehension la fa il lookahead senza conflitti. La verifica resta
  `lemon` che esce 0 e `cpl2.out` senza conflitti.

### D5. Semantica a runtime

Tutta ereditata dai p-code del `for each` (variabili locali, `--typecheck` sull'assegnazione,
ordine di visita, errori su valori non iterabili e su chiave di tupla, `with` che deve essere
`bool`). La comprehension non aggiunge controlli propri. Il risultato è un `arrayObj` non
tipizzato, come una costante array: `a:=[...]` con `var array[int] a` passa `--typecheck` come
`a:=[1,2,3]`.

## Risks / Trade-offs

- [Il `.pcd` esce tutto a fine sorgente invece che riga per riga] → nessun effetto osservabile: il
  compilatore non legge mai il proprio output e una compilazione fallita oggi già lascia un `.pcd`
  senza `PCODEEND`; ora lo lascia comunque completo fino al punto dell'errore. Memoria: pochi KB per
  i sorgenti attuali.
- [Qualcosa scrive su `emit_out` senza passare dalle funzioni di `emit.c`] → finirebbe fuori
  ordine. Oggi in `parser.c` e `cpl2.gram` non c'è (le diagnostiche vanno su stderr con `diag()`);
  il task 1 lo ricontrolla con `grep`.
- [`emit_rotate` sbaglia i confini e sposta mezzo p-code] → i confini sono sempre inizi di riga
  (ogni emissione termina con `\n`); il listato di `./asm` dell'esempio D1 è la verifica.
- [GC: `COMPR_APPEND` fa puntare un array forse già promosso a un valore giovane] → è la stessa
  mutazione che oggi fa `STORESLICE`; il test include una comprehension con abbastanza elementi
  allocati (stringhe) da far partire più collect durante il ciclo.
- [`lsq` cambia le regole di `array_const`] → stesso p-code di prima; i test esistenti con
  costanti array (`foreach`, `type_check`, ...) lo verificano.
