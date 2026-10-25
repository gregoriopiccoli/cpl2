# Design

## Context

Motivazione in `proposal.md`. Stato attuale rilevante:

- `cpl2.gram`, sezione `--- switch statement ---`: le regole `switch_stm`, `case_list`,
  `case_else`, `case_case`, `case_const_list`, `case_const` esistono ma solo `switch_stm` ha
  un'azione (`COMMENT "da fare switch ..."`); `case_const` non emette nulla. Il lexer conosce
  già `switch`, `case`, `else`, `end`.
- I salti usano label numerate: `sState->labelCnt++` alloca la label, `LABEL n` la marca (NOP a
  runtime, `pcodeProgram` registra la posizione al caricamento), `GOTO n` e `IF_FALSE n` saltano.
  `if` e `while` sono il modello: le label viaggiano come valore sintetizzato dei non-terminali
  (`%type ... {int}` o `{ifelseif}`), perché lemon è LALR e non ha attributi ereditati.
- Ogni azione emette **in ordine di riduzione**: il p-code va pensato come una sequenza lineare
  prodotta mentre il parser legge, senza back-patching.
- `pcodeEq` usa `obj::eq`, che per `int`/`str`/`bool` **solleva** un errore se l'altro operando
  è di tipo diverso (`check_int`, `check_str`, `check_bool`); `nilObj::eq` invece restituisce
  `false`.
- Il compilatore emette `LINE n` all'inizio di ogni riga sorgente: compaiono intercalati nel
  p-code reale e sono omessi negli esempi qui sotto.

## Goals / Non-Goals

**Goals:**
- Generare lo `switch` in un solo passaggio LALR, senza back-patching e senza variabili nascoste.
- Valutare il selettore una volta sola e tenerlo sullo stack durante i confronti.
- Un solo p-code nuovo; le costanti riusano `INT_CONST`/`STR_CONST`/`TRUE`/`FALSE`.

**Non-Goals:**
- Tabelle di salto o ricerca hash per `switch` con molti `case` (ricerca lineare).
- `case` con espressioni o intervalli (la grammatica accetta solo `INT_CONST`, `-INT_CONST`,
  `STR_CONST`, `true`, `false`).
- Inserire `CASE` in `pcodejump[]` (non è un p-code caldo).

## Decisions

### D1. Nuovo p-code `CASE label` (`P_CASE 107`)

Semantica, con lo stack `... sel const`:

```
CASE L:   c := pop()            ; la costante del case
          s := top()            ; il selettore, NON viene tolto
          if stessoTipo(s,c) and s = c  then  pc := pos(LABEL L)
```

Effetto sullo stack: −1 sempre. Il selettore resta in cima sia col salto sia senza, quindi i
`CASE` di uno stesso `switch` si concatenano senza p-code di servizio.

Alternative scartate:
- **`DUP` + `EQ` + `IF_TRUE`** (tre p-code per costante): `IF_TRUE` non è implementato, serve un
  `DUP` nuovo comunque, ed `EQ` solleva errore su tipi diversi, il che rende impossibile
  `case 1,2,3` e `case 'a'` nello stesso `switch` (`test/test_stm1.cpl`).
- **Variabile nascosta** (`VAR_STORE $sw1` + `LOAD $sw1` + `EQ` + `IF_FALSE` per ogni costante):
  inquina il contesto con nomi generati, collide negli `switch` annidati/ricorsivi, stesso
  problema di `EQ` sui tipi.
- **Un unico p-code `SWITCH` con tabella nell'operando**: il formato `.pcd` ha un solo operando
  intero o stringa per riga; servirebbe una codifica nuova e una modifica a `asm.c`.

### D2. Uguaglianza in `CASE`: tipi diversi = non uguali

`pcodeCase::exec` confronta prima il tipo dinamico dei due oggetti (`typeid`): le costanti sono
sempre `intObj`, `strObj` o `boolObj`, e un selettore `nil` (`nilObj`) non coincide mai. Solo se
coincide chiama `sel->eq(const)`, che a quel punto non può sollevare errori.
`getBaseType()` non basta: `strObj` non lo ridefinisce e restituisce `theNil` come `nilObj`,
quindi non distinguerebbe una stringa da `nil`.
Alternativa scartata: chiamare `eq` direttamente e lasciar uscire l'eccezione — vieterebbe i
`case` misti che la grammatica permette e che `test_stm1.cpl` già usa.

