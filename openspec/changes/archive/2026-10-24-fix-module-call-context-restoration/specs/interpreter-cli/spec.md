# Spec Delta

## MODIFIED Requirements

### Requirement: Derivazione del percorso da caricare
Il percorso caricato SHALL essere derivato dal nome di base del solo percorso indicato: se il nome di base non contiene alcun punto, ad esso SHALL essere accodato `.pcd`; se ne contiene uno, il percorso indicato SHALL essere usato invariato. Il punto SHALL essere cercato solo nel nome di base, mai nella directory, e la directory indicata SHALL essere conservata. Per l'esecuzione normale, l'interprete SHALL utilizzare la stessa regola di derivazione del percorso (`pcodePathFor`) e SHALL caricare/eseguire il file derivato usando il meccanismo di esecuzione del p-code (come `runPcd`), assicurando il corretto funzionamento dei comandi come `./interpreter q` anche quando il file `q.pcd` si trova accanto al sorgente.

#### Scenario: Nome senza estensione
- **WHEN** l'utente esegue `./interpreter fib` e il file `fib.pcd` esiste
- **THEN** l'interprete carica ed esegue `fib.pcd`

#### Scenario: Nome gia' con estensione
- **WHEN** l'utente esegue `./interpreter fib.pcd`
- **THEN** l'interprete usa il percorso `fib.pcd` come indicato, senza appendere nulla

#### Scenario: Estensione diversa da `.pcd`
- **WHEN** l'utente esegue `./interpreter fib.cpl`
- **THEN** l'interprete tenta di caricare `fib.cpl` e segnala che il file non e' un p-code compilato, senza cercare `fib.pcd` al suo posto

#### Scenario: Punto nella directory e non nel nome di base
- **WHEN** l'utente esegue `./interpreter dir.d/x`
- **THEN** l'interprete carica `dir.d/x.pcd`, considerando il punto della directory come parte della directory

#### Scenario: File in una sottodirectory
- **WHEN** l'utente esegue `./interpreter test/simple_class` e `test/simple_class.pcd` esiste
- **THEN** l'interprete carica ed esegue `test/simple_class.pcd`

#### Scenario: Nome senza estensione per file accanto al sorgente
- **WHEN** l'utente esegue `./interpreter q` e il file `q.pcd` esiste nella directory corrente
- **THEN** l'interprete lo carica ed esegue correttamente
