# Spec Delta

## MODIFIED Requirements

### Requirement: Selezione dei sorgenti
Il compilatore SHALL leggere i sorgenti dagli argomenti della command line che sono percorsi,
e dallo standard input quando non e' presente alcun percorso. Un argomento che inizia con `-`
e non e' il solo `-` e' un'opzione e non un sorgente; `--` termina il riconoscimento delle
opzioni e ogni argomento successivo e' un sorgente; il solo `-` indica lo standard input.

#### Scenario: Compilazione di un file indicato in command line
- **WHEN** l'utente esegue `compiler fib.cpl`
- **THEN** il compilatore traduce il contenuto di `fib.cpl` e non legge lo standard input

#### Scenario: Compilazione di piu' file indicati in command line
- **WHEN** l'utente esegue `compiler fib.cpl primo.cpl test/simple_class.cpl`
- **THEN** il compilatore traduce ciascuno dei tre file, nell'ordine in cui sono stati indicati

#### Scenario: Nessun file in command line
- **WHEN** l'utente esegue `compiler` senza argomenti, anche con lo standard input reindirizzato da un file
- **THEN** il compilatore traduce lo standard input, come ha sempre fatto

#### Scenario: Il solo trattino indica lo standard input
- **WHEN** l'utente esegue `cat fib.cpl | compiler -`
- **THEN** il compilatore traduce lo standard input e scrive il p-code su stdout, come nella modalita' senza argomenti

#### Scenario: Sorgente il cui nome comincia con un trattino
- **WHEN** l'utente esegue `compiler fib.cpl ./-sorgente.cpl` e il file `./-sorgente.cpl` esiste
- **THEN** il compilatore traduce anche `./-sorgente.cpl`, perche' il percorso digitato non comincia con `-`

#### Scenario: Terminatore `--`
- **WHEN** l'utente esegue `compiler -- -sorgente.cpl` e il file `-sorgente.cpl` esiste
- **THEN** il compilatore traduce il contenuto di `-sorgente.cpl`, perche' dopo `--` gli argomenti sono percorsi

## ADDED Requirements

### Requirement: Opzione sconosciuta in command line
Se la command line contiene un'opzione che il compilatore non dichiara, il compilatore SHALL
segnalarla su stderr nominandola, SHALL NOT tradurre alcun sorgente e SHALL terminare con
codice di uscita 1. Questa condizione e' diversa da un sorgente che non puo' essere aperto:
un'opzione sconosciuta riguarda tutta l'invocazione, quindi gli altri sorgenti non vengono
tradotti.

#### Scenario: Opzione sconosciuta corta
- **WHEN** l'utente esegue `compiler -x fib.cpl`
- **THEN** su stderr compare un messaggio che nomina `-x` come opzione sconosciuta, non viene prodotto alcun p-code e il codice di uscita e' 1

#### Scenario: Opzione sconosciuta lunga
- **WHEN** l'utente esegue `compiler --verbose fib.cpl`
- **THEN** su stderr compare un messaggio che nomina `--verbose` come opzione sconosciuta, non viene prodotto alcun p-code e il codice di uscita e' 1

#### Scenario: Argomento che sembrava un sorgente
- **WHEN** l'utente esegue `compiler -h fib.cpl`
- **THEN** il compilatore non tenta di aprire il file di nome `-h`, segnala `-h` come opzione sconosciuta e termina con codice di uscita 1

#### Scenario: L'opzione sconosciuta interrompe gli altri sorgenti
- **WHEN** l'utente esegue `compiler a.cpl -x b.cpl` e `a.cpl` e `b.cpl` esistono
- **THEN** il compilatore segnala `-x` come opzione sconosciuta, non produce ne' `a.pcd` ne' `b.pcd` e termina con codice di uscita 1

#### Scenario: La segnalazione non finisce su stdout
- **WHEN** l'utente esegue `compiler -x fib.cpl` ridirigendo stderr su `/dev/null`
- **THEN** lo standard output non contiene la segnalazione dell'opzione sconosciuta

#### Scenario: Le opzioni dichiarate non sono ancora accettate dal compilatore
- **WHEN** l'utente esegue `compiler --bench fib.cpl`
- **THEN** il compilatore segnala `--bench` come opzione sconosciuta, perche' l'opzione di misura appartiene all'interprete e il compilatore non ne dichiara nessuna