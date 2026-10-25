# Proposal

## Why

La grammatica accetta già l'operatore `is` nelle due forme `is(<expr>, <tipo>)` e `(<expr> is <tipo>)`
e il compilatore emette `<expr>`, `<tipo>`, `IS`, ma l'interprete non implementa `P_IS`: il programma
compila e poi si ferma con `not impl!`. Senza `is` un programma cpl2 non può chiedere a runtime se un
valore appartiene a un tipo, che è il complemento naturale del check di tipo sulle assegnazioni appena
introdotto.

## What Changes

- L'interprete esegue il p-code `IS`: consuma il tipo in cima allo stack e sostituisce il valore
  sottostante con `true` o `false`.
- La risposta segue regole proprie di `is`, distinte da quelle del check di assegnazione:
  - `nil` appartiene solo a `any`/`?` (e a un tipo unione che contiene `any`): `(nil is int)` è
    `false`, mentre l'assegnazione di `nil` a una variabile `int` resta accettata come oggi;
  - il controllo è **profondo** sui contenitori: `array[T]` verifica ogni elemento, `dict[T]` ogni
    valore, `tuple[T1,...,Tn]` l'arità e ogni posizione, ricorsivamente;
  - `T not`, i tipi unione e `any` mantengono il significato che hanno già nel sistema dei tipi.
- L'interprete esegue anche il p-code `FLOAT_TYPE` (oggi non implementato), così `(x is float)`
  risponde invece di fermarsi con `not impl!`.
- Un tipo che `is` non sa riconoscere è un errore a runtime invece di una risposta silenziosa.
- Nessuna modifica a grammatica, compilatore, formato `.pcd` o al check di assegnazione
  (`tipoCompatibile` resta invariato).

## Capabilities

### New Capabilities
- `is-operator`: semantica dell'operatore `is` di cpl2 — le due forme sintattiche, il risultato
  booleano e le regole di appartenenza di un valore a un tipo (nil, tipi semplici, contenitori in
  profondità, unione, `not`, `any`, proc/func, tipi non riconosciuti).

### Modified Capabilities
- `interpreter-pcodes`: si aggiungono i requisiti di esecuzione dei p-code `IS` e `FLOAT_TYPE`
  (effetto sullo stack, e che non siano più p-code non implementati).

## Impact

- `interpreter.cpp`: nuova classe `pcodeIs` e `pcodeFloatType` con i rispettivi `case` in
  `makePCode()`; una nuova funzione di appartenenza accanto a `tipoCompatibile`; accessori in sola
  lettura sui tipi composti (`arrayType`, `dictType`, `tupleType`) e sui contenitori (`arrayObj`,
  `dictObj`, `tupleObj`) per la visita degli elementi.
- `test/`: un nuovo test completo `is.cpl` + `is.out`.
- `AGENTS.md`: `is` (e i tipi `float`) escono dall'elenco dei costrutti che compilano e poi falliscono.
- Fuori ambito: tipi utente (`dottedid`, che oggi non emette alcun p-code per il tipo), tipi con
  `with`, `date`/`time`/`datetime`, il confronto delle firme di proc/func, i pcode `P_AND`/`P_OR`.
