# Proposal

## Why

La grammatica riconosce già `import` e genera i p-code `IMPORTID`, `IMPORTIDX` e `IMPORT`,
ma l'interprete non li riconosce in `makePCode()`: qualunque sorgente con un `import` viene
compilato e poi fallisce a runtime con `pcode not implemented`. Serve un primo passo
eseguibile che costruisca sullo stack i nomi dei moduli richiesti, così da poterli
controllare prima di implementare la vera inclusione dei moduli.

## What Changes

- L'interprete riconosce ed esegue `IMPORTID`: mette in cima allo stack una stringa con
  l'identificatore che costituisce l'operando del p-code.
- L'interprete riconosce ed esegue `IMPORTIDX`: mette in cima la stringa dell'identificatore
  e poi sostituisce le due stringhe in cima con la loro concatenazione separata da `.`
  (quindi `a` seguito da `IMPORTIDX b` lascia un'unica stringa `a.b`).
- L'interprete riconosce ed esegue `IMPORT`: usa il numero N del suo operando per prendere
  le N stringhe in cima allo stack, ne stampa il valore su standard output (una stringa per
  riga, nell'ordine della lista) e le rimuove, così ogni comando `import` lascia lo stack
  pulito e non altera i nomi del successivo.
- Aggiunge un test completo con output atteso per le forme `import a`, `import a.b.c` e
  `import a.b,c.d`; `test/simple_import.cpl` resta un test di sola compilazione perché usa
  anche `from`, ancora unimplementato.
- Aggiorna `AGENTS.md` per togliere `import` dall'elenco dei costrutti che esplodono sempre
  a runtime, precisando che l'inclusione vera non è ancora implementata.

## Capabilities

### New Capabilities

<!-- nessuna capability nuova -->

### Modified Capabilities

- `interpreter-pcodes`: nuovi requisiti per l'esecuzione di `IMPORTID`, `IMPORTIDX` e
  `IMPORT`, per l'effetto sullo stack e per i nomi stampati da `IMPORT` (N dall'operando),
  oltre al requisito che i tre p-code non siano più "not implemented".

## Impact

- Solo `interpreter.cpp`: tre nuove classi p-code, i relativi metodi `exec()` e tre casi in
  `makePCode()`.
- `pcodes.h`, `pcodes.c`, `cpl2.gram` e `asm.c` non cambiano: i p-code e i loro testi
  esistenti sono già registrati.
- Test: nuova coppia `test/import_stack.cpl` / `test/import_stack.out`; nessuna modifica alle
  regole di `runtest`.
- `AGENTS.md` aggiornato nella sezione sui costrutti non eseguibili.
- Nessuna dipendenza nuova. La vera inclusione dei moduli, `FROM` e la risoluzione dei
  percorsi restano fuori da questo change.