### D3. Forma del p-code generato

Per uno `switch` con *k* case e un `else`:

```
<selettore>                      ; stack: sel
; --- case i: c1, c2, ..., cn ---
<c1>  CASE Lbody_i               ; uguale -> corpo (sel ancora sullo stack)
<c2>  CASE Lbody_i
...
<cn>  CASE Lbody_i
GOTO Lnext_i                     ; nessuna costante corrisponde -> case seguente
LABEL Lbody_i
POP                              ; il corpo parte con lo stack pulito
<stm_list del case i>
GOTO Lend
LABEL Lnext_i
; --- ripetuto per ogni case ---
; --- else (o else implicito) ---
POP                              ; nessun case ha corrisposto: via il selettore
<stm_list dell'else>             ; assente se manca l'else
LABEL Lend
```

Invariante: da qualunque cammino, il selettore viene tolto **esattamente una volta** (o col
`POP` d'ingresso di un corpo o col `POP` dell'else), quindi lo `switch` è neutro sullo stack e
i corpi — incluso un `return`/uscita di funzione o uno `switch` annidato — vedono lo stack come
se lo `switch` non ci fosse.

Il `GOTO Lnext_i` subito seguito dalla fine dei test è inevitabile senza back-patching (la label
del corpo deve stare dopo i test); costa un salto per case non corrispondente.

### D4. Grammatica riscritta

Le label viaggiano come valori sintetizzati: `Lend` nasce alla riduzione di `SWITCH expr EOL`
e viene passato lungo la lista dei case; `Lbody` nasce con la prima costante; `Lnext` nasce alla
fine dei test.

```
// --- switch statement ---

%type switch_case_list {int}
%type case_head        {int}
%type case_test        {int}

switch_stm ::= switch_case_list(N) case_else END.              {emit_i(P_LABEL,N);}

switch_case_list(N) ::= SWITCH expr EOL.                        {N=sState->labelCnt++;}
switch_case_list(N) ::= switch_case_list(P) case_head(H) stm_list. {N=P;emit_i(P_GOTO,N);emit_i(P_LABEL,H);}

case_head(N) ::= case_test(T) EOL.                              {N=sState->labelCnt++;emit_i(P_GOTO,N);emit_i(P_LABEL,T);emit(P_POP);}

case_test(N) ::= CASE case_const.                               {N=sState->labelCnt++;emit_i(P_CASE,N);}
case_test(N) ::= case_test(P) COMMA case_const.                 {N=P;emit_i(P_CASE,N);}

case_else ::= .                                                 {emit(P_POP);}
case_else ::= else_tok stm_list.
else_tok ::= ELSE EOL.                                          {emit(P_POP);}

case_const ::= INT_CONST(value).                                {emit_s(P_INT_CONST,value);dispose(&value);}
case_const ::= MINUS INT_CONST(value).                          {char n[64];snprintf(n,sizeof(n),"-%s",value);emit_s(P_INT_CONST,n);dispose(&value);}
case_const ::= STR_CONST(value).                                {emit_s(P_STR_CONST,value);dispose(&value);}
case_const ::= TRUE.                                            {emit(P_TRUE);}
case_const ::= FALSE.                                           {emit(P_FALSE);}
```

Cambiamenti rispetto a oggi:
- `case_list`/`case_case`/`case_const_list` sostituiti da `switch_case_list`/`case_head`/
  `case_test`: la lista porta `Lend`, il test porta `Lbody`, la testa del case porta `Lnext`.
- `SWITCH expr EOL` diventa la base della lista (prima era inline in `switch_stm`), così `Lend`
  è allocata prima di qualunque corpo.
- `case_const` emette la costante (prima non emetteva nulla); le azioni sono identiche a quelle
  di `fact`.
- Nuova forma `MINUS INT_CONST` per le costanti negative: emette un solo `INT_CONST` con
  l'operando preceduto da `-` (`case -1` -> `INT_CONST -1`), non `INT_CONST 1` + `UMINUS`,
  così `CASE` trova sempre una sola costante sopra il selettore. L'interprete legge l'operando
  con `atoi`, che accetta il segno. Nessun conflitto: dopo `CASE`/`COMMA` il token `MINUS` può
  solo iniziare una costante.
