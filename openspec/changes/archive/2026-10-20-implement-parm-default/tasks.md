# Tasks

## 1. Classi p-code e fabbrica (fase di definizione)

- [x] 1.1 Estendere `procParm` in `interpreter.cpp` con il bookkeeping dei default: per ogni parametro in `prmOrder` poter memorizzare l'intervallo p-code `[inizio espressione, posizione di ENDPARM_DEFAULT]`, con sentinella "nessun default". Verifica: `make` compila senza errori.
- [x] 1.2 Aggiungere `pcodeEndParmDefault` (sotto-classe di `pcode`, `exec` = `interpreter.stop=true` come `pcodeEndParm`) e i `case P_PARM_DEFAULT`/`case P_ENDPARM_DEFAULT` in `makePCode()`. Verifica: `make` compila e `./interpreter` non stampa più `pcode:93 PARM_DEFAULT ... not impl!` su un `.pcd` che dichiara `proc p3(int i:=100)` (la dichiarazione non abortisce più, anche se i default non funzionano ancora).
- [x] 1.3 Implementare `pcodeParmDefault::exec` (sotto-classe di `interningpcode`): registrare il parametro come fa `pcodeParm` (consuma il tipo dalla cima dello stack), registrare in `procParm` l'intervallo dell'espressione di default, e saltare a `ENDPARM_DEFAULT` (`interpreter.pc` sulla sua posizione) così l'espressione non viene valutata in fase di definizione. Verifica: creare il fixture `test/parm_default.cpl` (proc `p3(int i:=100)` + chiamate `p3()` e `p3(7)`, con il blocco p-code atteso commentato dentro, come in `test/simple_func.cpl`), compilare con `./compiler test/parm_default.cpl` e controllare con `./asm < test/parm_default.pcd` che il listing corrisponde al blocco atteso: `PARM_DEFAULT i / INT_CONST 100 / ENDPARM_DEFAULT / ENDPARM`.

## 2. Valutazione dei default alla chiamata

- [x] 2.1 Riordinare `procObj::call`: settare `interpreter.prg=prg` e `interpreter.context=vars` prima di `prm->initParms(...)` (oggi dopo). Verifica: `make` e riesecuzione di fixture già funzionanti senza parametri di default (per esempio `./compiler test/simple_func.cpl` con una chiamata, e i programmi di `test/`) — nessun regressione nel percorso di chiamata.
- [x] 2.2 Estendere `procParm::initParms`: per ogni parametro con indice ≥ `parmCnt` che ha un intervallo default, eseguire l'espressione con l'`interp` del chiamante (`i.pc` all'inizio dell'espressione, `i.stop=false`, `i.run()`), recuperare il valore in cima allo stack (`pop_back`, `sp--`) e passarlo a `vars->add(...)`; i parametri passati restano inizializzati come ora, in ordine posizionale. Verifica: `./compiler test/parm_default.cpl && ./interpreter test/parm_default.pcd` stampa `100` poi `7`.
- [x] 2.3 Estendere il fixture `test/parm_default.cpl` con il caso misto posizionale (`proc p(int a, int b:=10, int c:=20)` chiamata `p(1)` → stampa `1 10 20`) e con default che referenzia un parametro precedente (`proc q(int a, int b:=a*2)` chiamata `q(5)` → stampa `5 10`); aggiornare il blocco p-code atteso commentato. Verifica: `./compiler test/parm_default.cpl && ./interpreter test/parm_default.pcd` produce le uscite attese con exit 0.

## 3. Verifica di integrazione

- [x] 3.1 Verificare che la chiamata che passa tutti i parametri non valuta l'espressione di default: aggiungere al fixture un default che produrrebbe errore (espressione con nome inesistente) su un parametro sempre passato nelle chiamate del fixture. Verifica: `./interpreter test/parm_default.pcd` termina con exit 0 senza errori.
- [x] 3.2 Passata di regressione sui programmi già funzionanti: `./compiler test/*.cpl` senza errori di parse nuovi e riesecuzione dei fixture eseguibili di `test/` (per esempio fibonacci/funzioni con parametri normali) — nessun cambiamento di comportamento fuori dai default. Verifica: output identico a prima della modifica per i fixture senza default.
- [x] 3.3 Controllare che nulla fuori da `interpreter.cpp` sia cambiato: `git status`/`git diff` mostrano solo `interpreter.cpp` (più il nuovo fixture `test/parm_default.cpl` e i file di build non tracciati). Verifica: `./asm < test/parm_default.pcd` produce comunque la testata `CPL2 PCODE VER:0.0.1` e il listing senza operandi nuovi su `ENDPARM_DEFAULT`.

## Workflow follow-up

- Dopo la revisione: `openspec archive implement-parm-default` per consolidare i requisiti in `openspec/specs/interpreter-pcodes/spec.md`.
