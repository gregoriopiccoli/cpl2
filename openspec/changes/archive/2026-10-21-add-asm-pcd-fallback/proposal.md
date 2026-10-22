# Proposal

## Why

Il workflow quotidiano produce `.pcd` accanto ai sorgenti e l'interprete accetta già un
nome senza estensione (`./interpreter mio` carica `mio.pcd`), ma `asm` no: `./asm mio`
fallisce con "impossibile aprire il file" anche quando `mio.pcd` esiste, e l'utente deve
ricordarsi di scrivere l'estensione a mano. Basta un fallback: nome dato prima,
`<nome>.pcd` come seconda prova.

## What Changes

- `asm`, quando gli viene indicato un percorso che non esiste e il suo nome di base non
  contiene alcun punto, prova in seconda battuta a caricare lo stesso percorso con
  `.pcd` accodato (stessa regola del "nome di base" già usata dall'interprete: il punto
  si cerca solo nel nome di base, mai nella directory).
- Nessun cambio di comportamento quando il percorso indicato esiste, quando il nome di
  base contiene un punto, in modalità standard input, né nei codici di uscita o
  nell'output (sempre su stdout, nessun file creato).

## Capabilities

### New Capabilities

<!-- nessuna capability nuova -->

### Modified Capabilities

- `asm-cli`: il requisito "File non apribile" viene esteso con il fallback `.pcd` —
  la diagnostica di errore nomina ancora il percorso digitato quando entrambe le prove
  falliscono, e la prima prova resta sempre il percorso indicato.

## Impact

- Solo `asm.c`, funzione `main()`: la `fopen` del percorso indicato viene affiancata
  dalla seconda prova `<percorso>.pcd` quando il nome di base non ha punti.
- Nessun cambio in `emit.c`, `pcodes.c`, nel compilatore o nell'interprete.
- Nessuna dipendenza nuova; `cli.c` invariato.
- `AGENTS.md` documenta il comportamento di `asm` e va aggiornato con il fallback.
