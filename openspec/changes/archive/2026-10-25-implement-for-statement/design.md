# Design

## Context

Motivazione in `proposal.md`, requisiti in `specs/for-statement/spec.md` e
`specs/interpreter-pcodes/spec.md`. Stato attuale rilevante:

- `cpl2.gram`, sezione `--- for statement ---`: quattro regole monolitiche
  `for_stm ::= FOR ID ASSIGN expr TO|DOWNTO expr [STEP expr] EOL stm_list END.` con la sola azione
  `COMMENT "da fare for ..."`. Il lexer conosce già `for`, `to`, `downto`, `step`, `end`; la
  grammatica oggi non ha conflitti.
- Lemon è LALR senza azioni a metà regola: per emettere p-code fra l'intestazione e il corpo
  bisogna spezzare la regola in non-terminali intermedi il cui valore sintetizzato porta le label
  (modello: `while_expr`/`while_tok`, `switch_case_list`/`case_head`). Il p-code esce **in ordine
  di riduzione**, senza back-patching.
- Le label: `sState->labelCnt++` alloca, `LABEL n` marca (NOP a runtime), `GOTO n` salta.
- Il formato `.pcd` ha **un solo operando** per p-code, intero *o* stringa: un p-code non può
  portare insieme il nome della variabile e una label.
- `contextObj` (in `interpreter.cpp`) ha `objs`/`types` per i nomi del livello e `superlevel` per
  la ricerca all'indietro; `load`/`store` risalgono i livelli, `add` lavora solo sul livello
  corrente. Manca un modo per chiedere "il nome è in *questo* livello?".
- `STORE` passa per `contextObj::store`, che fa il controllo di tipo (`checkTipo`, attivo con
  `--typecheck`). Gli errori a runtime seguono lo schema di `checkTipo`: messaggio su stdout e
  poi `throw domain_error`, che non viene catturato e interrompe il processo con codice ≠ 0.
- `intObj::plus` ricicla gli interi tramite `theIntCache`: va bene anche per l'incremento.
- Il compilatore emette `LINE n` intercalati: omessi negli esempi.

## Goals / Non-Goals

**Goals:**
- Generare il `for` in un solo passaggio LALR, senza variabili nascoste nel contesto.
- Fine e passo valutati una volta e tenuti sullo stack durante il ciclo (come il selettore dello
  `switch`), così i `for` annidati e ricorsivi non interferiscono.
- Una sola forma di p-code per le quattro varianti sintattiche: `step` assente diventa una
  costante, `to`/`downto` cambia solo il p-code di inizializzazione.

**Non-Goals:**
- Controllo a compile time di variabile locale o dei tipi (il compilatore non ha tabella dei
  simboli).
- Inserire i nuovi p-code in `pcodejump[]`; si valuterà dopo, misurando con `--bench`.
- Uscita anticipata dal ciclo (`break`) o `result:=` che salta fuori lasciando fine/passo sullo
  stack: non esistono statement che lo facciano oggi.

## Decisions

### D1. Forma del p-code

```
    <inizio>                 ; expr
    <fine>                   ; expr
    <passo>                  ; expr, oppure INT_CONST 1 (to) / INT_CONST -1 (downto)
    FOR_TO i                 ; oppure FOR_DOWNTO i        stack: ... fine passo
    LABEL Ltest
    LOAD i                   ;                            stack: ... fine passo v
    FOR_TEST Lend            ; toglie v, salta se oltre   stack: ... fine passo
    <corpo>
    FOR_NEXT i               ; i := i + passo             stack: ... fine passo
    GOTO Ltest
    LABEL Lend
    POP                      ; passo
    POP                      ; fine
```

Esempio, `for i:=10 downto 1 step -2` (`UMINUS` sull'`INT_CONST 2` com'è oggi per `-2`
nelle espressioni):

```
INT_CONST 10 / INT_CONST 1 / INT_CONST 2 / UMINUS / FOR_DOWNTO i /
LABEL 0 / LOAD i / FOR_TEST 1 / LOAD i / PRINT 1 / FOR_NEXT i / GOTO 0 / LABEL 1 / POP / POP
```

