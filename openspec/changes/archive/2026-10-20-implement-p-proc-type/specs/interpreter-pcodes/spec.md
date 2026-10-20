# Spec Delta

## Purpose

Descrive come l'interprete esegue i singoli p-code implementati: effetti sullo stack di esecuzione, oggetti prodotti e comportamento osservabile. In questo change copre i p-code `PROC_TYPE` e `FUNC_TYPE`; accoglierà gli altri p-code man mano che vengono implementati.

## ADDED Requirements

### Requirement: Esecuzione del p-code PROC_TYPE

L'interprete SHALL eseguire il p-code `PROC_TYPE`, il cui operando intero N è il numero di tipi dei parametri, consumando i N tipi in cima allo stack di esecuzione e lasciando al loro posto un unico oggetto tipo che rappresenta il tipo proc con quei N parametri, nell'ordine in cui erano sullo stack.

#### Scenario: Tipo proc con parametri

- **WHEN** viene eseguito `PROC_TYPE` con operando 2 quando lo stack di esecuzione contiene i tipi `int` e `str`
- **THEN** lo stack di esecuzione contiene un solo oggetto tipo proc con parametri `(int, str)`

#### Scenario: Tipo proc senza parametri

- **WHEN** viene eseguito `PROC_TYPE` con operando 0
- **THEN** lo stack di esecuzione contiene un solo oggetto tipo proc senza parametri

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `PROC_TYPE` con operando N su uno stack di esecuzione con N tipi in cima
- **THEN** la profondità dello stack diminuisce di N-1 e il tipo proc risiede in cima

### Requirement: Rappresentazione stampabile del tipo proc

L'oggetto tipo prodotto da `PROC_TYPE` SHALL avere una rappresentazione testuale che elenca i tipi dei parametri separati da virgola, nel formato `(t1, t2, ...)`, e `()` quando il tipo non ha parametri.

#### Scenario: Stampa di un tipo proc con parametri

- **WHEN** viene stampato un tipo proc con parametri `int` e `str`
- **THEN** la rappresentazione testuale è `(int, str)`

#### Scenario: Stampa di un tipo proc senza parametri

- **WHEN** viene stampato un tipo proc senza parametri
- **THEN** la rappresentazione testuale è `()`

### Requirement: Sicurezza GC del tipo proc

L'oggetto tipo prodotto da `PROC_TYPE` SHALL rendere raggiungibili per il garbage collector tutti i tipi dei parametri che lo compongono, sia durante la marcatura sia durante l'espansione delle generazioni.

#### Scenario: Marcatura dei tipi dei parametri

- **WHEN** il garbage collector marca un tipo proc marcato come raggiungibile
- **THEN** anche tutti i tipi dei suoi parametri risultano marcati o già raggiungibili

### Requirement: PROC_TYPE non è più un p-code non implementato

L'interprete SHALL riconoscere `PROC_TYPE` nella fabbrica dei p-code: l'esecuzione di un `.pcd` che contiene `PROC_TYPE` non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma cpl2 che dichiara un tipo proc

- **WHEN** viene eseguito un `.pcd` compilato da un sorgente che dichiara una variabile di tipo proc
- **THEN** il programma si esegue senza l'errore `pcode not implemented`

### Requirement: Esecuzione del p-code FUNC_TYPE

L'interprete SHALL eseguire il p-code `FUNC_TYPE`, il cui operando intero N è il numero di tipi dei parametri. Lo stack di esecuzione contiene il tipo di ritorno seguito dai N tipi dei parametri; il p-code li consuma tutti e lascia in cima un unico oggetto tipo che rappresenta il tipo func, con il tipo di ritorno e i parametri nell'ordine in cui erano sullo stack.

#### Scenario: Tipo func con parametri

- **WHEN** viene eseguito `FUNC_TYPE` con operando 2 quando lo stack contiene i tipi `int` (ritorno), `int` e `str` (parametri)
- **THEN** lo stack contiene un solo oggetto tipo func con tipo di ritorno `int` e parametri `(int, str)`

#### Scenario: Tipo func senza parametri

- **WHEN** viene eseguito `FUNC_TYPE` con operando 0 quando lo stack contiene il tipo di ritorno `str`
- **THEN** lo stack contiene un solo oggetto tipo func con tipo di ritorno `str` e nessun parametro

#### Scenario: Effetto sullo stack del FUNC_TYPE

- **WHEN** viene eseguito `FUNC_TYPE` con operando N su uno stack con N+1 tipi in cima (tipo di ritorno più i parametri)
- **THEN** la profondità dello stack diminuisce di N e il tipo func risiede in cima

### Requirement: Rappresentazione stampabile del tipo func

L'oggetto tipo prodotto da `FUNC_TYPE` SHALL avere una rappresentazione testuale nel formato `(ritorno:p1, p2, ...)`, con i tipi dei parametri separati da virgola, e `(ritorno:)` quando il tipo non ha parametri.

#### Scenario: Stampa di un tipo func con parametri

- **WHEN** viene stampato un tipo func con ritorno `int` e parametri `str`, `int`
- **THEN** la rappresentazione testuale è `(int:str, int)`

#### Scenario: Stampa di un tipo func senza parametri

- **WHEN** viene stampato un tipo func con ritorno `str` e nessun parametro
- **THEN** la rappresentazione testuale è `(str:)`

### Requirement: Sicurezza GC del tipo func

L'oggetto tipo prodotto da `FUNC_TYPE` SHALL rendere raggiungibili per il garbage collector sia il tipo di ritorno sia tutti i tipi dei parametri, sia durante la marcatura sia durante l'espansione delle generazioni.

#### Scenario: Marcatura di ritorno e parametri

- **WHEN** il garbage collector marca un tipo func marcato come raggiungibile
- **THEN** anche il tipo di ritorno e tutti i tipi dei parametri risultano marcati o già raggiungibili

### Requirement: FUNC_TYPE non è più un p-code non implementato

L'interprete SHALL riconoscere `FUNC_TYPE` nella fabbrica dei p-code: l'esecuzione di un `.pcd` che contiene `FUNC_TYPE` non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma cpl2 che dichiara un tipo func

- **WHEN** viene eseguito un `.pcd` compilato da un sorgente che dichiara una variabile di tipo func
- **THEN** il programma si esegue senza l'errore `pcode not implemented`
