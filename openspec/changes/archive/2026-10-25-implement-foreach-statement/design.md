# Design

## Context

Motivazione in `proposal.md`, requisiti in `specs/foreach-statement/spec.md` e
`specs/interpreter-pcodes/spec.md`. Stato attuale rilevante:

- `cpl2.gram`, sezione `--- for each statement ---`: quattro regole monolitiche
  `foreach_stm ::= FOR EACH ID [COMMA ID] IN expr [WITH expr] EOL stm_list END.` con la sola azione
  `COMMENT`. Il lexer conosce già `each`, `in`, `with`. Le comprehension
  (`[expr FOR EACH ID IN expr WITH expr]`, `{k:v FOR EACH ...}`) usano gli stessi token in altre
  regole, anch'esse `COMMENT`.
- Lemon è LALR senza azioni a metà regola: il p-code esce in ordine di riduzione, quindi fra
  intestazione e corpo servono non-terminali intermedi che portano label e nomi (modello: il `for`,
  con `for_init`/`for_head` e la struct `forloop` in `parser.c`).
- Il formato `.pcd` ha **un solo operando** per p-code, intero *o* stringa.
- Il `for` numerico tiene il suo stato (fine, passo) **sullo stack** e controlla la località della
  variabile con `contextObj::isLocal()`; gli errori a runtime usano `forErrore()` (messaggio su
  stdout e `throw domain_error`).
- `STORE` non toglie il valore dallo stack (serve alle assegnazioni dentro le espressioni); uno
  statement `x:=e` è `STORE x` + `POP`.
- Collezioni in `interpreter.cpp`: `arrayObj` e `tupleObj` hanno un `vector<obj*>` (indici da 0);
  `dictObj` ha un `unordered_map<string,obj*>`: la chiave è il `print()` del valore usato come
  chiave (`{1:'uno'}` ha la chiave stringa `"1"`) e l'ordine di visita è quello della hash map.
- Lo stack dell'interprete è radice del GC (`stack_gc`): un oggetto sullo stack è vivo, e i suoi
  figli lo sono se il suo `mark()`/`expand()` li visita.
- Le chiamate di func/proc sono **ricorsive sul C++**: `procObj::call` salva `pc`/`prg`/`context`,
  esegue `interpreter.run()` fino a `END_PROC`/`END_FUNC` (che mettono `stop=true`) e ripristina.
- Nei listati che seguono i `LINE n` intercalati sono omessi; i numeri delle label sono quelli che
  il compilatore assegnerebbe a un sorgente che contiene solo quel ciclo.

## Goals / Non-Goals

**Goals:**
- Un solo schema di p-code per le quattro forme sintattiche, con lo stato del ciclo sullo stack
  (annidamento e ricorsione gratis, come il `for`).
- Il minimo lavoro per iterazione: un p-code di test che fa anche le assegnazioni.
- Un **protocollo di iterazione** nell'interprete abbastanza generale da accogliere i generatori
  senza toccare né la grammatica né i p-code del ciclo.

**Non-Goals:**
- Implementare generatori, `yield` o comprehension (D7 descrive solo il progetto).
- Inserire i nuovi p-code in `pcodejump[]`; si misurerà dopo con `--bench`.
- Iterare su stringhe, contesti/moduli o altri oggetti: errore, come da spec.
- Controlli a compile time (il compilatore non ha tabella dei simboli).

## Decisions

### D1. Forma del p-code

```
    <collezione>            ; expr, valutata una volta              stack: ... coll
    FOREACH_INIT            ; coll -> iteratore (errore se non iterabile) stack: ... it
    FOREACH_VAR v           ; v locale? registrato nell'iteratore
    FOREACH_KEY k           ; solo forma a due variabili; errore su tupla
    LABEL Ltest
    FOREACH_NEXT Lend       ; finito: salta a Lend; altrimenti v:=valore (k:=chiave)
    [<condizione>           ; solo con with
     IF_FALSE Ltest]        ; elemento scartato: torna al test
    <corpo>
    GOTO Ltest
    LABEL Lend
    POP                     ; l'iteratore
```