Perché così:
- Il vincolo "un solo operando" obbliga a separare i p-code che portano il **nome**
  (`FOR_TO`/`FOR_DOWNTO`, `FOR_NEXT`) da quello che porta la **label** (`FOR_TEST`); il valore
  corrente arriva a `FOR_TEST` tramite un normale `LOAD i`.
- Il test in testa (`LABEL Ltest` prima del test) dà gratis le zero iterazioni quando l'inizio è
  già oltre la fine.
- La variabile viene riletta dal contesto a ogni giro (`LOAD i`, `FOR_NEXT i`), quindi
  un'assegnazione nel corpo ha effetto (spec "Variabile riletta a ogni iterazione"). Il valore
  dopo il ciclo è il primo oltre la fine.
- La pulizia sono due `POP` esistenti: nessun p-code `FOR_END`.

Alternative scartate:
- **Contatore sullo stack** (`fine passo cont`) copiato in `i` a ogni giro: le assegnazioni a `i`
  nel corpo verrebbero ignorate (semantica Python), ma servirebbe comunque un p-code con nome per
  la copia e uno con label per il test; più stato e nessun p-code in meno.
- **Variabili nascoste** `$end1`/`$step1` nel contesto: inquinano il contesto, collidono nei
  `for` ricorsivi e richiedono `VAR`/`STORE` di servizio; lo stack è già il posto giusto, come
  per lo `switch`.
- **Desugaring in `while`** con `LOAD`/`LE`/`IF_FALSE`/`PLUS`/`STORE`: niente p-code nuovi ma
  fine e passo andrebbero rivalutati a ogni giro (o messi in variabili nascoste), la direzione
  andrebbe decisa a compile time anche per un passo calcolato, e non ci sarebbe un punto dove
  controllare "variabile locale" e "passo coerente".
- **Un unico `FOR_TEST` che fa anche `LOAD`** con nome e label nell'operando (`"i 3"`): cambia la
  convenzione del formato e il parsing di `asm.c`.

### D2. Semantica del passo: incremento con segno, direzione da `to`/`downto`

Il passo è il valore sommato alla variabile; `to` richiede passo > 0, `downto` passo < 0
(`downto 1 step -2` in `q.cpl` lo impone). `FOR_TO`/`FOR_DOWNTO` controllano il segno una volta
all'inizializzazione; `FOR_TEST` sceglie il confronto dal segno del passo che trova sullo stack
(`> fine` se positivo, `< fine` se negativo), quindi non ha bisogno di sapere la direzione e
basta un solo p-code di test.

Alternative scartate:
- **Passo come modulo** (`downto` sottrae `|step|` o `step`): `downto 1 step -2` andrebbe verso
  l'alto, ciclo infinito sull'esempio dell'utente.
- **Passo di segno sbagliato = zero iterazioni** (come un ciclo "vuoto"): nasconde un errore
  quasi certo del programmatore; `step 0` sarebbe comunque infinito. Errore a runtime, coerente
  con gli altri controlli.

### D3. Variabile locale: nuovo `contextObj::isLocal(intern)`

`FOR_TO`/`FOR_DOWNTO` chiamano un nuovo metodo di `contextObj` che cerca il nome solo in `objs`
del livello corrente, senza risalire `superlevel`. Se manca: messaggio
`<nome>: la variabile del for deve essere dichiarata nel contesto corrente` su stdout e
`throw domain_error`, prima di assegnare. Il controllo si fa una volta, all'inizializzazione:
`FOR_NEXT` usa poi il normale `store` (che troverà il nome al primo livello) e quindi il controllo
di tipo dichiarato (`var int i`) con `--typecheck`.

Il "contesto corrente" è `interpreter.context` nel momento dell'esecuzione: il contesto del modulo
per il codice di livello esterno, il contesto di attivazione per il corpo di una func/proc
(i parametri compresi, che vi sono registrati con `add`).

Alternativa scartata: **dichiarazione implicita** (il `for` fa `add` se il nome manca). Il
requisito dell'utente chiede che la variabile sia già presente; inoltre un `add` a ogni
esecuzione del `for` fallirebbe la seconda volta (`name already in context`).

