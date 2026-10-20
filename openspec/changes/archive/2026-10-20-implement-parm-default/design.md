# Design

## Context

La grammatica (`cpl2.gram`, regole `pardecla ::= type ID` e `pardecl ::= pardecla ASSIGN expr`) emette già `PARM_DEFAULT nome` seguito dall'espressione di default e concluso da `ENDPARM_DEFAULT`; `pcodes.h` ha già `P_PARM_DEFAULT` (93) e `P_ENDPARM_DEFAULT` (95) e `pcodes.c` i nomi stampabili, quindi anche `./asm` e il formato `.pcd` non cambiano. Mancano solo i `case` in `makePCode()` e la semantica a runtime.

Meccanismi esistenti da cui dipende il progetto:

- Alla dichiarazione, `procObj::procObj` esegue la lista dei parametri con `i.context=prm` (un `procParm`) partendo dall'istruzione dopo `PROC`/`FUNC`, fermandosi su `ENDPARM`; così nasce la struttura dei parametri (`prmOrder` + `types`). Con un default nella sorgente questa esecuzione oggi si schianta su `pcodeNotImpl`.
- Alla chiamata, `procObj::call` salva pc/prg/context, crea `vars` (`mkVars()`), chiama `prm->initParms(vars, interpreter, parmCnt)` — che legge i valori dalla cima dello stack del chiamante e fa `vars->add(...)` per ogni parametro, `theNil` per i non passati — e solo poi setta `interpreter.prg=prg`, `interpreter.pc=pc+1` ed esegue il corpo.
- `interp::run()` esegue finché non setta `stop`; ogni p-code che deve terminare un blocco (`ENDPARM`, `ENDPROC`, `ENDFUNC`) fa esattamente `interpreter.stop=true`.

Vincolo utile: il layout p-code di un parametro con default è sempre `...tipo... PARM_DEFAULT nome <expr> ENDPARM_DEFAULT`, e l'<expr> è un'espressione: la grammatica non permette di dichiarare proc/func dentro un'espressione, quindi dentro il region di default non può mai esserci un altro `PARM_DEFAULT` né un `ENDPARM`/`ENDPROC`/`ENDFUNC` (però sì `CALL` e label di control-flow: `iif`, short-circuit).

## Goals / Non-Goals

**Goals:**

- Semantica descritta dall'autore: parametro non passato → si eseguono i p-code fino a `ENDPARM_DEFAULT` e il valore calcolato diventa il valore del parametro; parametro passato → si assegna il valore dallo stack e si salta l'espressione fino a `ENDPARM_DEFAULT`.
- Nessuna modifica al compilatore, alla grammatica né al formato `.pcd` (testata `#cpl20.0.1` valida, listing `./asm` invariato).
- Interazione corretta con la GC (valori calcolati passano dalla cima dello stack a `vars`, già radicata e lock-ata per tutta la chiamata).

**Non-Goals:**

- Parametri per riferimento con default (la grammatica la commenta: `pardecl ::= type AT ID ASSIGN expr`).
- Check di tipo del valore di default contro il tipo dichiarato (oggi nessun check neppure per i parametri normali).
- Parametri nominati (`P_NAMEDPARM`), variadici `...`/`*`: non implementati affatto, resta l'attuale errore `pcode not implemented`.
- Defaults che referenziano il parametro stesso o parametri successivi: danno l'errore naturale di `LOAD` sul nome non trovato.

## Decisions

**D1 — `PARM_DEFAULT` salta l'espressione in fase di definizione, `ENDPARM_DEFAULT` è un marcatore puro.**
`pcodeParmDefault::exec` (sotto-classe di `interningpcode`, come `pcodeParm`) registra il parametro nel contesto attuale (stesso schema di `pcodeParm::exec`: consuma il tipo dalla cima dello stack), poi — perché è l'unico momento in cui `PARM_DEFAULT` viene eseguito — cerca avanti nel `prg` il primo `ENDPARM_DEFAULT` (nessuna nesting possibile, vedi Context), memorizza in `procParm` l'intervallo `[posizione successiva a PARM_DEFAULT, posizione di ENDPARM_DEFAULT]` per quel parametro, e setta `interpreter.pc` sulla posizione di `ENDPARM_DEFAULT`: il `pc++` del loop di `run()` atterra subito dopo, quindi l'espressione non viene valutata. `pcodeEndParmDefault::exec` è `interpreter.stop=true`, identico a `pcodeEndParm`.
*Alternativa rifiutata:* dare a `ENDPARM_DEFAULT` il nome del parametro come operando (richiederebbe cambiare la grammatica e rigenerare `cpl2.c`/`cpl2.h` e muterebbe il listing `./asm` già catturato in `test/simple_func.cpl`); con il marcatore puro nessuna modifica al lato compilatore.