Il vincolo "un solo operando" è risolto separando i p-code che portano i **nomi** (eseguiti una
volta, in testa) da quello che porta la **label** (eseguito a ogni giro): i nomi vengono
memorizzati (come intern) dentro l'iteratore, così `FOREACH_NEXT` assegna direttamente nel
contesto. Per iterazione si eseguono `LABEL` (nop), `FOREACH_NEXT`, il corpo e `GOTO`: tre p-code
di servizio, come il `for` numerico.

**Esempio 1** — `for each i in [1,2,3]` / `? i` / `end`:

```
INT_CONST 1
INT_CONST 2
INT_CONST 3
ARRAY 3
FOREACH_INIT
FOREACH_VAR i
LABEL 0
FOREACH_NEXT 1
LOAD i
PRINT 1
GOTO 0
LABEL 1
POP
```

**Esempio 2** — chiave e filtro, `for each i,k in a with k<>1` / `? k,'=',i` / `end`:

```
LOAD a
FOREACH_INIT
FOREACH_VAR i
FOREACH_KEY k
LABEL 0
FOREACH_NEXT 1
LOAD k
INT_CONST 1
NE
IF_FALSE 0              ; k=1: salta il corpo e torna al test (non a Lend)
LOAD k
STR_CONST =
LOAD i
PRINT 3
GOTO 0
LABEL 1
POP
```

**Esempio 3** — dizionario con chiave, `for each i,s in d` (`d:={"alfa":100,"beta":200}`): stesso
schema dell'esempio 2 senza il filtro; i due giri assegnano `i:=100, s:='alfa'` e
`i:=200, s:='beta'`, nell'ordine (non specificato) della hash map.

**Esempio 4** — tupla con chiave, `for each v,k in t` con `t` tupla: compila come l'esempio 2, e a
runtime `FOREACH_KEY k` interrompe il programma prima di `LABEL 0` (il compilatore non conosce il
tipo di `t`).

**Esempio 5** — annidamento, `for each i in x` con dentro `for each j in y`:

```
LOAD x / FOREACH_INIT / FOREACH_VAR i / LABEL 0 / FOREACH_NEXT 1 /
    LOAD y / FOREACH_INIT / FOREACH_VAR j / LABEL 2 / FOREACH_NEXT 3 /
        ...corpo interno... /
    GOTO 2 / LABEL 3 / POP /
GOTO 0 / LABEL 1 / POP
```

Durante il corpo interno lo stack contiene `it_x it_y`: ognuno dei `FOREACH_NEXT` lavora sempre e
solo sulla cima, quindi il ciclo interno non vede quello esterno.

Alternative scartate:
- **Iteratore che spinge valore e chiave**, seguito da `STORE v` / `POP` / `STORE k` / `POP`: niente
  `FOREACH_VAR`/`FOREACH_KEY` e la località si controllerebbe con un p-code generico, ma ogni giro
  costa quattro p-code in più, e `FOREACH_NEXT` dovrebbe comunque sapere se spingere la chiave
  (operando 1/2 al posto della label: impossibile, la label serve).
- **Indice sullo stack e `SLICE`** (`coll idx` sullo stack, `LOAD`/`SLICE`/`STORE`, `PLUS`):
  desugaring senza p-code nuovi, ma non funziona per i dizionari (serve la chiave i-esima, che la
  hash map non ha) né per i generatori, e la località andrebbe controllata altrove.
- **Un unico `FOREACH_INIT "v k"`** con i due nomi nell'operando separati da spazio: un p-code in
  meno, ma introduce un operando composto che `asm.c` e `loadPcd` non conoscono.
- **Variabile nascosta per l'iteratore nel contesto**: stessi difetti già discussi per il `for`
  (collisioni nella ricorsione, inquinamento del contesto); lo stack è il posto giusto.

### D2. Protocollo di iterazione: `iterObj`

Nuova classe astratta interna, mai visibile ai programmi (sta solo sullo stack fra
`FOREACH_INIT` e il `POP` finale):

```cpp
class iterObj : public obj {
public:
  int nomeValore=-1, nomeChiave=-1;          // intern registrati da FOREACH_VAR/FOREACH_KEY, -1 = assente
  virtual bool haChiave() const {return true;}
  // avanza: false se finito; altrimenti valore e chiave (la chiave solo se richiesta)
  virtual bool next(obj*& valore, obj*& chiave, interp& interpreter) = 0;
  virtual string print() const override {return "<iteratore>";}
};
```

