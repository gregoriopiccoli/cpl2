# Spec Delta

## ADDED Requirements

### Requirement: Esecuzione del p-code COMPR_APPEND

L'interprete SHALL eseguire `COMPR_APPEND`, senza operando, con lo stack che contiene in cima un
valore, sotto di esso un iteratore e sotto ancora un array: SHALL aggiungere il valore in fondo a
quell'array e toglierlo dallo stack, lasciando array e iteratore al loro posto.

#### Scenario: Aggiunta di un elemento

- **WHEN** viene eseguito `COMPR_APPEND` con lo stack `[..., [1], <iteratore>, 2]`
- **THEN** lo stack diventa `[..., [1,2], <iteratore>]`: la profondità diminuisce di 1 e l'array è
  lo stesso oggetto di prima, con un elemento in più

### Requirement: COMPR_APPEND riconosciuto da interprete e asm

L'interprete SHALL riconoscere `COMPR_APPEND` nella fabbrica dei p-code (l'esecuzione non termina
con `not impl!`) e `./asm` SHALL mostrarlo con il suo nome nel listato.

#### Scenario: Listato di asm

- **WHEN** `./asm` legge un `.pcd` compilato da `? [x for each x in [1,2]]`
- **THEN** il listato contiene la riga `COMPR_APPEND`