**D2 — Ogni p-code ha un solo contesto di esecuzione, non serve una flag di modalità.**
Poiché la fase di definizione salta *oltre* `ENDPARM_DEFAULT` (che quindi non esegue mai lì) e la valutazione alla chiamata parte *dopo* `PARM_DEFAULT` (che quindi non esegue mai lì), `PARM_DEFAULT::exec` gira solo in fase di definizione e `ENDPARM_DEFAULT::exec` solo in fase di valutazione. Niente flag su `interp`, meno stato.
*Alternativa rifiutata:* un flag `creatingParms` su `interp` settato da `procObj::procObj` — più invasiva e inutile per come si distribuiscono le esecuzioni.

**D3 — La valutazione del default avviene in `initParms`, riusando l'`interp` come sotto-interprete.**
`procObj::call` viene riordinato: dopo aver salvato pc/prg/context e creato `vars`, setta *prima* di `initParms` `interpreter.prg=prg` e `interpreter.context=vars` (oggi li setta dopo). `initParms` itera `prmOrder` in ordine posizionale come ora; per ogni parametro con indice ≥ `parmCnt` che ha un intervallo default memorizzato (D1), salva `i.pc`, setta `i.pc` all'inizio dell'espressione, `i.stop=false`, esegue `i.run()`: `ENDPARM_DEFAULT` ferma il sotto-run lasciando il valore calcolato in cima a `stack`/`sp`; lo si recupera (`pop_back`, `sp--`) e si fa `vars->add(p, type, value)`. I valori passati restano intatti sullo stack del chiamante finché non vengono tolti tutti a fine `initParms`, come oggi; `procObj::call` ripristina poi `interpreter.pc=pc+1` e il resto, invariato. Con `prg` già settato, anche una chiamata annidata dentro un'espressione di default (`int b:=f(2)`) funziona perché `procObj::call` è rientrante su pc/prg/context.
*Nota GC:* il valore calcolato vive sulla cima dello stack — radice GC via `gc_array_` — finché non passa in `vars` (lock-ata per la chiamata); nessun nuovo lavoro GC. I nuovi campi di `procParm` contengono solo interi.

**D4 — Integrazione con `makePCode` e nessuna novità fuori da `interpreter.cpp`.**
Due `case` in `makePCode()` (`P_PARM_DEFAULT` → `pcodeParmDefault(s)`, `P_ENDPARM_DEFAULT` → `pcodeEndParmDefault()`). Non si toccano `pcodes.h`, `pcodes.c`, `cpl2.gram`, `cpl2.c`/`cpl2.h` (già a posto), `asm.c` (`MAX_PCODE` = 105 li copre) e `pcodejump[]` (non sono hot).

## Risks / Trade-offs

- [Lo scan avanti di D1 cerca il primo `ENDPARM_DEFAULT`: un futuro annidamento nei default romperebbe l'assunzione] → L'assunzione è garantita dalla grammatica oggi (default = `expr`, e le espressioni non dichiarano proc/func); lasciare un commento nel codice che la esplicita.
- [Un'espressione di default con control-flow (`iif`, short-circuit `and`/`or`) contiene `GOTO`/`IF_FALSE`/label] → Le label sono risolte sulle tabelle globali del programma (`prg.getLabelPos`), il sotto-run è delimitato solo da `stop`, quindi funziona senza nulla di extra.
- [`initParms` riusa lo stesso `interp` del chiamante: uno stack non bilanciato da un default spezzerebbe la chiamata] → Il bilanciamento è strutturale: l'espressione lascia un solo valore e `ENDPARM_DEFAULT` non tocca lo stack; in debug si può verificare `sp` prima/dopo.
- [`vars->add` per un parametro già presente (default valutato due volte) o mancante] → La chiamata evalua il default al massimo una volta per parametro (indice ≥ `parmCnt` e intervallo esistente, mutually exclusive per costruzione).
- [Exit code / abort attuali: i programmi con default falliscono oggi con `domain_error` non catturato] → Comportamento solo migliorabile; nessuna compatibilità indietro da preservare.

## Migration Plan

Non applicabile: feature interamente within-repo, il `.pcd` prodotto non cambia. Rollback = revert del commit.

## Open Questions

(nessuna — la semantica è definita dall'autore e i casi limite scelti (riferimento a parametri precedenti, nessun check di tipo) sono registrati nei Non-Goals)
