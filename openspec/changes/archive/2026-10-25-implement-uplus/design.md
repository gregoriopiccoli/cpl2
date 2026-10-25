# Design

## Context

Il comportamento richiesto è già quello del codice attuale (vedi proposal.md - Why). In
`cpl2.gram`:

```
fact ::= MINUS fact.                              {emit(P_UMINUS);}
fact ::= PLUS fact.                               // {printf("UPLUS\n");} non serve!
```

Una regola lemon senza azione non emette nulla: il p-code di `+x` è quello di `x`. Verificato a
mano con i binari attuali: `? +5` dà `INT_CONST 5`, `PRINT 1`; `? -+3` e `? +-3` danno entrambi
`INT_CONST 3`, `UMINUS`; `? +'a'` stampa `a`, `? +true` stampa `true`, exit 0. Il lexer assegna lo
stesso token `PLUS` al `+` binario e a quello unario; lemon risolve l'ambiguità dalla posizione
(`fact` contro `term PLUS mterm`), senza conflitti.

Il lavoro quindi, per le espressioni, non è implementare un meccanismo ma **fissarlo**:
documentarlo nella grammatica e coprirlo con un test, così che una modifica futura che lo cambi
faccia fallire `runtest`.

L'unico punto in cui il `+` unario manca davvero sono le costanti dei `case`. Le costanti dello
`switch` non passano da `expr` ma da `case_const`, che ha una produzione dedicata per il segno meno:

```
case_const ::= INT_CONST(value).        {emit_s(P_INT_CONST,value);dispose(&value);}
case_const ::= MINUS INT_CONST(value).  {char n[64];snprintf(n,sizeof(n),"-%s",value);emit_s(P_INT_CONST,n);dispose(&value);}
```

Nessuna produzione con `PLUS`, quindi oggi `case +1` dà `syntax error` (verificato: exit 1).

## Goals / Non-Goals

**Goals:**
- Commento esplicito sulla regola `fact ::= PLUS fact.` in `cpl2.gram`.
- `case +<intero>` accettato dallo `switch`, con lo stesso p-code di `case <intero>`.
- Test completo che inchiodi gli scenari di entrambe le spec.

**Non-Goals:**
- Qualunque modifica a `pcodes.h`, `pcodes.c`, `interpreter.cpp`, `asm.c`.
- Toccare la produzione `fact ::= PLUS fact.` in sé (né spostarla né aggiungerle un'azione).
- `+` davanti a costanti non intere nei `case`, `+` sui float, controllo di tipo a compile time
  (vedi proposal.md).

## Decisions

**1. Nessun p-code `P_UPLUS`.** Alternativa considerata: un `P_UPLUS` che controlli che l'operando
sia numerico (simmetrico a `UMINUS`, che fallisce su stringhe e bool). Scartata per scelta
esplicita: il `+` unario è una convenzione sintattica, e un'istruzione in più per ogni `+x`
costerebbe un dispatch senza cambiare il valore. Il prezzo è che `+'a'` è accettato; la spec lo
rende un comportamento voluto e il test lo fissa.

**2. Il commento in italiano sostituisce quello provvisorio, sulla stessa riga.** Il nuovo
commento dice che il `+` unario non emette p-code e non controlla il tipo, e rimanda a `UMINUS`
per il contrasto. Va scritto come commento `//` fuori dalle graffe: non è un'azione, quindi la
regola continua a non averne. Dopo l'edit (e quello della decisione 4) si rigenera con il `lemon`
installato (`make` ha la regola per `cpl2.c`; `cpl2.h` esce dalla stessa invocazione). Il commento
da solo non entrerebbe nel codice generato, ma la nuova produzione di `case_const` sì: il diff di
`cpl2.c` cambia anche le tabelle del parser, oltre all'eventuale boilerplate di una diversa
versione di lemon. Si committa così com'è (AGENTS.md: mai riconciliare a mano).

**3. Test completo, non compile-only.** Il p-code identico (`? +5` contro `? 5`) non è controllato
da `runtest`, che confronta solo l'output; lo si verifica a mano con `cmp` durante l'apply. Il test
completo copre i valori, compresi `+'a'` e `+true` che qui, a differenza di `-'a'` in
`test/uminus.cpl`, possono stare nel `.out` perché non fanno abortire l'interprete. Gli scenari
dello `switch` con `case +1` vanno in `test/uplus.cpl`, non in `test/switch.cpl`, che resta
invariato. `case +'a'` non entra nel test: un sorgente che non compila farebbe fallire `runtest`,
quindi si verifica a mano.

**4. Una produzione `PLUS INT_CONST`, simmetrica a `MINUS INT_CONST`.** Si aggiunge
`case_const ::= PLUS INT_CONST(value).` con azione `emit_s(P_INT_CONST,value);dispose(&value);`,
cioè la stessa di `case_const ::= INT_CONST(value).`: il testo della costante si emette così com'è,
senza aggiungere il `+`, e il `.pcd` di `case +1` è identico a quello di `case 1`. Alternativa
scartata: `case_const ::= PLUS case_const.`, che accetterebbe anche `case +'a'`, `case +true`,
`case +nil` e `case + -1`, mentre il `-` dei `case` vale solo per gli interi. Restare simmetrici al
`-` tiene il `+` dei `case` più stretto del `+` delle espressioni (che accetta qualunque tipo): è
voluto, perché le costanti dei `case` sono letterali, non espressioni.

## Risks / Trade-offs

- [Un `+` unario su un tipo non numerico passa in silenzio] → È il comportamento scelto; se un
  giorno servirà il controllo, sarà un change che modifica il requisito "Il più unario non controlla
  il tipo" e lo fa fallire nel test.
- [`print +5` potrebbe essere letto come `print + 5`, come già succede per `print -5`] → Il test usa
  solo `?`, mai `print +...`.
- [Rigenerare `cpl2.c` con un lemon diverso produce un diff rumoroso] → Atteso e accettato da
  AGENTS.md; nessuna modifica a mano.
- [La nuova produzione introduce conflitti nel parser] → Dopo `CASE` (o dopo la `COMMA` di
  `case_test`) il `PLUS` può solo iniziare un `case_const`, come oggi il `MINUS`: non ci si
  aspettano conflitti. Si verifica che lemon esca 0 senza segnalarne.
