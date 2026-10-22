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
  stampa più i nomi: l'output dell'import è un nuvo contesto che viene contiene i simboli prodotti dall'esecuzione del modulo.
- l'import crea un nuovo simbolo nel contesto che lo importa che punta al nuovo contesto che è stato creato.
- **BREAKING** rispetto al comportamento attuale: `import` non è più un comando solo
  diagnostico. Un modulo assente o non compilato è un errore a runtime, non un nome stampato.
- Aggiorna i test: `test/a_module.cpl` + `.out` come modulo eseguibile e test completo;
  `test/import_stack.cpl` + `.out` riscritti per importare `a_module` due volte e verificare
  esecuzione una sola volta. `test/simple_import.cpl` resta di sola compilazione perché usa
  ancora `from`.
- Aggiorna `AGENTS.md`: `import` senza `from` non è più nell'elenco dei costrutti che non
  eseguono; resta non implementata la sola forma `from`.

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
  directory corrente, esecuzione del modulo, modifica di `pcodeImport::exec` e punti in cui
  viene impostata la directory del programma (`runPcd`/`benchPcd`). Nessuna nuova classe
  p-code e nessun nuovo p-code.
- `pcodes.h`, `pcodes.c`, `cpl2.gram`, `cpl2.c`, `asm.c`: invariati.
- Test: nuove coppie in `test/`, nessuna modifica a `runtest` né al formato `.pcd`.
- `AGENTS.md` aggiornato nella sezione sui costrutti non eseguibili.
- Restano fuori perimetro: `from ... import ...` (`P_FROM`, ancora non implementato),
  l'accesso ai nomi esportati dal modulo e la risoluzione di moduli fuori dalla directory del
  programma che importa.
