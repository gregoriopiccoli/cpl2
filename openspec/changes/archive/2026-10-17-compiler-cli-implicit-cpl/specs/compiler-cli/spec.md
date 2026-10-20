# Spec Delta

## ADDED Requirements

### Requirement: Risoluzione implicita di `.cpl` per un nome senza estensione
Quando il nome di base del percorso indicato in command line non contiene alcun punto e quel
percorso non puo' essere aperto perche' non esiste, il compilatore SHALL tentare di aprire
anche il percorso ottenuto accodando `.cpl` e, se si apre, SHALL tradurre il suo contenuto.

#### Scenario: Nome senza estensione risolto come `.cpl`
- **WHEN** l'utente esegue `compiler fib` in una directory dove `fib` non esiste e `fib.cpl` esiste
- **THEN** il compilatore traduce il contenuto di `fib.cpl` e scrive il p-code in `fib.pcd`

#### Scenario: Il percorso digitato ha la precedenza
- **WHEN** l'utente esegue `compiler fib` in una directory dove esistono sia `fib` sia `fib.cpl`
- **THEN** il compilatore traduce il contenuto di `fib` e non di `fib.cpl`

#### Scenario: Nome con estensione mai riscritto
- **WHEN** l'utente esegue `compiler sorg.txt` e `sorg.txt` non esiste, in una directory dove esiste `sorg.cpl`
- **THEN** il compilatore segnala che non puo' aprire `sorg.txt` e non traduce `sorg.cpl`

#### Scenario: Il punto si cerca solo nel nome di base
- **WHEN** l'utente esegue `compiler dir.d/x` in una directory dove `dir.d/x` non esiste e `dir.d/x.cpl` esiste
- **THEN** il compilatore traduce il contenuto di `dir.d/x.cpl` e scrive il p-code in `dir.d/x.pcd`

#### Scenario: Il ripiego scatta solo se il file non esiste
- **WHEN** l'utente esegue `compiler fib` in una directory dove `fib` esiste ma non e' leggibile e `fib.cpl` esiste ed e' leggibile
- **THEN** il compilatore segnala che non puo' aprire `fib` e non traduce `fib.cpl`

### Requirement: Le diagnostiche nominano il percorso risolto
Ogni diagnostica emessa durante la traduzione di un sorgente indicato in command line SHALL
essere preceduta dal percorso del file realmente tradotto, cioe' quello aperto, e non dal
percorso originariamente digitato.

#### Scenario: Prefisso con il percorso risolto
- **WHEN** l'utente esegue `compiler fib`, esiste solo `fib.cpl` e `fib.cpl` ha un errore di sintassi alla riga 3
- **THEN** su stderr compare un messaggio che inizia con `fib.cpl: ` e riporta `line 3`

#### Scenario: Scanner sul percorso risolto
- **WHEN** l'utente esegue `compiler fib`, esiste solo `fib.cpl` e `fib.cpl` contiene una stringa non terminata
- **THEN** su stderr compare il messaggio dello scanner con il prefisso `fib.cpl: `

## MODIFIED Requirements

### Requirement: Un file non apribile non interrompe gli altri
Se un file indicato in command line non esiste, non e' leggibile o il relativo file p-code
non puo' essere scritto, il compilatore SHALL riportare la condizione nominando tutti i
percorsi che ha tentato di aprire per quel sorgente, SHALL contarla fra gli errori e SHALL
proseguire con i file rimanenti.

#### Scenario: File inesistente in mezzo alla lista
- **WHEN** l'utente esegue `compiler a.cpl mancante.cpl b.cpl`
- **THEN** il compilatore segnala l'impossibilita' di aprire `mancante.cpl`, produce il p-code di `a.pcd` e di `b.pcd`, e termina con codice di uscita 1

#### Scenario: Output non scrivibile
- **WHEN** il file p-code di un sorgente non puo' essere creato, perche' la directory non esiste o non e' scrivibile
- **THEN** il compilatore segnala la condizione su stderr, non produce p-code per quel sorgente, e continua con gli altri file

#### Scenario: Percorsi tentati nella diagnostica del sorgente non apribile
- **WHEN** l'utente esegue `compiler fib` in una directory dove non esistono ne' `fib` ne' `fib.cpl`
- **THEN** su stderr compare una diagnostica che cita sia `fib` sia `fib.cpl`, non viene prodotto alcun p-code e il codice di uscita e' 1

#### Scenario: Un risolto e uno irraggiungibile nella stessa invocazione
- **WHEN** l'utente esegue `compiler fib mancante` in una directory dove esiste solo `fib.cpl` e non esistono `mancante` ne' `mancante.cpl`
- **THEN** il compilatore produce il p-code di `fib.cpl`, segnala l'irraggiungibilita' di `mancante` e `mancante.cpl` e termina con codice di uscita 1