# Tasks

## 1. Classe tipo runtime `procType`

- [x] 1.1 In `interpreter.cpp`, dichiarare `procType : public obj` accanto a `tupleType` (~riga 779): `vector` dei tipi dei parametri, `addPrmType(obj*)`, `print()` con formato `(t1, t2, ...)` e `()` vuoto — verifica: `make interpreter` compila senza errori né warning nuovi
- [x] 1.2 Implementare in `procType` `mark()` e `expand(int gen)` che propagano ai tipi dei parametri con lo stesso schema di `tupleType` — verifica: `make interpreter` compila

## 2. Classe p-code `pcodeProcType` e fabbrica

- [x] 2.1 Dichiarare `pcodeProcType : public ipcode` accanto a `pcodeTupleType` (~riga 365), con `exec()` override — verifica: `make interpreter` compila
- [x] 2.2 Aggiungere `case P_PROC_TYPE:return new pcodeProcType(atoi(s));` in `makePCode()` accanto a `P_TUPLE_TYPE` (~riga 461) — verifica: `make interpreter` compila
- [x] 2.3 Implementare `pcodeProcType::exec()`: con `value==0` fa `sp++` e push del `procType` vuoto; con `value>=1` replica la danza di stack di `pcodeTupleType::exec()` — verifica: `make interpreter` compila

## 3. Classe tipo runtime `funcType` e classe p-code `pcodeFuncType`

- [x] 3.1 Dichiarare `funcType : public obj` con `prmTypes` più `retType`, `print()` nel formato `(ritorno:p1, p2)` e `(ritorno:)`, `mark()`/`expand()` che propagano sia al tipo di ritorno sia ai parametri — verifica: `make interpreter` compila
- [x] 3.2 Dichiarare `pcodeFuncType : public ipcode` con `code=P_FUNC_TYPE` (56, non `P_PROC_TYPE`) e `case P_FUNC_TYPE:` in `makePCode()` — verifica: `make interpreter` compila
- [x] 3.3 Implementare `pcodeFuncType::exec()`: legge il tipo di ritorno in `stack[sp-value]`, i N parametri sopra, consuma N+1 elementi lasciando il tipo func in cima (funziona anche con N=0 perché il tipo di ritorno è sempre presente) — verifica: `make interpreter` compila

## 4. Verifica funzionale

- [x] 4.1 Scrivere un sorgente di prova in `tmp/` con variabili dichiarate di tipo proc (es. `(int, str)`), compilarlo con `./compiler` e controllare con `./asm < tmp/….pcd` che l'emissione contiene `PROC_TYPE 2` — verifica: listing `asm` coerente
- [x] 4.2 Eseguire il `.pcd` con `./interpreter`: il programma non deve terminare con `pcode not implemented` — verifica: esecuzione senza errore
- [x] 4.3 Verificare il caso N=0 (`()`): sorgente di prova con tipo proc senza parametri, `./compiler` + `./interpreter` — verifica: esecuzione senza errore e senza alterazione anomala dello stack (l'interprete non fallisce l'assert finale `sp==-1`)
- [x] 4.4 Verificare la stampa del tipo con `print` su un valore/variabile di tipo proc: atteso `(int, str)` (formato deciso in fase di implementazione) — verifica: output a schermo
- [x] 4.5 Verificare `FUNC_TYPE` con e senza parametri nel programma di prova `q.cpl` (`./compiler q.cpl`, listing `./asm < q.pcd` con `FUNC_TYPE 2` e `FUNC_TYPE 0`, esecuzione `./interpreter q.pcd` senza `not impl!`) — verifica: `q.cpl` arriva alla fine con exit 0

## 5. Integrazione

- [x] 5.1 `make` completo: `interpreter`, `compiler` e `asm` si compilano senza errori — verifica: `make` esce 0
- [x] 5.2 Collaudare che nessun p-code esistente è regredito: rieseguire un fixture già funzionante (es. `./compiler test/…` + `./interpreter …`) — verifica: stesso comportamento di prima del change
