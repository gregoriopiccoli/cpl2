# Proposal

## Why

La grammatica emette già i p-code per i parametri con valore di default (`P_PARM_DEFAULT`, `P_ENDPARM_DEFAULT`: regola `pardecla ::= type ID` + `pardecl ::= pardecla ASSIGN expr`), ma l'interprete non li implementa: `makePCode()` li fa cadere su `pcodeNotImpl`, quindi ogni proc o func dichiarata con un parametro inizializzato — per esempio `proc p3(int i:=100)` — compila correttamente e poi abortisce a runtime con `pcode:93 PARM_DEFAULT value:i not impl!` (errore verificato). È il punto 6 di `todo.md`: "parametri con valori di default".

## What Changes

- Implementazione di `PARM_DEFAULT` e `ENDPARM_DEFAULT` nell'interprete, con la semantica descritta dall'autore: durante l'inizializzazione dei parametri di una chiamata, se per un parametro non c'è un valore passato si eseguono i p-code dell'espressione di default fino a `ENDPARM_DEFAULT` e il valore calcolato diventa il valore del parametro; se il valore è stato passato nello stack, si assegna quello e si saltano i p-code dell'espressione di default fino a `ENDPARM_DEFAULT`.
- Nuove classi `pcodeParmDefault` e `pcodeEndParmDefault` in `interpreter.cpp` e i corrispondenti `case` in `makePCode()`; `PARM_DEFAULT` durante la creazione della proc registra il parametro e salta l'espressione di default; `ENDPARM_DEFAULT` è il marcatore di fine espressione.
- Estensione di `procParm`/`procObj` per ricordare, per ogni parametro con default, la posizione p-code della sua espressione di default, ed estensione di `initParms()` per eseguerla — con contesto = variabili della chiamata — quando il parametro non è stato passato.
- Nessuna modifica a grammatica, `pcodes.h`, `pcodes.c` né al formato `.pcd`: i due p-code e la loro emissione esistono già, così come il nome stampabile da `./asm`.
- Fuori scope: parametri per riferimento con default (la grammatica la commenta), check di tipo del valore di default contro il tipo dichiarato (nessun check esiste oggi nemmeno per i parametri normali), interazione con parametri nominati (`P_NAMEDPARM` non è ancora implementato) e con i parametri variadici `...`/`*` (non implementati).

## Capabilities

### New Capabilities

(nessuna)

### Modified Capabilities

- `interpreter-pcodes`: nuovi requisiti per l'esecuzione dei p-code `PARM_DEFAULT` e `ENDPARM_DEFAULT` e per il comportamento dell'inizializzazione dei parametri con default alla chiamata.

## Impact

- `interpreter.cpp` soltanto: due nuove classi p-code, due `case` in `makePCode()`, estensioni a `procParm` (mapping parametro -> intervallo p-code dell'espressione di default) e a `procParm::initParms`/`procObj::call` (esecuzione dell'espressione di default per i parametri non passati).
- Il `.pcd` prodotto dal compilatore non cambia: la testata `#cpl20.0.1` resta valida e l'output di `./asm` resta quello già riportato nel fixture `test/simple_func.cpl`.
- I programmi cpl2 che dichiarano proc/func con parametri di default passano da abort a runtime a esecuzione corretta, con chiamate parziali o senza argomenti.
