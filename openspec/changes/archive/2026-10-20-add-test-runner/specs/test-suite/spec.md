# Spec Delta

## Purpose

Definisce il sistema di test del progetto cpl2: quali file formano un test, come il test viene
eseguito e con quale esito, e come `runtest` riporta il risultato di un giro su una directory.

## ADDED Requirements

### Requirement: Selezione dei test in una directory

`runtest` SHALL prendere come argomento una directory e SHALL considerare test ogni file `.cpl`
di quella directory che abbia accanto un file `.out` con lo stesso nome di base. Un `.cpl` senza
`.out` SHALL NOT essere eseguito ne' contato. `runtest` SHALL NOT ricorrere nelle sottodirectory.

#### Scenario: Coppia completa

- **WHEN** l'utente esegue `./runtest` e nella directory `test` esistono `parm_default.cpl` e `parm_default.out`
- **THEN** `runtest` esegue `parm_default.cpl` come test e ne riporta l'esito

#### Scenario: Sorgente senza output atteso

- **WHEN** nella directory dei test esiste `test_expr1.cpl` e non esiste `test_expr1.out`
- **THEN** `runtest` non compila, non esegue e non conta `test_expr1.cpl`

#### Scenario: Directory indicata come argomento

- **WHEN** l'utente esegue `./runtest tmp/prove` e in quella directory esistono una coppia `.cpl`/`.out`
- **THEN** `runtest` esegue i test di `tmp/prove` e non quelli di `test`

#### Scenario: Directory senza coppie

- **WHEN** l'utente esegue `./runtest` su una directory che non contiene alcuna coppia `.cpl`/`.out`
- **THEN** `runtest` non esegue alcun test e termina con codice di uscita 0

### Requirement: Output atteso senza sorgente

Un file `.out` che non ha accanto un corrispondente `.cpl` SHALL essere segnalato da `runtest`
nominandolo, e SHALL contare come fallimento del giro di test.

#### Scenario: Output atteso orfano

- **WHEN** nella directory dei test esistono `orfano.out` e non esiste `orfano.cpl`
- **THEN** `runtest` segnala `orfano.out` e il giro termina con codice di uscita 1

### Requirement: Esecuzione di un test

Per ogni test `runtest` SHALL compilare il file `.cpl` con il binario `compiler`, come farebbe
l'utente dalla command line, e SHALL eseguire con il binario `interpreter` il p-code prodotto dal
compilatore. `runtest` SHALL catturare l'output dell'esecuzione per confrontarlo con il file
`.out`.

#### Scenario: Compilazione del sorgente

- **WHEN** `runtest` esegue il test `parm_default`
- **THEN** invoca `compiler` sul file `.cpl` del test e ottiene il `.pcd` corrispondente

#### Scenario: Esecuzione del p-code prodotto

- **WHEN** la compilazione di un test e' riuscita
- **THEN** `runtest` esegue con `interpreter` il `.pcd` appena prodotto dal compilatore, non un `.pcd` preesistente ne' un altro file

#### Scenario: Output catturato

- **WHEN** `runtest` esegue il p-code di un test
- **THEN** l'output dell'esecuzione viene catturato per il confronto, senza essere mostrato come output del giro di test

### Requirement: Confronto con l'output atteso

`runtest` SHALL confrontare l'output catturato con il contenuto del file `.out` byte per byte,
senza normalizzazioni, senza rimozione di spazi e senza interpretazione. Output uguale SHALL
rendere il test superato; output diverso SHALL rendere il test fallito.

#### Scenario: Output uguale

- **WHEN** l'esecuzione di un test produce esattamente il contenuto del suo file `.out`
- **THEN** `runtest` conta il test fra quelli superati

#### Scenario: Output diverso

- **WHEN** l'esecuzione di un test produce un output diverso dal contenuto del suo file `.out`
- **THEN** `runtest` conta il test fra quelli falliti

#### Scenario: Differenza di soli spazi bianchi

- **WHEN** l'output catturato differisce dal file `.out` solo per spazi o righe vuote finali
- **THEN** `runtest` conta il test fra quelli falliti

### Requirement: Isolamento ed esito di ogni test

Ogni test SHALL essere eseguito indipendentemente dagli altri: un test che non compila, che non si
esegue o il cui output non corrisponde SHALL essere contato come fallito senza interrompere il
giro di test, e gli altri test SHALL essere eseguiti comunque, fino all'ultimo.

#### Scenario: Test fallito e test successivi

- **WHEN** il primo test della directory fallisce e ne esistono altri con output corrispondente
- **THEN** `runtest` segnala il primo come fallito, esegue tutti gli altri e conta questi ultimi fra quelli superati

#### Scenario: Test il cui sorgente non compila

- **WHEN** un test ha un `.cpl` con un errore di sintassi
- **THEN** il test e' contato fra quelli falliti e gli altri test vengono eseguiti comunque

#### Scenario: Test il cui p-code non si esegue

- **WHEN** l'esecuzione del p-code di un test termina con codice di uscita diverso da 0
- **THEN** il test e' contato fra quelli falliti e gli altri test vengono eseguiti comunque

### Requirement: Ripetibilita' del giro di test

`runtest` SHALL scrivere i file che produce — i `.pcd` compilati e gli output catturati — in una
posizione di appoggio, e SHALL non lasciare alcun file nella directory dei test ne' modificare i
file che vi trova. La directory di appoggio SHALL essere rimossa alla fine del giro, anche se il
giro termina con test falliti.

#### Scenario: Nessun file lasciato nei test

- **WHEN** `runtest` esegue un giro su `test`
- **THEN** al termine `test` non contiene file nuovi rispetto a quelli che conteneva prima, e il file `.pcd` eventualmente gia' presente non e' stato modificato

#### Scenario: File di appoggio rimossi dopo un fallimento

- **WHEN** un giro termina con almeno un test fallito
- **THEN** i file prodotti da `runtest` sono stati rimossi come in un giro riuscito

#### Scenario: Due giri consecutivi

- **WHEN** l'utente esegue `./runtest` due volte di seguito
- **THEN** i due giri riportano lo stesso esito e il secondo non trova file lasciati dal primo

### Requirement: Rapporto finale del giro di test

Alla fine del giro `runtest` SHALL riportare il numero di test superati e il numero di test
falliti, e SHALL nominare ogni test fallito; SHALL terminare con codice di uscita 0 quando tutti i
test sono superati e con codice di uscita 1 quando almeno un test e' fallito.

#### Scenario: Tutti i test superati

- **WHEN** ogni test della directory produce un output uguale al proprio `.out`
- **THEN** `runtest` riporta il totale dei test superati, non nomina alcun fallimento e termina con codice 0

#### Scenario: Almeno un test fallito

- **WHEN** uno o piu' test producono un output diverso dal proprio `.out`
- **THEN** `runtest` nomina i test falliti, riporta il conteggio dei superati e dei falliti e termina con codice 1

#### Scenario: Esito leggibile da uno script

- **WHEN** l'utente esegue `./runtest > /dev/null` e uno dei test fallisce
- **THEN** il codice di uscita e' comunque 1, perche' il riporto del rapporto non finisce sullo standard output in modo esclusivo

### Requirement: Nessuna interazione con la modalita' di misura

`runtest` SHALL eseguire i test in modalita' normale e SHALL NOT usare l'opzione di misura
dell'interprete, ne' confrontare l'output di una misura con un file `.out`.

#### Scenario: Nessuna tabella di misura nel confronto

- **WHEN** `runtest` esegue il p-code di un test
- **THEN** lo standard output catturato e' quello di una singola esecuzione, senza la tabella dei tempi di nanobench