- Il ramo `else` passa per `else_tok` perché il `POP` deve essere emesso **prima** delle sue
  istruzioni; l'`else` vuoto (`case_else ::= .`) emette il solo `POP` al momento in cui il
  parser vede `END`, cioè dopo `LABEL Lnext` dell'ultimo case.
- Sparisce `emit_s(P_COMMENT,"da fare switch ...")`.

Ordine di riduzione alla fine dell'ultimo case senza `else`, lookahead `END`:
`switch_case_list ::= switch_case_list case_head stm_list` (emette `GOTO Lend`, `LABEL Lnext`)
→ `case_else ::= .` (emette `POP`) → shift `END` → `switch_stm` (emette `LABEL Lend`). È l'ordine
richiesto da D3. `lemon` deve continuare a non segnalare conflitti: nessuno statement inizia con
`CASE`, `ELSE` o `END`, che sono i tre lookahead che chiudono la `stm_list` di un case.

### D5. Esempio completo

Sorgente:

```
var int x:=2
switch x
  case 1,2
    ? 'uno o due'
  case 'a'
    ? 'a'
  else
    ? 'altro'
end
```

Label: `Lend`=0 (alla riduzione di `switch x`), primo case `Lbody`=1 `Lnext`=2, secondo case
`Lbody`=3 `Lnext`=4. Listato `./asm` atteso (senza le righe `LINE`):

```
INT_TYPE
INT_CONST 2
VAR_STORE x
POP
LOAD x            ; selettore                      stack: x
INT_CONST 1
CASE 1            ; x=1 ? -> LABEL 1               stack: x
INT_CONST 2
CASE 1            ; x=2 ? -> LABEL 1               stack: x
GOTO 2            ; nessuna -> case seguente
LABEL 1
POP               ;                                stack: (vuoto)
STR_CONST uno o due
PRINT 1
GOTO 0            ; fine switch
LABEL 2
STR_CONST a
CASE 3            ; int vs str: non uguale, nessun errore
GOTO 4
LABEL 3
POP
STR_CONST a
PRINT 1
GOTO 0
LABEL 4
POP               ; else: via il selettore
STR_CONST altro
PRINT 1
LABEL 0
PCODEEND 0
```

Con `x`=2 si esegue `LOAD x`, `INT_CONST 1`, `CASE 1` (no), `INT_CONST 2`, `CASE 1` (sì) →
`LABEL 1`, `POP`, stampa `uno o due`, `GOTO 0`. Output: `uno o due`.

### D6. Interprete

- `pcodes.h`: `#define P_CASE 107`, `MAX_PCODE 107` (così `asm.c` lo trova in `FindPCode`).
- `pcodes.c`: `pcodetxt[P_CASE]="CASE";`.
- `interpreter.cpp`: `class pcodeCase: public ipcode` accanto a `pcodeIfFalse`, `exec` come in
  D1/D2; `case P_CASE:return new pcodeCase(atoi(s));` in `makePCode()`. Nessuna allocazione,
  quindi nessun impatto sul GC: la costante tolta dallo stack è posseduta dal p-code che l'ha
  prodotta.
- L'header `.pcd` resta `#cpl20.0.1`: aggiungere un p-code non rende illeggibili i vecchi file.

## Risks / Trade-offs

- [Ricerca lineare, due p-code per costante] → accettabile per gli `switch` attuali; una tabella
  di salto è un change separato che può sostituire la sequenza di `CASE` senza toccare la
  semantica.
- [`GOTO Lnext` + `LABEL Lbody` anche per un case a una costante] → un salto in più per case non
  corrispondente; ottimizzabile in seguito con un `CASE_NE` se servisse.
- [Conflitti LALR introdotti dalla riscrittura] → l'implementazione verifica che `lemon` esca 0
  senza conflitti (`cpl2.out`) prima di proseguire.
- [Tipi diversi silenziosamente non uguali] → nasconde un `case "1"` scritto per errore su un
  selettore int; coerente con lo scopo dei case misti, e un futuro type-check a compile time
  potrà segnalarlo come warning.
- [Uscita da un corpo con `return`/fine funzione] → nessun rischio: il selettore è già stato tolto
  dal `POP` d'ingresso del corpo.
