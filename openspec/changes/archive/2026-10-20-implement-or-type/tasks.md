# Tasks

## 1. Classe tipo runtime `orType`

- [x] 1.1 In `interpreter.cpp`, dichiarare `orType : public obj` accanto a `tupleType` (~riga 809): `vector` delle alternative, `addAltType(obj*)`, `print()` con formato `t1|t2|...` — verifica: `make interpreter` compila senza errori né warning nuovi
- [x] 1.2 Implementare in `orType` `mark()` e `expand(int gen)` che propagano alle alternative con lo stesso schema di `tupleType` — verifica: `make interpreter` compila

## 2. Classe p-code `pcodeOrType` e fabbrica

- [x] 2.1 Dichiarare `pcodeOrType : public ipcode` accanto a `pcodeTupleType` (~riga 371), con `code=P_OR_TYPE` e `exec()` override — verifica: `make interpreter` compila
- [x] 2.2 Aggiungere `case P_OR_TYPE:return new pcodeOrType(atoi(s));` in `makePCode()` accanto a `P_TUPLE_TYPE` (~riga 480) — verifica: `make interpreter` compila
- [x] 2.3 Implementare `pcodeOrType::exec()`: legge i `value` tipi dalla cima dello stack, li registra nell'`orType` e lascia il tipo prodotto al loro posto replicando la danza di stack di `pcodeTupleType::exec()` (`sp-=(value-1)`, `pop_back()` value-1 volte, sostituzione della cima; `value>=2` sempre, come garantito dalla grammatica) — verifica: `make interpreter` compila

## 3. Verifica funzionale

- [x] 3.1 Compilare il programma di prova `q.cpl` (`./compiler q.cpl`) e controllare con `./asm < q.pcd` che l'emissione contiene `OR_TYPE 2` — verifica: listing `asm` coerente
- [x] 3.2 Eseguire `q.cpl` con `./interpreter q.pcd`: il programma non deve terminare con `not impl!` e deve arrivare alla fine stampando `------------ arrivato alla fine ------------` con exit 0 (l'interprete non fallisce l'assert finale `sp==-1`) — verifica: output a schermo e exit 0
- [x] 3.3 Verificare il caso a tre alternative: sorgente di prova in `tmp/` con `var int|str|bool o`, `./compiler` + listing `./asm` con `OR_TYPE 3` + `./interpreter` senza errore — verifica: esecuzione senza `not impl!` e senza alterazione anomala dello stack
- [x] 3.4 Verificare la rappresentazione stampabile del tipo or: controllare che `print()` dell'`orType` produce `int|str` (per esempio temporaneamente abilitando il trace o verificando la stampa del contesto) — verifica: output `int|str`

## 4. Integrazione

- [x] 4.1 `make` completo: `interpreter`, `compiler` e `asm` si compilano senza errori — verifica: `make` esce 0
- [x] 4.2 Collaudare che nessun p-code esistente è regredito: rieseguire un fixture già funzionante che usa i tipi composti (es. `./compiler` + `./interpreter` su un fixture di `test/` che usa `TUPLE`/tipi proc/func) — verifica: stesso comportamento di prima del change
