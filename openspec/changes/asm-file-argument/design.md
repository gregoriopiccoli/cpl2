# Design

## Context

`asm.c` e' minuscolo: `cpl2asm(FILE*)` fa già tutto il lavoro partendo da un `FILE*`,
e `main()` attualmente la chiama con `stdin` e esce con il suo valore di ritorno.
I codici di uscita oggi sono: 0 = la prima riga non e' p-code, 1 = traduzione
listing→binario riuscita, 2 = binario→listing riuscita.

Il progetto ha gia' un scanner di command line condiviso, `cli.c`, incluso sia in
`parser.c` sia in `interpreter.cpp`: conosce la *forma* delle opzioni (lunga, corta,
alias, `--`, il solo `-` come percorso) e non il vocabolario, che ogni binario
dichiara con la propria tabella. `parser.c` ha oggi una tabella volutamente vuota.

## Goals / Non-Goals

**Goals:**
- `asm` legge da un file quando gli viene indicato un percorso, senza cambiare il
  resto del comportamento osservabile (formato dell'output, codici di uscita della
  conversione, modalità stdin).
- Stessa sintassi della command line di `compiler` e `interpreter` per opzioni
  sconosciute, `--` e `-`.

**Non-Goals:**
- Opzioni vere per `asm` (nessuna e' richiesta: la tabella resta vuota).
- Scrittura dell'output su file o opzione `-o`.
- Cambiare i codici di uscita 0/1/2 della conversione, che restano come sono.
- Cambi a `runtest` o ai test di `test/`.

## Decisions

- **Riusare `cli.c` con tabella vuota, `max=1` posizionale.** E' esattamente il caso
  d'uso per cui `cli.c` e' stato scritto: `parser.c` dimostra gia' la tabella vuota.
  Alternativa considerata: un parsing a mano in `main()` — rigettato perche'
  duplicherebbe la sintassi di `--`, del solo `-` e delle opzioni sconosciute,
  che invece resterebbero uniformi ai altri due binari gratis. `asm.c` include
  `cli.c` come includera' `pcodes.c` ed `emit.c`; il file resta C puro e il Makefile
  non cambia (`cc asm.c`).
- **Il valore di ritorno di `cliScansiona` tiene il percorso.** Con `max=1` i
  posizionali eccedenti non entrano nel buffer ma il conteggio li denuncia:
  piu' di un posizionale, oppure `errore != CLI_ERRORE_NESSUNO`, e' errore di
  command line → messaggio su stderr, exit 1, nessuna traduzione.
- **Nessun percorso → `stdin`, un percorso → `fopen(...,"r")`.** La differenza tra
  "non ha aperto il file" e "nessun argomento" si decide prima di chiamare
  `cpl2asm`, che resta intoccata: le sue due `fgets` di intestazione e il resto del
  ciclo funzionano identiche su qualunque `FILE*`.
- **Codice di uscita degli errori di command line/apertura: 1.** E' il codice che
  `compiler` (opzione sconosciuta, file non apribile) e `interpreter` (file non
  caricabile) usano gia' per le stesse condizioni, e il caso d'uso di `asm` e'
  interattivo/scriptato a mano.
- **Aggiornare `cli.c` solo nei commenti.** La sua intestazione nomina "compiler e
  interpreter": con un terzo binario che lo include il commento va allargato a `asm`,
  senza cambiare codice.

## Risks / Trade-offs

- [Exit 1 ambiguo: significa sia "errore di command line/apertura" sia "listing→
  binario riuscita", che gia' esce con 1] → I due casi non si sovrappongono mai
  nell'uso reale (un errore non produce listing e va sempre accompagnato dal
  messaggio su stderr), e cambiare i codici di conversione sara' un breaking
  change separato, se mai servira'.
- [`cli.c` e' scritto per restare valido sia come C sia come C++, ma `asm.c` e' solo
  C] → Nessun rischio aggiuntivo: l'inclusione da C e' gia' provata da `cli_test.c`.
- [Il Makefile non ha regole di dipendenza] → `cc asm.c` rilegge `cli.c` a ogni
  build, quindi nessuna build obsoleta; niente da cambiare.

## Migration Plan

Nessuna: e' un'aggiunta retrocompatibile. La rollback e' il singolo commit.

## Open Questions

Nessuna.
