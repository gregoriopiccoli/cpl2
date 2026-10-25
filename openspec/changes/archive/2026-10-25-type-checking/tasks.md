# Tasks

## 1. Il p-code NOT_TYPE

- [x] 1.1 In `pcodes.h` aggiungi `#define P_NOT_TYPE 106` accanto ai p-code dei tipi
      (dopo `P_OR_TYPE`) e porta `MAX_PCODE` a 106. Verifica: `grep P_NOT_TYPE
      pcodes.h` lo mostra, e `asm.c` continua a compilare (usa `MAX_PCODE` come
      limite di `FindPCode`).
- [x] 1.2 In `pcodes.c`, dentro `initpcodetxt()`, aggiungi
      `pcodetxt[P_NOT_TYPE]=(char*)"NOT_TYPE";` accanto a `OR_TYPE`: e' il nome che
      `./asm` stampa sulla riga. Verifica: `./asm < x.pcd` mostra `NOT_TYPE` sulla
      riga di un p-code generato da una dichiarazione con `not`.
- [x] 1.3 In `cpl2.gram` la regola `type ::= btype NOT.` (che prima non emetteva
      nulla, cosi' il `not` spariva in compilazione) emette `P_NOT_TYPE`; poi
      rigeneri con `lemon cpl2.gram` e fai commit di `cpl2.c` cosi' come esce (il
      diff esteso del boilerplate e' atteso, e' la regola del Makefile). Verifica:
      `lemon` esce con 0 e senza conflitti, e `cpl2.h` resta solo codici `TOK_*`.
- [x] 1.4 In `interpreter.cpp`: la classe `notType` (avvolge il tipo base e si
      marca al GC come `orType` e le classi simili) con `inter()` che restituisce
      il tipo avvolto, la classe `pcodeNotType` col suo `exec()` che rimpiazza il
      p-code in cima allo stack, e il `case P_NOT_TYPE:` in `makePCode()`.
      Verifica: i 4 edit coordinati (`pcodes.h`, `pcodes.c`, `cpl2.gram`,
      `interpreter.cpp`) sono in ordine e `make` compila senza errori.

## 2. Le regole di compatibilita'

