# Spec Delta

## ADDED Requirements

### Requirement: Seconda prova con suffisso .pcd
Quando il percorso indicato sulla command line non esiste e il suo nome di base non
contiene alcun punto, `asm` SHALL provare a leggere il p-file da quel percorso con
`.pcd` accodato. Il punto SHALL essere cercato solo nel nome di base, mai nella
directory, e la directory indicata SHALL essere conservata. Se la prima apertura ha
successo, il suffisso SHALL non essere provato e il percorso indicato SHALL essere
usato invariato.

#### Scenario: Nome senza estensione, il .pcd esiste
- **WHEN** l'utente esegue `./asm mio` e il file `mio` non esiste ma `mio.pcd` si
- **THEN** `asm` traduce il contenuto di `mio.pcd` sullo standard output e il codice
  di uscita e' quello della conversione riuscita

#### Scenario: Nome di base con un punto, nessun suffisso
- **WHEN** l'utente esegue `./asm mio.bak` e il file `mio.bak` non esiste
- **THEN** `asm` NON prova a caricare `mio.bak.pcd`, riporta l'errore sullo standard
  error e termina con codice di uscita 1

#### Scenario: Il punto nella directory non conta
- **WHEN** l'utente esegue `./asm dir.d/mio` e `dir.d/mio` non esiste ma
  `dir.d/mio.pcd` si
- **THEN** `asm` traduce il contenuto di `dir.d/mio.pcd`

#### Scenario: Il file indicato esiste, nessun suffisso
- **WHEN** l'utente esegue `./asm mio` e il file `mio` esiste
- **THEN** `asm` traduce `mio` e NON prova `mio.pcd`, anche se `mio.pcd` esiste

## MODIFIED Requirements

### Requirement: File non apribile
Se il percorso indicato non puo' essere aperto - ne' la prima prova ne', quando il
nome di base non contiene alcun punto, la seconda prova con `.pcd` accodato - `asm`
SHALL riportare la condizione nominando il percorso digitato su stderr, SHALL NON
produrre output e SHALL terminare con codice di uscita 1.

#### Scenario: Percorso inesistente
- **WHEN** l'utente esegue `./asm mancante.pcd` e il file non esiste
- **THEN** su stderr compare un messaggio che nomina `mancante.pcd`, lo standard output
  resta vuoto e il codice di uscita e' 1

#### Scenario: Il percorso non compare su stdout
- **WHEN** l'utente esegue `./asm mancante.pcd` ridirigendo stderr su `/dev/null`
- **THEN** lo standard output non contiene alcun messaggio di errore

#### Scenario: Entrambe le prove falliscono, il nome digitato resta il nome dell'errore
- **WHEN** l'utente esegue `./asm mancante` e non esistono ne' `mancante` ne'
  `mancante.pcd`
- **THEN** su stderr compare un messaggio che nomina `mancante` (senza suffisso) e il
  codice di uscita e' 1
