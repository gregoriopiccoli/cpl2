# Spec Delta

## ADDED Requirements

### Requirement: Esecuzione del p-code BOOL_TYPE

L'interprete SHALL eseguire il p-code `BOOL_TYPE`, p-code senza operando, spingendo in cima allo stack di esecuzione l'oggetto tipo che rappresenta il tipo bool.

#### Scenario: Tipo bool

- **WHEN** viene eseguito `BOOL_TYPE`
- **THEN** lo stack di esecuzione contiene in cima l'oggetto tipo bool

#### Scenario: Effetto sullo stack

- **WHEN** viene eseguito `BOOL_TYPE` con lo stack di esecuzione a una certa profondità
- **THEN** la profondità dello stack aumenta di 1 e il tipo bool risiede in cima

### Requirement: Rappresentazione stampabile del tipo bool

L'oggetto tipo prodotto da `BOOL_TYPE` SHALL avere la rappresentazione testuale `bool`.

#### Scenario: Stampa del tipo bool

- **WHEN** viene stampato l'oggetto tipo bool
- **THEN** la rappresentazione testuale è `bool`

### Requirement: Sicurezza GC del tipo bool

L'oggetto tipo bool SHALL sopravvivere alle raccolte del garbage collector mentre è referenziato (per esempio sullo stack di esecuzione) e SHALL essere rilasciato alla fine dell'esecuzione insieme agli altri oggetti di sistema.

#### Scenario: Raccolta con il tipo bool sullo stack

- **WHEN** il garbage collector esegue una raccolta mentre un oggetto tipo bool è sullo stack di esecuzione
- **THEN** l'oggetto tipo bool sopravvive e resta utilizzabile

#### Scenario: Rilascio alla fine dell'esecuzione

- **WHEN** l'interprete rilascia gli oggetti di sistema a fine esecuzione
- **THEN** anche l'oggetto tipo bool viene rilasciato

### Requirement: BOOL_TYPE non è più un p-code non implementato

L'interprete SHALL riconoscere `BOOL_TYPE` nella fabbrica dei p-code: l'esecuzione di un `.pcd` che contiene `BOOL_TYPE` non SHALL terminare con l'errore `pcode not implemented`.

#### Scenario: Programma cpl2 che dichiara un tipo bool

- **WHEN** viene eseguito un `.pcd` compilato da un sorgente che dichiara una variabile di tipo bool
- **THEN** il programma si esegue senza l'errore `pcode not implemented`
