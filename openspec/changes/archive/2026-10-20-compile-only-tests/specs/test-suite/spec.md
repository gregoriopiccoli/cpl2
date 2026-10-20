# Spec Delta

## MODIFIED Requirements

### Requirement: Selezione dei test in una directory

`runtest` SHALL prendere come argomento una directory e SHALL considerare test ogni file `.cpl`
di quella directory: se ha accanto un file `.out` con lo stesso nome di base e' un test completo,
altrimenti e' un test di sola compilazione (requirements "Test di sola compilazione").
`runtest` SHALL NOT ricorrere nelle sottodirectory.

#### Scenario: Coppia completa

- **WHEN** l'utente esegue `./runtest` e nella directory `test` esistono `parm_default.cpl` e `parm_default.out`
- **THEN** `runtest` esegue `parm_default.cpl` come test e ne riporta l'esito

#### Scenario: Sorgente senza output atteso

- **WHEN** nella directory dei test esiste `test_expr1.cpl` e non esiste `test_expr1.out`
- **THEN** `runtest` compila `test_expr1.cpl` come test di sola compilazione, non lo esegue e lo conta nel rapporto

#### Scenario: Directory indicata come argomento

- **WHEN** l'utente esegue `./runtest tmp/prove` e in quella directory esistono una coppia `.cpl`/`.out`
- **THEN** `runtest` esegue i test di `tmp/prove` e non quelli di `test`

#### Scenario: Directory senza coppie

- **WHEN** l'utente esegue `./runtest` su una directory che non contiene alcun file `.cpl`
- **THEN** `runtest` non compila e non esegue alcun test e termina con codice di uscita 0

## ADDED Requirements

### Requirement: Test di sola compilazione

Un file `.cpl` della directory dei test che non abbia accanto un file `.out` con lo stesso nome di
base SHALL essere compilato con il binario `compiler` come un test qualunque, e SHALL essere
contato nel rapporto. `runtest` SHALL NOT eseguire con `./interpreter` il p-code di un test di
sola compilazione. La compilazione riuscita SHALL rendere il test superato; una compilazione
fallita o terminata in modo anomalo SHALL renderlo fallito, nominandolo nel rapporto, senza
interrompere il giro.

#### Scenario: Sorgente che compila

- **WHEN** nella directory dei test esiste `simple_class.cpl` senza `simple_class.out` e `compiler` lo compila con esito 0
- **THEN** `runtest` conta `simple_class` fra i test superati senza aver mai invocato `./interpreter` su di esso

#### Scenario: Sorgente che non compila

- **WHEN** nella directory dei test esiste un `.cpl` senza `.out` che produce errori di sintassi
- **THEN** `runtest` lo nomina come fallito e compila comunque gli altri test della directory

#### Scenario: Test completo e test di sola compilazione nello stesso giro

- **WHEN** la directory dei test contiene sia una coppia `.cpl`/`.out` sia un `.cpl` senza `.out`, entrambi validi
- **THEN** il primo viene compilato, eseguito e confrontato con il `.out`, il secondo viene solo compilato, e il rapporto li conta entrambi fra i superati
