# Design

## Context

Vedi `proposal.md` per il perché. Stato attuale rilevante:

- Il lexer (`parser.c`, ramo "è un numero") produce `TOK_FLOAT_CONST` per `cifre.cifre`, togliendo
  i `_`; il testo del token è sempre della forma `[0-9]+\.[0-9]*` (niente segno, niente esponente,
  niente `.5`). La grammatica fa `emit_s(P_FLOAT_CONST,value)`: nel `.pcd` l'operando è quel testo.
- `makePCode()` riceve l'operando come `const char*` e oggi, per `P_FLOAT_CONST`, ricade nel
  `default` → `pcodeNotImpl`.
- `floatObj` esiste ma è inutilizzato: `float value`, `print()` con `to_string` (6 decimali fissi),
  nessun `getBaseType()` (quindi eredita `theNil` da `obj`), nessuna operazione.
- `pcodeIntConst`/`pcodeStrConst` costruiscono il valore una volta nel costruttore e lo tengono in
  un `lockgc_ptr<obj>` (bloccato per il GC, rilasciato nel distruttore); `getObjValue()` lo espone.
- `pcodeUMinus::exec` delega a `obj::uminus()`; l'intero lo ridefinisce, gli altri tipi lanciano
  `domain_error("uminus not implemented")`.
- `tipoCompatibile` e `tipoIs` riconoscono già `floatObj` per `theFloatType`.

## Goals / Non-Goals

**Goals:**
- `FLOAT_CONST` eseguibile, con lo stesso schema di vita della costante di `INT_CONST`.
- Una stampa dei float stabile e byte-esatta (serve a `runtest`, che confronta l'output byte per
  byte).
- `UMINUS` sui float.

**Non-Goals:**
- Nessuna cache/riciclo di `floatObj` come `theIntCache` per gli interi: senza aritmetica float
  non c'è un percorso caldo che lo giustifichi.
- `FLOAT_CONST` non entra in `pcodejump[]`.
- Nessun cambio al compilatore: il testo dell'operando è già in un formato che si legge senza
  ambiguità.

## Decisions

**D1 — `double` al posto di `float`.** `floatObj::value` diventa `double`, il costruttore prende
`double`. Con `float` a 32 bit `0.1` si stamperebbe in forma minima come `0.1` ma i valori letti
da sorgente perderebbero cifre (`3.14159265358979` → `3.1415927`), e un domani l'aritmetica
accumulerebbe errore visibile. Il nome del tipo nel linguaggio resta `float`. Alternativa
scartata: lasciare `float` — più piccolo, ma nessun vantaggio reale in un interprete che alloca
un oggetto per valore.

**D2 — Lettura dell'operando con `strtod`.** In `makePCode()`:
`case P_FLOAT_CONST:return new pcodeFloatConst(s);` e il costruttore fa `strtod(s,nullptr)`.
L'interprete non chiama `setlocale`, quindi gira nel locale `"C"` e il separatore è sempre `.`.
`strtod` accetta `7.` (punto finale) che il lexer può produrre. Alternativa: `std::from_chars` su
`double` — indipendente dal locale per costruzione, ma non aggiunge nulla finché il locale è `"C"`
e `strtod` è già lo stile del file (`atoi` per `INT_CONST`). La classe è uno `spcode` (operando
stringa, come `STR_CONST`), così `asm` e qualunque diagnostica che stampa l'operando mostrano il
testo originale.

**D3 — Stampa: forma più corta che fa il giro, con `.0` forzato.** `floatObj::print()` usa
`std::to_chars(buf, buf+N, value)` senza formato né precisione: per il C++17 dà la
rappresentazione più corta che riletta restituisce esattamente lo stesso `double` (GCC ≥ 11; la
macchina ha GCC 13 e il Makefile compila col default `gnu++17`). Se il risultato non contiene né
`.` né `e` né lettere (`inf`/`nan`), si aggiunge `.0`. Quindi `3.140`→`3.14`, `2.0`→`2.0`,
`0.1`→`0.1`. Alternative scartate: `to_string` (sempre 6 decimali, `3.140000`), `%g` (perde il
`.0`, `2.0` stampa come `2`, indistinguibile da un intero), `%.17g` (`0.1` →
`0.10000000000000001`). Per valori molto grandi o molto piccoli `to_chars` può scegliere la
notazione esponenziale (`1e+20`): accettato, non è raggiungibile con le sole costanti scritte in
modo ragionevole e non è nei test.

**D4 — Costante condivisa e immutabile.** `pcodeFloatConst` tiene il `floatObj` in un
`lockgc_ptr<obj>` creato nel costruttore e azzerato nel distruttore, come `pcodeIntConst`, e lo
spinge sullo stack a ogni `exec`. È sicuro condividerlo perché nessuna operazione modifica un
`floatObj` in place: `uminus()` restituisce sempre `new floatObj(-value)`. `getObjValue()` è
ridefinito come negli altri `*Const`.

**D5 — `getBaseType()` del float.** `floatObj` dichiara `getBaseType()` e la definizione va dopo
`theFloatType` (come `intObj::getBaseType` dopo `theIntType`). Oggi nessuno lo usa per i float,
ma senza la ridefinizione un float risulterebbe di tipo base `nil`, il che è sbagliato e
sorprenderebbe la prima operazione mista.

**D6 — Niente promozioni.** `var float x := 1` resta rifiutato con `--typecheck` (tipoCompatibile
vuole un `floatObj`): è coerente con la regola "nessuna conversione" già specificata per `is`.

## Risks / Trade-offs

- [`std::to_chars` per `double` richiede `<charconv>` e una libstdc++ recente] → GCC 13 è quello in
  uso; il Makefile non fissa `-std`, il default di GCC 13 è `gnu++17`. Se un giorno servisse un
  compilatore più vecchio, il ripiego è un ciclo `%.{15..17}g` che sceglie la prima precisione che
  rilegge lo stesso valore.
- [Stampa in notazione esponenziale per valori estremi] → comportamento accettato e documentato in
  D3, fuori dai test.
- [Un `floatObj` per costante bloccato per tutta la vita del programma] → stesso costo di
  `INT_CONST`, trascurabile.
- [`AGENTS.md` dice che le costanti float non girano] → aggiornarlo nello stesso change, altrimenti
  la documentazione mente.