Tre implementazioni: `arrayIter` (collezione + posizione; chiave `intObj` della posizione),
`tupleIter` (come l'array, `haChiave()` falso), `dictIter` (copia delle chiavi fatta all'inizio,
D3; chiave `strObj`). `mark()`/`expand()` visitano la collezione. `FOREACH_INIT` sceglie
l'implementazione con `dynamic_cast` sulla collezione; il parametro `interp&` di `next` non serve
alle collezioni ed è lì per i generatori (D7).

`FOREACH_NEXT` chiama `next`; se vero assegna con `interpreter.context->store(nome,…)`: dato che
il nome è stato trovato nel livello corrente da `FOREACH_VAR`, `store` lo trova subito e applica
il controllo di tipo `--typecheck` come `STORE`.

Visita **viva per posizione**: l'iteratore tiene l'indice, non una copia. A ogni `next` confronta
la posizione con la lunghezza *attuale*, quindi un elemento aggiunto nel corpo viene visitato
(e un corpo che aggiunge sempre non termina, come in Python), un elemento rimosso non fa uscire
dai limiti. Un'assegnazione `a[i]:=x` nel corpo è vista dai giri successivi. La riassegnazione
della *variabile* `a` non conta: l'iteratore tiene l'oggetto (spec "Collezione valutata una sola
volta").

Alternativa scartata: **copia della collezione** in `FOREACH_INIT` (semantica "snapshot"): costo
O(n) a ogni ciclo e allocazione in più per il GC, solo per regolare un caso (modifica durante la
visita) che oggi è raro e che con la visita per posizione non va comunque in crash.

### D3. Dizionari: ordine non specificato, chiavi copiate all'inizio

`dictObj` resta un `unordered_map<string,obj*>` e l'ordine di visita è quello della hash map,
**non specificato** (può cambiare con la crescita della tabella o con la libreria). Chi ha bisogno
di un ordine potrà chiedere l'array delle chiavi e ordinarlo (funzione da definire in un change
futuro).

`dictIter` non può tenere un iteratore della `unordered_map`: un'inserzione nel corpo può
provocare un rehash e invalidarlo (comportamento indefinito). `FOREACH_INIT` copia quindi le
**chiavi** in un `vector<string>` dentro l'iteratore; a ogni `next` prende la chiave successiva e
legge il valore *attuale* con `find` (una chiave che non c'è più verrebbe saltata: oggi non esiste
la rimozione). Conseguenze, scritte nella spec: le chiavi aggiunte nel corpo non vengono visitate,
un valore cambiato prima della visita si vede. È diverso dagli array (visita viva, D2), ma è il
comportamento sicuro per una hash map.

Alternative scartate:
- **Ordine di inserimento** (elenco ordinato delle chiavi in `dictObj`): deterministico, ma è una
  modifica della rappresentazione del dizionario che non serve al `for each`; rifiutato.
- **`std::map`**: ordine alfabetico come stringhe (`"10"` prima di `"9"`), accesso O(log n).
- **Iteratore della `unordered_map`**: niente copia, ma invalidato da un rehash durante il corpo.

### D4. Tipo della chiave

- Array: `intObj` con l'indice da 0, preso da `theIntCache` come gli altri interi.
- Dizionario: `strObj` con il testo della chiave memorizzata. È una conseguenza di come `dictObj`
  memorizza oggi le chiavi (`print()` del valore): `{1:'uno'}` dà la chiave `'1'`, non l'intero
  `1`. Conservare il tipo originale della chiave richiede di cambiare la rappresentazione del
  dizionario (chiave `obj*` con hash/uguaglianza), un change a sé.
- Tupla: nessuna chiave (spec "Le tuple non hanno chiave").

### D5. Località delle variabili e controlli