### D4. Controlli dei tipi nei p-code

`FOR_TO`/`FOR_DOWNTO` verificano con `dynamic_cast<const intObj*>` che inizio, fine e passo siano
`int` (errore altrimenti, anche per `nil`); `FOR_TEST` e `FOR_NEXT` verificano che il valore
corrente della variabile sia `int` (il corpo potrebbe averle assegnato altro, senza
`--typecheck`). Fine e passo non vanno ricontrollati: stanno sullo stack e nessun p-code del corpo
li tocca.

### D5. Grammatica

Il nome della variabile deve arrivare dall'intestazione fino alla riduzione finale (per
`FOR_NEXT`): nuova struct in `parser.c` accanto a `ifelseif`:

```c
typedef struct forloop {
	int start;          // Ltest
	int last;           // Lend
	char* name;         // variabile di ciclo, liberata alla fine del for
} forloop;
```

```
%type for_init {forloop}
%type for_head {forloop}

for_stm ::= for_head(N) stm_list END.   {emit_s(P_FOR_NEXT,N.name);emit_i(P_GOTO,N.start);
                                         emit_i(P_LABEL,N.last);emit(P_POP);emit(P_POP);dispose(&N.name);}
for_head(N) ::= for_init(P) EOL.        {N=P;N.start=sState->labelCnt++;N.last=sState->labelCnt++;
                                         emit_i(P_LABEL,N.start);emit_s(P_LOAD,N.name);emit_i(P_FOR_TEST,N.last);}
for_init(N) ::= FOR ID(value) ASSIGN expr TO expr.                {emit_s(P_INT_CONST,"1");emit_s(P_FOR_TO,value);N.name=value;}
for_init(N) ::= FOR ID(value) ASSIGN expr TO expr STEP expr.      {emit_s(P_FOR_TO,value);N.name=value;}
for_init(N) ::= FOR ID(value) ASSIGN expr DOWNTO expr.            {emit_s(P_INT_CONST,"-1");emit_s(P_FOR_DOWNTO,value);N.name=value;}
for_init(N) ::= FOR ID(value) ASSIGN expr DOWNTO expr STEP expr.  {emit_s(P_FOR_DOWNTO,value);N.name=value;}
```

Il token `ID` non viene liberato in `for_init`: la proprietà passa a `N.name` e `dispose` avviene
nella riduzione di `for_stm`. La scelta fra la forma con e senza `STEP` si decide sul lookahead
(`STEP` contro `EOL`), quindi non nascono conflitti; va comunque verificato in `cpl2.out` dopo
`lemon`. I numeri dei p-code: `P_FOR_TO 108`, `P_FOR_DOWNTO 109`, `P_FOR_TEST 110`,
`P_FOR_NEXT 111`, `MAX_PCODE 111`.

## Risks / Trade-offs

- [Un errore di sintassi dentro il corpo fa perdere `N.name`] → lemon scarta i simboli senza
  distruttore: è un leak di poche byte in una compilazione già fallita, come per gli altri `ID`
  della grammatica. Accettato.
- [Uscita dal ciclo senza passare da `LABEL Lend` lascerebbe fine e passo sullo stack] → oggi non
  esiste nessuno statement che lo faccia; quando arriveranno `break`/`return` dovranno emettere i
  due `POP` (annotato in `AGENTS.md`).
- [Overflow di `int` vicino a `INT_MAX` con `to INT_MAX`] → l'incremento oltre la fine va in
  overflow come ogni `PLUS` su `int`; non trattato, stesso comportamento delle espressioni.
- [Tre p-code per iterazione in più rispetto a un `while` ottimizzato] → nessun `pcodejump[]` per
  ora; se il `for` diventa caldo nei benchmark si aggiungono `FOR_TEST`/`FOR_NEXT` alla tabella.

## Migration Plan

Nessuna: il formato `.pcd` e l'header restano invariati; i `.pcd` esistenti con un `for`
contenevano un `COMMENT` non eseguibile e vanno solo ricompilati.
