## MODIFIED Requirements

### Requirement: Esecuzione di un test

Per ogni test `runtest` SHALL compilare il file `.cpl` con il binario `compiler`, come farebbe
l'utente dalla command line, e SHALL eseguire con il binario `interpreter` il p-code prodotto dal
compilatore, con il check di tipo nelle assegnazioni acceso (opzione `--typecheck`). `runtest`
SHALL catturare l'output dell'esecuzione per confrontarlo con il file `.out`.

#### Scenario: Compilazione del sorgente

- **WHEN** `runtest` esegue il test `parm_default`
- **THEN** invoca `compiler` sul file `.cpl` del test e ottiene il `.pcd` corrispondente

#### Scenario: Esecuzione del p-code prodotto

- **WHEN** la compilazione di un test e' riuscita
- **THEN** `runtest` esegue con `interpreter` il `.pcd` appena prodotto dal compilatore, non un `.pcd` preesistente ne' un altro file

#### Scenario: Output catturato

- **WHEN** `runtest` esegue il p-code di un test
- **THEN** l'output dell'esecuzione viene catturato per il confronto, senza essere mostrato come output del giro di test

#### Scenario: Check di tipo acceso

- **WHEN** `runtest` esegue il p-code di un test completo
- **THEN** l'interprete gira con il check di tipo acceso, esattamente come `./interpreter --typecheck` sul `.pcd` del test, senza che l'utente debba passare alcuna opzione a `runtest`

#### Scenario: Assegnazione rifiutata dal check di tipo

- **WHEN** il programma di un test completo assegna a una variabile `int` un valore `str`
- **THEN** l'esecuzione si interrompe con l'errore di tipo, il test e' contato fra quelli falliti e gli altri test vengono eseguiti comunque

#### Scenario: Assegnazioni compatibili con il check acceso

- **WHEN** `runtest` esegue `type_check.cpl`, le cui assegnazioni sono tutte compatibili con i tipi dichiarati
- **THEN** l'output catturato coincide con `type_check.out` e il test e' superato

### Requirement: Nessuna interazione con la modalita' di misura

`runtest` SHALL eseguire i test in modalita' normale e SHALL NOT usare l'opzione di misura
dell'interprete, ne' confrontare l'output di una misura con un file `.out`. L'unica opzione che
`runtest` SHALL passare all'interprete e' quella del check di tipo.

#### Scenario: Nessuna tabella di misura nel confronto

- **WHEN** `runtest` esegue il p-code di un test
- **THEN** lo standard output catturato e' quello di una singola esecuzione, senza la tabella dei tempi di nanobench

#### Scenario: Nessun report del GC nel confronto

- **WHEN** `runtest` esegue il p-code di un test
- **THEN** lo standard output catturato non contiene il report del GC, perche' `runtest` non passa `--gc` all'interprete
