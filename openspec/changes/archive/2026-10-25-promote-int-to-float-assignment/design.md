# Design

## Context

Tutte le assegnazioni a un nome con tipo dichiarato passano per tre punti di `interpreter.cpp`
(vedi proposal.md - Why per il problema):

- `contextObj::add(intern,type,value)` — `VAR_STORE` e i parametri di proc/func (passati e di
  default, via `procParm::initParms`);
- `contextObj::store(intern,value)` — `STORE`, `STOREX` (passa per `cnt->store`), `FOR_TO`/
  `FOR_DOWNTO`/`FOR_NEXT` e `FOREACH_NEXT`;
- `funcVars::store_result(value)` — `STORE_RESULT`.

Ognuno chiama `checkTipo()`, che ritorna subito senza `--typecheck` e altrimenti usa
`tipoCompatibile()`. Il tipo dichiarato e' sempre disponibile (mappa `types` del contesto,
`result_type` della func), anche senza `--typecheck`.

## Goals / Non-Goals

**Goals:**
- Una sola regola di conversione, applicata nei tre punti, prima del check.
- Nessun costo misurabile sulle assegnazioni int -> int / untyped (il caso di `fib`).

**Non-Goals:**
- Toccare `tipoCompatibile()` o `tipoIs()`: restano le regole di compatibilita' e di `is`.
- Convertire dentro i contenitori o negli operatori (gli operatori promuovono gia' da soli).

## Decisions

**D1 — Conversione, non solo accettazione.** Si sostituisce il valore con `new floatObj(v)`
invece di far accettare l'`intObj` a `tipoCompatibile()`. Alternativa scartata: allargare
`tipoCompatibile()` — la variabile conterrebbe un int, `f/3` sarebbe divisione intera e
`(f is float)` darebbe `false`. Scelta confermata dall'utente.

**D2 — Una funzione `adattaTipo(tipo,valore)` che restituisce il valore da memorizzare.**
Logica:
1. se `valore` non e' un `intObj` -> `valore` invariato (copre nil, float, str, contenitori);
2. se `tipoCompatibile(tipo,valore)` e' vero -> invariato (copre `int`, `?`, senza tipo,
   `int|float`, tipi utente non riconosciuti);
3. se `tipoCompatibile(tipo, <float>)` e' vero -> `new floatObj((double)v)`; altrimenti
   invariato e sara' `checkTipo` a rifiutarlo.
Il passo 3 riusa le regole di unione e `not` invece di duplicarle: basta un `floatObj` sonda
statico (es. `0.0`) da passare a `tipoCompatibile`. Alternativa scartata: una funzione ricorsiva
"il tipo contiene float" separata — doppione delle regole di `orType`/`notType`.

**D3 — Fast path prima di tutto.** Il controllo iniziale e' sul tipo, non sul valore:
`if (tipo==theNil || tipo==theIntType) return valore;` poi il `dynamic_cast<intObj*>`. Le
variabili `int` e quelle senza tipo (tutte quelle di `fib`) non pagano altro che due confronti
di puntatore. `store()` fa gia' oggi la `types.find`, quindi non si aggiunge nessuna ricerca.

**D4 — Chiamata nei tre punti, fuori da `checkTipo`.** `checkTipo` ritorna subito senza
`--typecheck`, quindi non puo' contenere la conversione (che vale anche senza, vedi spec).
In ciascun punto: `value=adattaTipo(tipo,value); checkTipo(...); <memorizza value>`. In
`store()` si sposta la conversione dentro il ramo `ft!=types.end()`.

**D5 — `for` con variabile `float`.** `FOR_TO` scrive l'inizio con `store()`, quindi in una
variabile dichiarata `float` diventerebbe un float e `FOR_TEST` darebbe
`la variabile del for deve essere int`. Si accetta: il `for` richiede gia' interi (inizio, fine,
passo) e con `--typecheck` lo stesso programma era gia' rifiutato all'assegnazione. Non si
introduce un percorso di store senza conversione solo per il `for`.

**D6 — Il fixture.** I casi della spec che non sono rifiuti vanno in `test/float.cpl`/`.out`
(eseguito da `runtest` con `--typecheck`, quindi prova anche che il check non rifiuta piu').
I rifiuti (`var int i := 1.5`, `var float f := "a"`) vanno nel commento di
`test/type_check_err.cpl`, verificati a mano come gli altri.

## Risks / Trade-offs

- [Programmi esistenti senza `--typecheck` che contavano su un int dentro una variabile
  `float`] -> cambia la stampa (`0` -> `0.0`) e la divisione; e' il comportamento voluto, i
  fixture in `test/` vanno rieseguiti per accorgersene (`./runtest`).
- [Un `floatObj` nuovo a ogni assegnazione int -> float] -> allocazione gestita dal GC come ogni
  risultato aritmetico float; nessuna condivisione con la costante, quindi nessun rischio di
  modificarla.
- [`for` su variabile `float` ora fallisce anche senza `--typecheck`] -> documentato (D5).
