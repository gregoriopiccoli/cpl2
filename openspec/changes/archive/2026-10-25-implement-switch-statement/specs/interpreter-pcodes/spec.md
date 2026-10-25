# Spec Delta

## ADDED Requirements

### Requirement: Esecuzione del p-code CASE

L'interprete SHALL eseguire il p-code `CASE`, il cui operando intero è una label, togliendo dalla
cima dello stack la costante del `case`, confrontandola con il selettore che sta subito sotto e
saltando alla label quando sono uguali. Il selettore SHALL restare in cima allo stack sia quando
il salto avviene sia quando non avviene.

#### Scenario: Costante uguale al selettore

- **WHEN** viene eseguito `CASE 5` con lo stack che contiene il selettore `2` e sopra la costante `2`
- **THEN** l'esecuzione prosegue dalla posizione della `LABEL 5` e in cima allo stack c'è il
  selettore `2`

#### Scenario: Costante diversa dal selettore

- **WHEN** viene eseguito `CASE 5` con lo stack che contiene il selettore `2` e sopra la costante `3`
- **THEN** l'esecuzione prosegue con il p-code successivo e in cima allo stack c'è il selettore `2`

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `CASE` su uno stack con selettore e costante in cima
- **THEN** la profondità dello stack diminuisce di 1, sia con salto sia senza

### Requirement: Confronto di CASE fra tipi diversi

Quando la costante e il selettore hanno tipi diversi, oppure il selettore è `nil`, `CASE` SHALL
considerarli non uguali e proseguire senza salto e senza sollevare errori. Per valori dello stesso
tipo (`int`, `str`, `bool`) SHALL usare la stessa uguaglianza dell'operatore `=`.

#### Scenario: Selettore int, costante stringa

- **WHEN** viene eseguito `CASE` con selettore `1` e costante `"1"`
- **THEN** non c'è salto e non viene sollevato alcun errore

#### Scenario: Selettore nil

- **WHEN** viene eseguito `CASE` con selettore `nil` e costante `0`
- **THEN** non c'è salto e non viene sollevato alcun errore

#### Scenario: Stringhe uguali

- **WHEN** viene eseguito `CASE` con selettore `"b"` e costante `"b"`
- **THEN** l'esecuzione salta alla label dell'operando

### Requirement: CASE non è un p-code non implementato

L'interprete SHALL riconoscere `CASE` nella fabbrica dei p-code: l'esecuzione di un `.pcd` che
contiene `CASE` non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma cpl2 con switch

- **WHEN** viene eseguito un `.pcd` compilato da un sorgente che contiene uno `switch`
- **THEN** il programma si esegue senza l'errore `pcode not implemented`

### Requirement: Rappresentazione leggibile di CASE

Il p-code `CASE` SHALL avere il nome testuale `CASE`, così che `./asm` lo mostri come
`CASE <label>` e la conversione inversa del listato lo riconosca.

#### Scenario: Listato di asm

- **WHEN** `./asm` legge un `.pcd` che contiene il p-code `CASE` con operando 3
- **THEN** il listato contiene una riga `CASE 3`
