# Spec Delta

## Purpose

Definisce la semantica dell'operatore `is` di cpl2, che risponde a runtime se il valore di
un'espressione appartiene a un tipo, con regole di appartenenza proprie e distinte da quelle del
check di tipo sulle assegnazioni.

## ADDED Requirements

### Requirement: Le due forme di is producono un booleano

Le espressioni `is(<expr>, <tipo>)` e `(<expr> is <tipo>)` SHALL essere equivalenti e SHALL valere
`true` se il valore di `<expr>` appartiene a `<tipo>`, `false` altrimenti. `<expr>` SHALL essere
valutata una sola volta. Il risultato SHALL essere un `bool` usabile ovunque serve un'espressione.

#### Scenario: Forma funzionale

- **WHEN** viene eseguito il programma `? is(1, int)`
- **THEN** l'output è `true`

#### Scenario: Forma infissa

- **WHEN** viene eseguito il programma `? ("alfa" is int)`
- **THEN** l'output è `false`

#### Scenario: Usato come condizione

- **WHEN** viene eseguito `var int i := 3` seguito da un `if (i is int)` che stampa `si` e un `else` che stampa `no`
- **THEN** l'output è `si`

### Requirement: Appartenenza ai tipi semplici

Un valore SHALL appartenere a `int`, `str`, `bool`, `float` solo se è rispettivamente un intero, una
stringa, un booleano, un float. Nessuna conversione SHALL essere applicata.

#### Scenario: Tipo giusto e tipo sbagliato

- **WHEN** vengono eseguiti `? (1 is int)`, `? ("a" is str)`, `? (true is bool)`, `? (1 is str)`, `? ("1" is int)`, `? (true is int)`
- **THEN** l'output è, riga per riga, `true`, `true`, `true`, `false`, `false`, `false`

#### Scenario: Tipo float

- **WHEN** viene eseguito `? (1 is float)`
- **THEN** l'output è `false` e il programma non si ferma con `not impl!`

### Requirement: nil appartiene solo ad any

Il valore `nil` SHALL appartenere solo a `any` (scritto anche `?`) e ai tipi unione che hanno `any`
fra le alternative; per ogni altro tipo `is` SHALL dare `false`. Questa regola SHALL valere solo per
`is`: il check di tipo sulle assegnazioni SHALL continuare ad accettare `nil` nei tipi senza `not`.

#### Scenario: nil e un tipo definito

- **WHEN** vengono eseguiti `? (nil is int)` e `? (nil is array[int])`
- **THEN** l'output è `false` per entrambi

#### Scenario: nil e any

- **WHEN** vengono eseguiti `? (nil is any)`, `? (nil is ?)` e `? (nil is int|?)`
- **THEN** l'output è `true` per tutti

#### Scenario: Una variabile int che vale nil

- **WHEN** viene eseguito `var int i := nil` e poi `? (i is int)`
- **THEN** la dichiarazione è accettata anche con il check di tipo acceso e l'output è `false`

### Requirement: any accetta qualunque valore

Ogni valore, `nil` compreso, SHALL appartenere a `any`/`?`.

#### Scenario: Valori diversi

- **WHEN** vengono eseguiti `? (1 is ?)`, `? ("a" is any)` e `? ([1] is ?)`
- **THEN** l'output è `true` per tutti

### Requirement: Tipo NOT

Un valore SHALL appartenere a `<tipo> not` se non è `nil` e appartiene a `<tipo>`.

#### Scenario: NOT su un tipo semplice

- **WHEN** vengono eseguiti `? (1 is int not)`, `? (nil is int not)` e `? ("a" is int not)`
- **THEN** l'output è, riga per riga, `true`, `false`, `false`

#### Scenario: NOT su any

- **WHEN** vengono eseguiti `? (1 is ? not)` e `? (nil is ? not)`
- **THEN** l'output è `true` e poi `false`

### Requirement: Tipo unione

Un valore SHALL appartenere a un tipo unione `T1|...|Tn` se appartiene ad almeno una delle
alternative, con le regole di `is` applicate a ciascuna.

#### Scenario: Una delle alternative

