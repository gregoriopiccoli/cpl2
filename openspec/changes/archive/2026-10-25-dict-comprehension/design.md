# Design

## Context

Vedi `proposal.md` (Why). Lo stato da cui si parte:

- L'array comprehension (`cpl2.gram`, sezione costanti) fa già tutto il lavoro difficile: il
  marcatore `lsq` registra con `emit_pos()` l'inizio del p-code dell'espressione, `compr_in` segna
  in `foreachloop.mark` la fine dell'espressione ed emette `ARRAY 0`, `foreachTesta()` (`parser.c`)
  emette `FOREACH_INIT`/`FOREACH_VAR`/`FOREACH_KEY`/`LABEL`/`FOREACH_NEXT`, e la regola finale
  sposta con `emit_rotate(S,H.mark)` il blocco dell'espressione dentro il ciclo, prima di
  `COMPR_APPEND`/`GOTO`/`LABEL`/`POP`.
- Il formato binario: `emit()` scrive solo il codice, `emit_i()` codice + intero; `makePCode()` legge
  l'operando con `atoi(s)`, quindi un operando assente vale `0`. `pcodeForeachInit` e
  `pcodeComprAppend` oggi derivano da `pcode` e ignorano l'operando.
- Gli iteratori (`iterObj` in `interpreter.cpp`): `next(valore,chiave,interp)` restituisce false a
  fine collezione e calcola la chiave solo se `nomeChiave>=0`; `haChiave()` è false solo per le
  tuple. Né `FOREACH_INIT` né `COMPR_APPEND` sono in `pcodejump[]`, passano dal dispatch virtuale.
- `dictObj::storeslice(idx,v)` fa già `map[idx->print()]=v`: è la stessa conversione della chiave
  usata da `DICT n` (`storekey(print(),...)`), e sovrascrive una chiave esistente.
- `cpl2.gram` ha sei regole `dict_compr` che iniziano con `LBR` come `dict_const`; le due a una
  variabile sono commentate (fuori scope, vedi proposal).

## Goals / Non-Goals

**Goals:**
- Zero p-code nuovi: `pcodes.h`, `pcodes.c`, `MAX_PCODE` e `asm.c` invariati.
- Il p-code già generato oggi (statement `for each`, array comprehension) resta byte per byte
  identico: i due p-code estesi continuano a essere emessi senza operando dove lo erano.
- Riuso dello schema dell'array comprehension: stesso marcatore, stesso `emit_rotate()`, stessa
  `foreachTesta()`.

**Non-Goals:**
- Forma parallela nello statement `for each` o nell'array comprehension (la grammatica non la ha;
  il p-code lo permetterebbe senza altre modifiche all'interprete).
- Ordine di stampa dei dizionari (resta quello della hash map).

## Decisions

### D1. Sequenza di p-code

Forma con chiave, `{ke:ve for each v,k in x with c}`:

```
DICT 0                      ; risultato
<x>
FOREACH_INIT
FOREACH_VAR v
FOREACH_KEY k
LABEL Ltest
FOREACH_NEXT Lend
<c> IF_FALSE Ltest          ; solo con with
<ke> <ve>                   ; spostati qui da emit_rotate()
COMPR_APPEND 2
GOTO Ltest
LABEL Lend
POP                         ; l'iteratore; il dizionario resta sullo stack
```

Forma parallela, `{ke:ve for each v:w in A : B with c}`: identica tranne la testa,
`<A> <B> FOREACH_INIT 2 FOREACH_VAR v FOREACH_KEY w`. Lo stack durante il ciclo è sempre
`[..., dict, iteratore]`, come nell'array comprehension.

### D2. `COMPR_APPEND 2` invece di un p-code nuovo o di `STORESLICE`

