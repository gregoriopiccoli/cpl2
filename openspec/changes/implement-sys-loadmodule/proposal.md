# Proposal

## Why

`sys::loadModule` è dichiarato in `interpreter.cpp` ma non ha alcuna definizione, e
`sys::modules` non viene mai usato: `import` si limita a stampare i nomi dei moduli e a
rimuoverli dallo stack, senza caricare né eseguire nulla. Serve il passo che rende reale
l'inclusione: leggere il `.pcd` del modulo indicato ed eseguirlo, così che un `import` abbia
finalmente un effetto osservabile.

## What Changes

- Implementa `sys::loadModule`: dal nome del modulo deriva un percorso `.pcd`, carica il
  programma con `pcodeProgram::loadPcd` e lo esegue una sola volta, in un contesto dedicato
  al modulo che ha i builtin come superlivello.
- Mappa il nome puntato sul percorso: `a.b.c` diventa `<directory del programma che importa>/a/b/c.pcd`
  (i punti diventano separatori di directory, il suffisso è `.pcd`).
- Mantiene la cache dei moduli già caricati in `sys::modules` (programma e contesto): un
  secondo `import` dello stesso nome non deve rieseguire il modulo. L'iscrizione in cache
  avviene prima dell'esecuzione, così un import ciclico o auto-referente non ricorre
  all'infinito.
- Collega `IMPORT` a `loadModule`: dopo aver verificato che le N posizioni in cima allo stack
  siano stringhe, carica i moduli nell'ordine della lista e poi rimuove i nomi. `IMPORT` non
  stampa più i nomi: l'output dell'import è quello prodotto dall'esecuzione del modulo.
- Dopo un caricamento riuscito, `IMPORT` crea nel contesto dell'importatore un simbolo con il
  nome locale del modulo — l'ultimo componente di un nome puntato (`q1` per `q1`, `x` per
  `a.x`) — il cui valore è il contesto del modulo. Se il nome è già presente nel contesto
  dell'importatore il simbolo resta quello esistente. I nomi del modulo non vengono copiati:
  restano raggiungibili solo attraverso il simbolo del modulo.
- Implementa `LOADX` e `STOREX`, i p-code che la grammatica emette per `a.b` e per
  `a.b := espressione` e che oggi finiscono in `pcodeNotImpl`. `LOADX` legge un nome dal
  contenitore in cima allo stack; `STOREX` lo scrive lasciando il valore sullo stack come
  `STORE`.
- **BREAKING** rispetto al comportamento attuale: `import` non è più un comando solo
  diagnostico. Un modulo assente o non compilato è un errore a runtime, non un nome stampato.
- Aggiorna i test: `test/a_module.cpl` + `.out` come modulo eseguibile e test completo;
  `test/import_stack.cpl` + `.out` riscritti per importare `a_module` due volte e verificare
  esecuzione una sola volta; `test/module_access.cpl` + `.out` per leggere e scrivere
  `a_module.x` attraverso il simbolo del modulo. `test/simple_import.cpl` resta di sola
  compilazione perché usa ancora `from`.
- Aggiorna `AGENTS.md`: `import` senza `from` non è più nell'elenco dei costrutti che non
  eseguono; `LOADX` e `STOREX` non sono più p-code non implementati. Resta non implementata la
  sola forma `from`.

## Capabilities

### New Capabilities

- `interpreter-modules`: come l'interprete risolve, carica, esegue e memorizza i moduli
  richiesti da un comando `import`, compresi il percorso derivato dal nome puntato, la cache,
  il contesto separato per modulo e il comportamento in caso di errore.

### Modified Capabilities

- `interpreter-pcodes`: cambia il requisito di `IMPORT`. Non stampa più i nomi e non si limita
  a consumarli: carica ed esegue i moduli indicati. Sparisce il requisito "IMPORT non include
  ancora i moduli".

## Impact

- `interpreter.cpp`: definizione di `sys::loadModule`, campi di `sys` per la cache e la
  directory corrente, esecuzione del modulo, creazione del simbolo del modulo nel contesto che
  importa, implementazione di `pcodeLoadX`/`pcodeStoreX` e loro casi in `makePCode()`, modifica
  di `pcodeImport::exec` e punti in cui viene impostata la directory del programma
  (`runPcd`/`benchPcd`). Nessun nuovo p-code: `LOADX` e `STOREX` sono già definiti.
- `pcodes.h`, `pcodes.c`, `cpl2.gram`, `cpl2.c`, `asm.c`: invariati.
- Test: nuove coppie in `test/`, nessuna modifica a `runtest` né al formato `.pcd`.
- `AGENTS.md` aggiornato nella sezione sui costrutti non eseguibili.
- Restano fuori perimetro: `from ... import ...` (`P_FROM`, ancora non implementato), i nomi del
  modulo usati senza il simbolo del modulo, e la risoluzione di moduli fuori dalla directory del
  programma che importa.
