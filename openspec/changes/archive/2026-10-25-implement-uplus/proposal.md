# Proposal

## Why

Il `+` unario esiste già nella grammatica (`fact ::= PLUS fact.` in `cpl2.gram`), ma solo come
regola senza azione, accompagnata da un commento provvisorio (`// {printf("UPLUS\n");} non serve!`).
Il comportamento c'è (`? +5` stampa `5`), però nessuna spec lo descrive e nessun test lo protegge.
La scelta è che `UPLUS` sia una **convenzione della grammatica**: il compilatore accetta `+x` e lo
tratta esattamente come `x`, **senza emettere alcun p-code**. Va messo nero su bianco prima che
qualcuno aggiunga un `P_UPLUS` "per simmetria" con `UMINUS`.

## What Changes

- Si fissa la semantica del `+` unario: `+ <fattore>` compila nello stesso p-code di `<fattore>`;
  nessun p-code nuovo, nessun `P_UPLUS` in `pcodes.h`/`pcodes.c`, nessuna classe in
  `interpreter.cpp`.
- Conseguenza voluta (non un bug): non essendoci un p-code, non c'è nemmeno un controllo di tipo a
  runtime. `+'a'` stampa `a`, `+true` stampa `true`, a differenza di `-'a'` che fallisce con
  `uminus not implemented`.
- In `cpl2.gram` il commento provvisorio sulla regola `fact ::= PLUS fact.` diventa un commento
  in italiano che spiega la convenzione.
- Lo `switch` accetta il `+` unario anche nelle costanti dei `case`: nuova regola
  `case_const ::= PLUS INT_CONST.` accanto a quella con `MINUS`. `case +1` emette `INT_CONST 1`,
  esattamente come `case 1`, senza p-code aggiuntivi. Solo davanti agli interi, come il `-`:
  `case +'a'` e `case +true` restano errori di sintassi. Oggi `case +1` è un errore di sintassi.
- Nuovo test completo `test/uplus.cpl` + `test/uplus.out`.
- `AGENTS.md`: il paragrafo sullo `switch` cita anche `case +1`.

Fuori scope: `+` unario sui float (i `FLOAT_CONST` non sono eseguibili);
un controllo di tipo a compile time sull'operando del `+` unario.

## Capabilities

### New Capabilities

- `unary-plus`: semantica del `+` unario nelle espressioni di cpl2 — identità sull'operando, nessun
  p-code emesso, nessun controllo di tipo.

### Modified Capabilities

- `switch-statement`: nuovo requisito per le costanti intere dei `case` scritte con il `+` unario.

## Impact

- `cpl2.gram`: commento sulla regola `fact ::= PLUS fact.` e nuova produzione
  `case_const ::= PLUS INT_CONST.`; `cpl2.c`/`cpl2.h` rigenerati (cambiano anche le tabelle del
  parser, non solo il boilerplate).
- `test/uplus.cpl`, `test/uplus.out`, `AGENTS.md`.
- Nessun impatto su `pcodes.h`, `pcodes.c`, `interpreter.cpp`, `asm.c`, formato del `.pcd` o altri
  test: il p-code dei sorgenti che compilano già oggi resta identico.
