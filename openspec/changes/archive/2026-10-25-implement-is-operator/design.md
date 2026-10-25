# Design

## Context

- La grammatica ha già `fact ::= IS LPAR expr COMMA type RPAR.` e `fact ::= LPAR expr IS type RPAR.`,
  entrambe con `{emit(P_IS);}`; il p-code emesso è `<expr>`, `<tipo>`, `IS` (verificato con `./asm`).
  `P_IS` (71) e il testo `"IS"` esistono già in `pcodes.h`/`pcodes.c`. In `makePCode()` manca il
  `case`, quindi oggi finisce in `pcodeNotImpl`.
- I p-code dei tipi lasciano sullo stack questi oggetti: `theIntType`, `theStrType`, `theBoolType`,
  `theNil` per `any`/`?`, `arrayType(t)`, `dictType(t)`, `tupleType` (lista), `orType` (alternative),
  `notType(t)`, `procType`/`funcType`. `FLOAT_TYPE` non ha `case` in `makePCode()` pur esistendo
  `theFloatType`. Un `dottedid` usato come tipo emette `LOAD <nome>`, quindi l'operando "tipo" può
  essere un valore qualunque.
- `tipoCompatibile()` decide il check di assegnazione: accetta `nil` in ogni tipo senza `not` ed è
  superficiale sui contenitori. Le regole di `is` (spec `is-operator`) sono diverse su entrambi i
  punti, quindi non si può riusarla così com'è.
- `arrayType`, `dictType`, `tupleType` tengono il tipo interno in campi privati senza accessori;
  `arrayObj` ha `len()` (non const) e `slice()`, `tupleObj` ha `size() const`, `dictObj` ha
  `chiavi()`. Nessuno espone gli elementi in sola lettura in modo uniforme.

## Goals / Non-Goals

**Goals:**
- `pcodeIs` + `pcodeFloatType` registrati in `makePCode()`, con la semantica della spec.
- Una funzione di appartenenza separata da `tipoCompatibile`, che resta byte-per-byte invariata.

**Non-Goals:**
- Rendere profondo o più stretto su `nil` il check di assegnazione.
- Mettere `IS` in `pcodejump[]`: non è un p-code caldo.
- Correggere `dictType::print()` che stampa `array[...]` invece di `dict[...]` (vedi Risks).

## Decisions

**D1 — Nuova funzione `tipoIs(const obj* tipo,const obj* valore)` accanto a `tipoCompatibile`.**
Stesso stile (catena di `dynamic_cast`, commenti in italiano) ma regole proprie: l'ordine è
`theNil` (any) → `true`; `orType` → una qualunque alternativa, **compresa** `any` (al contrario di
`tipoCompatibile`, che salta le alternative `theNil`); `notType` → `valore!=nil && tipoIs(inter)`;
poi `valore==nil` → `false`; poi tipi semplici e composti. Alternativa scartata: aggiungere un flag
a `tipoCompatibile` — mescolerebbe due semantiche in una funzione già usata dal runtime caldo delle
assegnazioni.

**D2 — Contenitori in profondità tramite accessori const.** Si aggiungono getter in sola lettura:
`arrayType::elem()`, `dictType::elem()`, `tupleType::tipi()` (come `orType::alts()`), e sui valori
`arrayObj::elementi()`, `tupleObj::elementi()` che restituiscono `const vector<obj*>&`, e
`dictObj::valori()` che visita la mappa (oppure un `const unordered_map<string,obj*>&`). Le tuple
confrontano prima l'arità. Alternativa scartata: passare da `slice()`/`len()`, che non sono const e
per i dict vogliono un oggetto chiave allocato.

**D3 — Strutture cicliche: guardia sulle coppie in visita.** Un array può contenere se stesso
(`a[0]:=a` via `STORESLICE`), e la visita ricorsiva non terminerebbe. `tipoIs` riceve (o crea) un
insieme di coppie `(valore,tipo)` in corso di verifica; se la coppia è già nell'insieme risponde
`true` (lettura coinduttiva: il ciclo non porta da solo controesempi). Alternativa scartata: limite
di profondità fisso — darebbe risposte sbagliate su strutture profonde ma acicliche.

**D4 — Tipo non riconosciuto = `domain_error`.** Se l'operando non è nessuno degli oggetti tipo
elencati nel Context, `tipoIs` lancia `domain_error("is: <print del tipo> non e' un tipo")`, come
fanno gli altri errori di runtime. Copre il `dottedid` che carica un valore. `theNil` resta `any`
per costruzione, quindi una variabile che vale `nil` usata come tipo risponde `true`: limite noto,
accettato finché non esistono i tipi utente.

**D5 — Proc/func superficiali.** `procType`/`funcType` accettano qualunque `procObj` (stesso
criterio di `tipoCompatibile`); il confronto delle firme è fuori ambito.

**D6 — `pcodeIs::exec`.** Legge tipo in `stack[sp]` e valore in `stack[sp-1]`, fa un `pop_back`,
`sp--`, e scrive `theTrue`/`theFalse` in `stack[sp]` — lo stesso schema di `pcodeEq`. Nessuna
allocazione, quindi nessun problema di GC fra lettura e scrittura. `pcodeFloatType::exec` è la copia
di `pcodeBoolType` con `theFloatType`.

**D7 — Test.** Un nuovo test completo `test/is.cpl` + `test/is.out`, con l'`.out` scritto a mano
dagli scenari della spec. Lo scenario d'errore (tipo non riconosciuto) termina con exit non zero,
quindi va solo verificato a mano e annotato in testa al file come in `type_check.cpl`.

## Risks / Trade-offs

- [`nil` diverso fra `is` e assegnazione: `var int i := nil` passa, ma `(i is int)` è `false`] →
  voluto (scelta dell'utente); scritto esplicitamente nella spec e nel commento di `tipoIs`.
- [Costo lineare sulla dimensione della struttura per array/dict grandi] → accettabile: `is` è una
  domanda esplicita del programma, non un controllo implicito.
- [L'insieme di D3 costa un'allocazione per chiamata] → crearlo solo alla prima discesa in un
  contenitore, non per i tipi semplici.
- [`dictType::print()` stampa `array[...]`: comparirebbe nel messaggio di D4 solo se un dict-type
  fosse "non riconosciuto", il che non accade] → nessuna azione in questo change.