- **WHEN** vengono eseguiti `? (1 is int|str)`, `? ("a" is int|str)` e `? (true is int|str)`
- **THEN** l'output è, riga per riga, `true`, `true`, `false`

### Requirement: Array verificati elemento per elemento

Un valore SHALL appartenere a `array[T]` se è un array e ogni suo elemento appartiene a `T` secondo
le regole di `is`, ricorsivamente. Un array vuoto SHALL appartenere a `array[T]` per ogni `T`.

#### Scenario: Elementi omogenei e no

- **WHEN** vengono eseguiti `? ([1,2,3] is array[int])` e `? ([1,"a"] is array[int])`
- **THEN** l'output è `true` e poi `false`

#### Scenario: Elementi di tipo unione

- **WHEN** viene eseguito `? ([1,"a"] is array[int|str])`
- **THEN** l'output è `true`

#### Scenario: Array vuoto

- **WHEN** viene eseguito `? ([] is array[str])`
- **THEN** l'output è `true`

#### Scenario: Elemento nil

- **WHEN** vengono eseguiti `? ([1,nil] is array[int])` e `? ([1,nil] is array[?])`
- **THEN** l'output è `false` e poi `true`

#### Scenario: Array annidati

- **WHEN** vengono eseguiti `? ([[1],[2,3]] is array[array[int]])` e `? ([[1],["x"]] is array[array[int]])`
- **THEN** l'output è `true` e poi `false`

#### Scenario: Non è un array

- **WHEN** vengono eseguiti `? (1 is array[int])` e `? (|1,2| is array[int])`
- **THEN** l'output è `false` per entrambi

### Requirement: Dizionari verificati valore per valore

Un valore SHALL appartenere a `dict[T]` se è un dizionario e ogni suo valore appartiene a `T`
secondo le regole di `is`, ricorsivamente. Le chiavi non SHALL essere verificate. Un dizionario
vuoto SHALL appartenere a `dict[T]` per ogni `T`.

#### Scenario: Valori omogenei e no

- **WHEN** vengono eseguiti `? ({"a":1,"b":2} is dict[int])` e `? ({"a":1,"b":"x"} is dict[int])`
- **THEN** l'output è `true` e poi `false`

#### Scenario: Dizionario vuoto e non dizionario

- **WHEN** vengono eseguiti `? ({} is dict[int])` e `? ([1] is dict[int])`
- **THEN** l'output è `true` e poi `false`

### Requirement: Tuple verificate posizione per posizione

Un valore SHALL appartenere a `tuple[T1,...,Tn]` se è una tupla di esattamente n elementi e
l'elemento in posizione i appartiene a `Ti` secondo le regole di `is`, ricorsivamente.

#### Scenario: Tupla conforme

- **WHEN** viene eseguito `? (|1,"a",true| is tuple[int,str,bool])`
- **THEN** l'output è `true`

#### Scenario: Posizione sbagliata

- **WHEN** viene eseguito `? (|1,"a"| is tuple[str,int])`
- **THEN** l'output è `false`

#### Scenario: Arità diversa

- **WHEN** vengono eseguiti `? (|1,2| is tuple[int])` e `? (|1| is tuple[int,int])`
- **THEN** l'output è `false` per entrambi

#### Scenario: Non è una tupla

- **WHEN** viene eseguito `? ([1,2] is tuple[int,int])`
- **THEN** l'output è `false`

### Requirement: Tipi proc e func

Un valore SHALL appartenere a un tipo proc o func se è una proc o una func; la firma (numero e tipi
dei parametri, tipo di ritorno) non SHALL essere confrontata.

#### Scenario: Una func e un intero

- **WHEN** viene dichiarata `func int f(int x)` e vengono eseguiti `? (f is (int:int))` e `? (1 is (int:int))`
- **THEN** l'output è `true` e poi `false`

### Requirement: Tipo non riconosciuto

Se l'operando di tipo di `is` non è un tipo che `is` sa verificare, l'esecuzione SHALL fermarsi con
un errore a runtime invece di produrre un booleano.

#### Scenario: Nome usato come tipo che contiene un valore

- **WHEN** viene eseguito `var int mytype := 3` seguito da `? (1 is mytype)`
- **THEN** l'interprete termina con un errore a runtime e non stampa né `true` né `false`