`COMPR_APPEND` riceve l'operando (classe che deriva da `ipcode`, `makePCode()` passa `atoi(s)`):
`0` → comportamento attuale (append all'array in `sp-2`); `2` → `dict(sp-3)->storeslice(key(sp-1),
val(sp))` e due pop. L'operando dice quanti valori consuma oltre all'iteratore (con 0 che vale 1 per
compatibilità con il p-code già emesso), così il p-code resta autodescrittivo nel listato di `asm`.
L'array comprehension continua a emettere `emit(P_COMPR_APPEND)`.

Alternative scartate:
- `STORESLICE`: vuole `[dict, chiave, valore]` in cima e lascia il valore sullo stack; il dizionario
  è sotto l'iteratore e non esiste un p-code per duplicarlo o per riordinare lo stack. Servirebbero
  `DUP`/`SWAP`/`ROT` nuovi, cioè più p-code nuovi e più istruzioni per elemento.
- Un `COMPR_INSERT` separato: un p-code in più (quattro modifiche coordinate) per un comportamento
  che differisce solo per il tipo del contenitore — contro la richiesta di riciclare.
- Decidere array/dizionario guardando il tipo del contenitore in `sp-2`: con il dizionario in `sp-2`
  c'è l'iteratore, e il test sui tipi a runtime sarebbe fragile; l'operando lo fissa il compilatore.

### D3. Visita parallela con `FOREACH_INIT 2` e un iteratore composto

`FOREACH_INIT` riceve l'operando: `0` → oggi; `2` → crea i due iteratori ordinari (stessa funzione
di oggi, quindi stesso errore su un valore non iterabile), li combina in un `zipIter` (nuova
sottoclasse di `iterObj`) e lo mette al posto delle due collezioni (un pop). `zipIter::next()`
chiama `next()` di entrambi con `nomeChiave=-1` (nessuna chiave calcolata, nessuna allocazione
inutile): restituisce in `valore` l'elemento della prima collezione e in `chiave` quello della
seconda; se finiscono entrambi restituisce false, se ne finisce uno solo solleva l'errore di
lunghezze diverse con la stessa `forErrore()` (messaggio su stdout + `domain_error`). `haChiave()`
è sempre true, perché la "chiave" è un valore della seconda collezione anche quando è una tupla.
`mark()`/`expand()` propagano a entrambi gli iteratori interni (che a loro volta raggiungono le
collezioni).

Così `FOREACH_VAR`, `FOREACH_KEY` e `FOREACH_NEXT` non cambiano, e i controlli sulle variabili
(contesto corrente, `--typecheck` di `STORE`) valgono identici.

Alternative scartate:
- Due iteratori sullo stack e un `FOREACH_NEXT` a profondità: cambierebbe il layout dello stack
  usato da `COMPR_APPEND` e da `POP` finale e richiederebbe un p-code nuovo o un secondo operando.
- Un p-code `FOREACH_ZIP` dopo due `FOREACH_INIT`: un p-code nuovo per lo stesso risultato.
- Fermarsi alla più corta (zip di Python): scartato dall'utente, il disallineamento deve essere un
  errore.

Il controllo della lunghezza avviene al passo in cui una delle due finisce: gli elementi precedenti
sono già stati inseriti nel dizionario, ma l'errore interrompe il programma prima che il dizionario
venga stampato o assegnato (non esiste ancora `try`/`catch` che potrebbe osservarlo).

### D4. Grammatica

- Marcatore `lbr(N) ::= LBR. {N=emit_pos();}` condiviso da `dict_const` e `dict_compr`, come `lsq`
  per gli array: dopo la `{` il parser non deve scegliere niente, e la posizione è presa prima che
  esca il p-code della prima espressione. `dict_const` usa `lbr` al posto di `LBR` con le stesse
  azioni (p-code invariato).
- Teste dedicate alle dict comprehension, perché `foreach_in` comprende anche la forma a una
  variabile che qui deve restare un errore di sintassi e perché la forma parallela ha due
  collezioni:
  ```
  dict_compr ::= lbr(S) expr COLON expr dcompr_head(H) RBR.
       {emit_rotate(S,H.mark);emit_i(P_COMPR_APPEND,2);emit_i(P_GOTO,H.start);emit_i(P_LABEL,H.last);emit(P_POP);}
  dcompr_head ::= dcompr_src | dcompr_src WITH expr      (IF_FALSE come compr_head)
  dcompr_src  ::= dcompr_in expr                          {foreachTesta}
  dcompr_src  ::= dcompr_zip expr COLON expr              {foreachTesta, con FOREACH_INIT 2}
  dcompr_in   ::= FOR EACH ID COMMA ID IN.                {name,key; mark=emit_pos(); DICT 0}
  dcompr_zip  ::= FOR EACH ID COLON ID IN.                {name,key; mark=emit_pos(); DICT 0}
  ```
  `expr` non contiene `COLON` al livello più esterno, quindi `A : B` non è ambiguo (un `:` dentro
  `A` è sempre racchiuso in `{}`).
- `foreachloop` riceve un campo `n` (numero di collezioni, 1 o 2); `foreachTesta()` emette
  `emit(P_FOREACH_INIT)` per 1 — come oggi — ed `emit_i(P_FOREACH_INIT,2)` per 2. Lo statement e
  l'array comprehension impostano `n=1` in `foreach_in`.
- Le regole commentate a una variabile restano commentate; la riga di commento "forse si potrebbe
  fare anche così" viene sostituita da un commento che descrive la forma parallela.
- Criterio di accettazione: `lemon cpl2.gram` esce con 0 e senza conflitti; se un conflitto
  emergesse fra `dict_list_elem ::= expr COLON expr` e la comprehension, il punto di decisione è il
  lookahead dopo la seconda `expr` (`COMMA`/`RBR` contro `FOR`), che LALR(1) distingue.

### D5. Chiavi e tipi

La chiave passa da `storeslice()`, quindi `print()` del valore come `DICT n`: `{k:x ...}` con `k`
intero produce la chiave `"0"`, letta con `d["0"]` (o con `d[0]`, che fa la stessa conversione).
Il dizionario prodotto non è tipizzato, come la costante: con `--typecheck` un `dict[T]` accetta
qualunque dizionario, come oggi.

Nota su `q.cpl`: il secondo esempio assegna stringhe a `k`, che `q.cpl` dichiara `int`. Senza
`--typecheck` (il modo in cui `q.cpl` si esegue a mano) funziona; con `--typecheck` sarebbe
un errore di tipo di `STORE`, corretto. `q.cpl` non va modificato; i test in `test/` (che girano
sempre con `--typecheck`) dichiarano le variabili con il tipo giusto.

## Risks / Trade-offs

- [Ordine di stampa non specificato] → i test completi stampano dizionari con una sola chiave, o
  leggono i valori per chiave (`d["alfa"]`); i casi a più chiavi di `q.cpl` si verificano a mano
  (spec: "un dizionario con le coppie ...").
- [Operando su p-code che prima non lo avevano] → l'operando assente vale 0 con `atoi`, quindi i
  `.pcd` già compilati si eseguono identici; si controlla che `./asm` mostri `COMPR_APPEND 2` e
  `FOREACH_INIT 2` e che la conversione inversa del listato resti byte-exact.
- [Errore di lunghezza scoperto a metà] → le coppie già inserite stanno in un dizionario che non è
  ancora raggiungibile dal programma; documentato in D3.
- [`emit_rotate` con espressioni annidate] → stesso meccanismo già verificato dall'array
  comprehension (le rotazioni interne avvengono prima dell'esterna e restano dentro `[S,fine)`);
  coperto da un test con comprehension annidate.
- [`zipIter` su un dizionario che cambia nel corpo] → i due iteratori interni mantengono le regole
  attuali (chiavi del dizionario copiate all'inizio, array visitati vivi): nessun caso nuovo.