`FOREACH_VAR`/`FOREACH_KEY` usano `contextObj::isLocal()` come `FOR_TO`, con messaggio
`<nome>: la variabile del for each deve essere dichiarata nel contesto corrente`; gli errori
riusano `forErrore()`. Gli altri messaggi: `for each su un valore non iterabile: <print>` in
`FOREACH_INIT`, `<nome>: le tuple non hanno chiave` in `FOREACH_KEY`. Il controllo si fa una sola
volta per ciclo.

Le due variabili possono essere lo stesso nome (`for each x,x in a`): non è vietato, l'ultima
assegnazione (la chiave) vince. Non vale la pena di un controllo.

### D6. Grammatica

I nomi sono scritti **prima** della collezione ma servono **dopo** (`FOREACH_VAR` segue
`FOREACH_INIT`), quindi viaggiano in un non-terminale. Nuova struct in `parser.c` accanto a
`forloop`:

```c
typedef struct foreachloop {
	int start;          // Ltest
	int last;           // Lend
	char* name;         // variabile del valore
	char* key;          // variabile della chiave, NULL se assente
} foreachloop;
```

```
%type foreach_in   {foreachloop}
%type foreach_src  {foreachloop}
%type foreach_head {foreachloop}

foreach_stm ::= foreach_head(N) stm_list END.           {emit_i(P_GOTO,N.start);emit_i(P_LABEL,N.last);emit(P_POP);}
foreach_head(N) ::= foreach_src(P) EOL.                 {N=P;}
foreach_head(N) ::= foreach_src(P) WITH expr EOL.       {N=P;emit_i(P_IF_FALSE,N.start);}
foreach_src(N) ::= foreach_in(P) expr.                  {N=P;emit(P_FOREACH_INIT);emit_s(P_FOREACH_VAR,N.name);
                                                         if(N.key) emit_s(P_FOREACH_KEY,N.key);
                                                         dispose(&N.name);if(N.key) dispose(&N.key);
                                                         N.start=sState->labelCnt++;N.last=sState->labelCnt++;
                                                         emit_i(P_LABEL,N.start);emit_i(P_FOREACH_NEXT,N.last);}
foreach_in(N) ::= FOR EACH ID(value) IN.                {N.name=value;N.key=NULL;}
foreach_in(N) ::= FOR EACH ID(value) COMMA ID(key) IN.  {N.name=value;N.key=key;}
```