- [x] 2.1 In `interpreter.cpp` definisci `tipoCompatibile(tipo,valore)` con le
      regole decise nel design (Decision 2): senza tipo dichiarato (`any`/`?`, e
      `theNil` come lo passano i builtin e `IMPORT`) accetta qualunque valore;
      per l'`orType` basta una alternativa, per il `notType` il nil e' rifiutato e
      poi vale il tipo base, entrambi in ricorsione; un tipo definito senza `not`
      accetta anche il nil; i tipi semplici si riconoscono col `dynamic_cast` sul
      valore (come gia' fanno `check_int` e `check_str`); `arrayType/dictType/
      tupleType` e `procType/funcType` si abbinano alla classe del valore; i tipi
      non riconosciuti (i nomi utente della grammatica `btype ::= dottedid`) non
      bloccano niente. Verifica: tutte le classi di cui i `dynamic_cast` hanno
      bisogno sono definite prima del corpo della funzione, e la dichiarazione
      anticipata sta prima di `contextObj` che la chiama.
- [x] 2.2 L'oggetto `notType` resta in vita perche' sta nella mappa `types` del
      contesto (che lo marca al GC), e `interp::run` non lo vede: il wrapper
      sostituisce il p-code gia' in cima allo stack, quindi `P_VAR`/`P_VAR_STORE`/
      `P_POP` proseguono come prima. Verifica: un programma con `var int not ni` e
      `ni := 10` stampa `10`, e col `--gc` il report chiude a zero oggetti.

## 3. Il check nelle scritture

- [x] 3.1 In `contextObj` il messaggio di rifiuto esce come fa il container dei
      builtin col "name not found": `<nome>: il valore <valore> non e' compatibile
      con il tipo <tipo>` su stdout, poi l'eccezione. Definisci il metodo
      `checkTipo` e chiamalo dall'aggiunta a 3 argomenti (dove arriveranno le
      inizializzazioni di `P_VAR_STORE` e i valori dei parametri) e da `store()`;
      il `store` senza voce in `types` non controlla (i nomi senza tipo), e
      l'aggiunta a 2 argomenti (dichiarazione senza inizializzazione) parte con
      nil e non controlla: il `not` si controlla alla prima assegnazione.
      Verifica: `var int i := 5`, `i := "pippo"`, `var int j := "pippo"`,
      `var int not ni` con `ni := nil` ciascuno stampa il messaggio atteso, e due
      giri di `./runtest` prima e dopo l'esercizio non cambiano esito.
- [x] 3.2 Per coprire `P_STORE` e `P_STOREX` dei moduli basta il check dentro
      `contextObj::store`, perche' `moduleObj` eredita: `q1.i := 200` controlla
      col tipo dichiarato in `q1`. Verifica: i fixture `q`, `q2`, `q1`,
      `module_access`, `a_module`, `import_stack` restano SUPERATO in `./runtest`.
- [x] 3.3 Sistemi la registrazione func/proc perche' il check abbia senso: in
      `makeProcOrFunc()` il nome di una func si lega alla firma completa (un
      `funcType` con il result type e i tipi dei parametri letti da `procParm`)
      e non al solo result type: prima il nome portava `int` e il check rifiutava
      il valore `funcObj` dandolo per int. Servono gli accessori
      `procObj::getParm()` e `procParm::tipoDi()`. Verifica: `q` e `q2` (che legge
      `? q1.doppio` e chiama `q1.doppio(5)`) tornano SUPERATO, `? doppio` stampa
      `<func int doppio(int x) -- pcode>`, e `var (int:(int)) g := doppio` passa.
- [x] 3.4 Il binding dei parametri alla chiamata passa gia' per `initParms()` che
      chiama `vars->add(...)` con tipo e valore: col check dentro l'aggiunta a 3
      argomenti, `func int f(int x)` con `f("a")` viene rifiutata alla chiamata.
      Verifica: `parm_default` resta SUPERATO (i default e le voci nil passano
      perche' i loro tipi non hanno il `not`), e `f("a")` piu' `h(nil)` con il
      parametro `int not x` stampano i messaggi attesi.

## 4. Prova di non-regressione e test

- [x] 4.1 `make` senza errori ed `./runtest` con tutti i test di `test/` SUPERATO ed
      exit 0: la prova che il check non rompe nessun programma esistente.
- [x] 4.2 Aggiungi `test/type_check.cpl` (test completo col `.out`) che esercita i
      percorsi validi: tipi di base in inizializzazione e riassegnazione,
      `array[int]`, unione `int|str` da ambo i lati, `?`, assegnazione del nil in
      un tipo senza `not`, il `not` con valore, func con parametri tipizzati,
      variabile func legata alla firma. Generi il `.out` dal run reale
      dell'interprete (esito 0).
- [x] 4.3 Aggiungi `test/type_check_err.cpl` come test di sola compilazione
      (nessun `.out` accanto): i percorsi rifiutati terminano il programma, quindi
      non ci sono output attesi da confrontare. I rifiuti restano verificabili a
      mano: per ciascuno scritto nei commenti del file, compila ed esegui lo
      snippet isolato e controlla il messaggio atteso (`int` <- `"pippo"`
      all'assegnazione e all'inizializzazione, `not int` <- `nil`,
      `f("a")`, `h(nil)`, `array[int]` <- `"pippo"`).

## Workflow follow-up

- Dopo la revisione: `openspec archive type-checking` per fondere le specifiche
  nella spec principale (atteso: capability nuova, la sync crea la main spec dai
  requirements ADDED), poi verifica con `openspec list --specs`.
- Fuori perimetro del change (restano per futuri change): il check degli elementi
  in `P_STORESLICE`, il controllo del result type in `store_result` delle func, e
  la cattura degli `domain_error` in `main()` (oggi un rifiuto di tipo abortisce
  l'interprete, come gia' il "name not found").
