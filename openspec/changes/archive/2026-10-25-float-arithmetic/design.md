# Design

## Context

Gli operatori binari sono già emessi dal compilatore (`term PLUS mterm` → `PLUS`, `rexpr ::= term
LT term` → `LT`, ...) e già eseguiti dall'interprete senza sapere nulla dei tipi:
`pcodePlus::exec` e il percorso veloce `p_plus:` di `run()` chiamano
`stack[sp-1]->plus(stack[sp])`, e lo stesso per `minus`, `mult`, `div`, `lt`, `le`, `eq`, `ge`,
`gt`, `ne`. Il tipo è tutto nei metodi virtuali di `obj`: la versione base lancia
`domain_error("<op> not implemented")`, `intObj` e `strObj` li ridefiniscono. `floatObj` oggi
ridefinisce solo `print`, `uminus` e `getBaseType`.

`intObj` controlla l'operando destro con `check_int()` (confronto di `getBaseType()` con
`theIntType`, poi `static_cast`) e lancia `integer + with a non integer` ecc. `plus`, `minus` e
`uminus` riciclano gli `intObj` liberi da `theIntCache`; il percorso `int + int` è quello caldo di
`fib`, misurato con `--bench`.

Il dispatch è singolo (sull'operando sinistro), quindi `1 + 0.5` arriva a `intObj::plus` e
`0.5 + 1` a `floatObj::plus`: la promozione va gestita in entrambe le classi.

## Goals / Non-Goals

**Goals:**
- Nessun costo aggiunto su `int op int` nel caso che riesce: il ramo float di `intObj` si paga
  solo quando l'operando destro non è un intero.
- Un solo posto dove si calcola in `double`, così `int op float` e `float op int` danno lo stesso
  risultato bit per bit.

**Non-Goals:**
- Doppio dispatch generale o una tabella di promozione fra tipi: c'è un solo caso (`int`/`float`).
- Toccare `pcodejump[]`, le classi `pcode*` o il compilatore.
- Un cache di riciclo per i `floatObj` come `theIntCache`.

## Decisions

### D1 — `floatObj` converte l'operando con un helper `check_num()`

`floatObj` ottiene un helper `double check_num(const obj* o, const char* msg) const`: se
`getBaseType()` di `o` è `theFloatType` restituisce il suo `value`, se è `theIntType` restituisce
`(double)` del valore intero, altrimenti lancia `domain_error(msg)`. Ogni operazione è una riga:
`plus` → `new floatObj(value + check_num(o,"float + with a non number"))`, i confronti →
`(value < check_num(o,...) ? theTrue : theFalse)`.

Messaggi sul modello di quelli di `intObj`/`strObj`: `float + with a non number`,
`float - ...`, `float * ...`, `float / ...`, `float = ...`, `float < ...`, `float <= ...`,
`float >= ...`, `float > ...`, `float <> ...`.

*Alternativa scartata:* `dynamic_cast` ripetuti in ogni metodo — il resto del codice usa già
`getBaseType()` per questo (`check_int`), e `dynamic_cast` è più lento.

### D2 — `intObj` delega al float solo quando `check_int` fallirebbe

Ogni operazione di `intObj` interessata (`plus`, `minus`, `mult`, `div`, `eq`, `lt`, `le`, `ge`,
`gt`, `ne`) guarda prima se `o` è un intero (il controllo che fa già `check_int`); se lo è, il
codice resta identico a oggi. Se invece `o->getBaseType()==theFloatType`, calcola in `double`
con `(double)value` a sinistra e il `value` del float a destra, passando per le stesse funzioni
libere usate da `floatObj` (vedi sotto). Altrimenti lancia il messaggio di oggi
(`integer + with a non integer`, ...), invariato.

Il calcolo vero sta in piccole funzioni libere sui `double`, una per operazione (per esempio
`static obj* floatPlus(double a,double b){return new floatObj(a+b);}`,
`static obj* floatLt(double a,double b){return a<b?theTrue:theFalse;}`), definite dopo
`theTrue`/`theFalse` e chiamate sia dai metodi di `floatObj` (dopo `check_num`) sia dal ramo
float di `intObj`: un unico punto di calcolo per `int op float`, `float op int` e
`float op float`.

*Alternativa scartata:* un `floatObj` temporaneo sullo stack C++ come operando sinistro
(`floatObj(value).plus(o)`) — il costruttore di `GCObject` (`gc.cpp`) registra ogni oggetto
nel collector con `GC::getGC().add(this)`, quindi il temporaneo finirebbe nella lista del GC e
verrebbe liberato con `delete` dopo essere già uscito di scope.

*Alternativa scartata:* invertire l'operazione (`o->plus(this)`) — sbagliata per `-`, `/` e i
confronti non simmetrici, e richiederebbe operatori "riflessi" come in Python.

Per non rallentare `int + int`: il test intero resta il primo e unico confronto nel caso caldo,
il ramo float è dopo. `check_int` resta per `mod` e `idiv`, che non cambiano, e per l'errore.

### D3 — Aritmetica sempre su oggetti nuovi, IEEE per la divisione

Come `floatObj::uminus`, ogni operazione aritmetica restituisce `new floatObj(...)`: le costanti
`FLOAT_CONST` sono condivise e non vanno mai modificate. La divisione è la `/` di `double` senza
controlli: per zero dà `inf`/`-inf`/`nan`, che `floatObj::print()` già stampa così
(`to_chars`, e il `.0` non viene aggiunto perché il testo contiene lettere). I confronti usano gli
operatori di `double`, quindi `nan` è diverso da tutto, anche da sé stesso, e `<>` su un `nan` è
`true`. `eq`/`ne` sui float sono esatti (`0.1 + 0.2 = 0.3` è `false`), nessuna tolleranza.

### D4 — `mod` e `idiv` restano come sono

`floatObj` non ridefinisce `mod` e `idiv`: `5.5 mod 2` continua a lanciare `mod not implemented`
dalla base `obj`, e `5 mod 2.0` / `5 div 2.0` continuano a lanciare il messaggio di `check_int`
(`integer % with a non integer`, `integer idiv with a non integer`). Nessuna riga da scrivere.

### D5 — Nessuna promozione fuori dagli operatori

`tipoCompatibile()` (assegnazioni con `--typecheck`), `tipoIs()`, `CASE` (che confronta prima i
`typeid`) e i controlli di `FOR_TO`/`FOREACH` non cambiano: `var float x := 1` resta rifiutato,
`var int i := 1 + 0.5` viene rifiutato perché il valore è un float, `case 1` non corrisponde a un
selettore `1.0`.

## Risks / Trade-offs

- [Rallentamento del percorso caldo `int + int`] → il ramo nuovo è dopo il controllo intero che
  c'era già; misurare `fib` con `./interpreter -b` prima e dopo (attenzione al bug noto: `-b`
  esce 1 anche quando misura bene) e confrontare i tempi.
- [Un `floatObj` costruito fuori dall'heap finisce nel GC] → ogni `floatObj` è creato con `new`
  e solo per il risultato (D2: nessun temporaneo); il GC lo raccoglie come gli altri oggetti.
- [`-nan` invece di `nan`] → il segno del NaN di `0.0/0.0` dipende dalla piattaforma (x86 dà
  `-nan`); la spec ammette entrambe le forme e il test `.out` non stampa mai un NaN direttamente,
  solo il risultato dei confronti.
- [Uguaglianza esatta sui float sorprende] → è il comportamento di C/C++ e di quasi tutti i
  linguaggi; documentato qui, nessuna mitigazione.
- [Allocazione a ogni operazione float] → accettata: i float non sono sul percorso caldo misurato;
  un cache come `theIntCache` è un change separato se servirà.