I nomi si liberano già in `foreach_src` (dopo l'emissione non servono più), quindi a differenza
del `for` non c'è leak su un errore di sintassi nel corpo. `foreach_src` si riduce col lookahead
`WITH` o `EOL`; le comprehension hanno prefissi diversi (`LSQ expr FOR EACH …`) e quindi stati
LALR diversi, ma l'assenza di conflitti va verificata in `cpl2.out` dopo `lemon`. Numeri:
`P_FOREACH_INIT 112`, `P_FOREACH_VAR 113`, `P_FOREACH_KEY 114`, `P_FOREACH_NEXT 115`,
`MAX_PCODE 115`.

### D7. Generatori (solo progetto, non in questo change)

Un generatore è una funzione il cui corpo restituisce più valori con `yield`, sospendendosi a
ognuno. Con il protocollo di D2 entra nel `for each` **senza cambiare grammatica del ciclo né
p-code del ciclo**: `FOREACH_INIT` riconosce un oggetto generatore e crea un `genIter`, e
`genIter::next` riprende il generatore fino al prossimo `yield`.

Sintassi ipotetica (da decidere nel change dei generatori). Il compilatore è a passata singola e
il prologo di una func è già emesso prima del corpo, quindi non può scoprire *dopo* che il corpo
contiene `yield`: serve una parola chiave in testa.

```
gen int conta(int n)
  var int i
  for i:=1 to n
    yield i
  end
end

var int x
for each x in conta(3)
  ? x
end
```

P-code del chiamante: identico a D1, la collezione è solo una chiamata.

```
LOAD conta
INT_CONST 3
CALL 1            ; chiamare un gen NON esegue il corpo: lascia un generatorObj
FOREACH_INIT      ; generatorObj -> genIter
FOREACH_VAR x
LABEL 0
FOREACH_NEXT 1    ; riprende il generatore fino al prossimo YIELD (o alla fine)
LOAD x
PRINT 1
GOTO 0
LABEL 1
POP
```

P-code del corpo del generatore (dopo il blocco dei parametri, come per una func):

```
...dichiarazione di i...
INT_CONST 1
LOAD n
INT_CONST 1
FOR_TO i
LABEL 0
LOAD i
FOR_TEST 1
LOAD i
YIELD             ; toglie il valore, salva lo stato, sospende  -- stack del gen: [fine, passo]
FOR_NEXT i
GOTO 0
LABEL 1
POP
POP
END_GEN           ; generatore esaurito
```

Nuovi p-code previsti: `GEN` (definizione, come `FUNC`), `YIELD`, `END_GEN` (o `END_FUNC` con uno
stato che lo distingue). Meccanica, ricalcata su `procObj::call`:

- **Chiamata** di un `genObj`: crea il contesto delle variabili e inizializza i parametri
  (`initParms`, default compresi) come una func, ma invece di eseguire il corpo mette sullo stack
  un `generatorObj` {funzione, contesto, `pc` di ripresa, segmento di stack salvato, ultimo valore,
  stato nuovo/sospeso/in esecuzione/finito}.
- **Ripresa** (`genIter::next` → `generatorObj::resume(interp&)`): salva `pc`/`prg`/`context`
  dell'interprete, imposta quelli del generatore, ricorda la base dello stack e ci **rispinge il
  segmento salvato**, poi `interpreter.run()`.
- **`YIELD`**: toglie il valore e lo mette in `ultimo`, sposta nel generatore il segmento di stack
  sopra la base (nell'esempio `fine` e `passo` del `for` interno: senza salvarli il `for` del
  generatore si romperebbe), memorizza `pc` e mette `stop=true`. `run()` ritorna a `resume`, che
  ripristina lo stato del chiamante e restituisce vero.
- **`END_GEN`**: stato finito, stack riportato alla base, `stop=true`; `next` restituisce falso e
  `FOREACH_NEXT` salta a `Lend`.
- Funziona con le chiamate ricorsive sul C++ perché `yield` sospende **solo il frame del
  generatore**: le chiamate fatte dal corpo sono già tornate quando si arriva a `YIELD`. Non c'è
  bisogno di coroutine C++ né di uno stack separato.
- **GC**: `generatorObj::mark()` visita funzione, contesto, segmento salvato e ultimo valore;
  `genIter` visita il generatore.
- Riprendere un generatore "in esecuzione" (il corpo che scorre se stesso) è un errore.

Domande che il change dei generatori dovrà chiudere (non toccano questo change):
- chiave con un generatore: contatore 0,1,2… (come `enumerate`) oppure errore come le tuple;
- un generatore è monouso: un secondo `for each` sullo stesso `g` non visita nulla, diversamente
  da un array;
- `return` anticipato dentro un `gen` (oggi non esiste `return`).

Le comprehension (`[x*2 for each x in a with x>0]`) potranno riusare `FOREACH_INIT`/`VAR`/`NEXT`
con un array accumulatore sotto l'iteratore, e quindi funzionare anche sui generatori.

## Risks / Trade-offs

- [Statement futuri che escono dal ciclo (`break`, `return`) lascerebbero l'iteratore sullo
  stack] → nessuno esiste oggi; dovranno emettere un `POP` per ogni `for each` attraversato (due
  per ogni `for`). Annotato in `AGENTS.md`.
- [Copia O(n) delle chiavi a ogni `for each` su un dizionario] → accettato; si eviterebbe solo
  con una struttura che non si invalida, cioè cambiando `dictObj`.
- [L'ordine dei dizionari non è deterministico] → i test sono scritti in modo che l'output non
  dipenda dall'ordine (somme, conteggi, dizionari a una chiave).
- [La chiave dei dizionari è sempre `str`] → limite della rappresentazione attuale, esplicitato
  nella spec; `var int k` come chiave di un dizionario con chiavi intere fallisce con
  `--typecheck`.
- [Un corpo che fa `append` sulla collezione che scorre non termina] → comportamento documentato
  della visita viva (D2), come in Python.

## Migration Plan

Nessuna: il formato `.pcd` e l'header restano invariati; i `.pcd` esistenti con un `for each`
contenevano un `COMMENT` non eseguibile e vanno solo ricompilati.
