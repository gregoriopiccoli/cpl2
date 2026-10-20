# Tasks

## 1. Classe tipo runtime `boolType` e singleton

- [x] 1.1 In `interpreter.cpp`, dichiarare `boolType : public obj` accanto a `intType` (~riga 743), con `print()` che restituisce `bool` — verifica: `make interpreter` compila senza errori né warning nuovi
- [x] 1.2 Dichiarare il singleton `lockgc_ptr<obj> theBoolType{new boolType()}` accanto a `theIntType`/`theStrType` (~riga 758) e azzerarlo in `releaseSysObjs()` (~riga 1925) insieme agli altri — verifica: `make interpreter` compila

## 2. Classe p-code `pcodeBoolType` e fabbrica

- [x] 2.1 Dichiarare `pcodeBoolType : public pcode` (senza operando, come `pcodeIntType`) accanto a `pcodeIntType` (~riga 341), con `code=P_BOOL_TYPE` e `exec()` override — verifica: `make interpreter` compila
- [x] 2.2 Aggiungere `case P_BOOL_TYPE:return new pcodeBoolType();` in `makePCode()` accanto a `P_INT_TYPE` (~riga 469), senza `atoi(s)` — verifica: `make interpreter` compila
- [x] 2.3 Implementare `pcodeBoolType::exec()`: `sp++` e push del singleton `theBoolType`, identico a `pcodeIntType::exec()` — verifica: `make interpreter` compila

## 3. `getBaseType()` di `boolObj`

- [x] 3.1 Aggiungere l'override `boolObj::getBaseType()` che ritorna `theBoolType` (sostituendo l'ereditato che restituisce `theNil`), replicando `intObj::getBaseType()` — verifica: `make interpreter` compila e un fixture già funzionante che usa confronti tra bool (es. `test/test_expr1.cpl`) non regredisce

## 4. Verifica funzionale

- [x] 4.1 Scrivere un sorgente di prova in `tmp/` con una variabile di tipo bool (`var bool b:=true`) e un `print`, compilarlo con `./compiler` e controllare con `./asm < tmp/….pcd` che l'emissione contiene `BOOL_TYPE` — verifica: listing `asm` coerente
- [x] 4.2 Eseguire il `.pcd` con `./interpreter`: il programma non deve terminare con `pcode not implemented` e deve stampare `true` — verifica: output a schermo e exit 0 (l'interprete non fallisce l'assert finale `sp==-1`)
- [x] 4.3 Verificare la rappresentazione stampabile del tipo bool: dichiarare un tipo bool e stamparlo, atteso `bool` — verifica: output a schermo

## 5. Integrazione

- [x] 5.1 `make` completo: `interpreter`, `compiler` e `asm` si compilano senza errori — verifica: `make` esce 0
- [x] 5.2 Collaudare che nessun p-code esistente è regredito: rieseguire un fixture già funzionante (es. `./compiler test/test_var1.cpl` + `./interpreter test/test_var1.pcd`) — verifica: stesso comportamento di prima del change
