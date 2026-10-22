# Design

## Context

Vedi `proposal.md` per la motivazione. Nello stato attuale `pcodes.h` definisce già
`P_IMPORTID` (103), `P_IMPORTIDX` (104) e `P_IMPORT` (101), `pcodes.c` ne registra i testi e
`cpl2.gram` li emette: `import a` produce `IMPORTID a` + `IMPORT 1`, `import a.b.c` produce
`IMPORTID a`, `IMPORTIDX b`, `IMPORTIDX c`, `IMPORT 1`, e una lista con la virgola produce un
`IMPORT N` con N nomi già ridotti a una stringa ciascuno. Manca solo il lato interprete:
`makePCode()` non ha i tre `case` e li lascia cadere in `pcodeNotImpl`.

I vincoli che contano:

- `pcodeImportId`/`pcodeImportIdx` hanno un operando stringa, quindi possono derivare da
  `spcode` come già fa `pcodeStrConst`; `pcodeImport` ha un operando intero e deriva da `ipcode`.
- gli oggetti valore sono `strObj` (campo pubblico `value`, `print()` che restituisce il valore).
- lo stack è `interp::stack` più l'intero `sp`; `runPCode()` fa `assert(intp.sp==-1)` dopo
  `run()`, quindi un `import` che lascia stringhe sullo stack farebbe abortire il programma.
- la stringa è già un oggetto GC: finché è sullo stack è raggiungibile, dopo il pop non serve
  conservarla.

## Goals / Non-Goals

**Goals:**

- eseguire i tre p-code con gli effetti sullo stack descritti nella delta spec;
- lasciare lo stack esattamente alla profondità iniziale dopo ogni comando `import`;
- rendere eseguibile un sorgente con soli `import` senza toccare grammatica, tabelle dei
  p-code o `asm`.

**Non-Goals:**

- nessuna inclusione, risoluzione o cache dei moduli, nessun uso di `sys::modules`;
- nessun supporto a `from`/`FROM`: `test/simple_import.cpl` resta di sola compilazione;
- nessuna modifica al formato `.pcd`, a `pcodes.h`/`pcodes.c`, a `MAX_PCODE` o a `asm.c`;
- nessuna ottimizzazione per il dispatch: questi p-code non sono caldi e non entrano in
  `pcodejump[]`.

## Decisions

### Tre classi dedicate, registrate solo in `makePCode()`

In `interpreter.cpp` si aggiungono `pcodeImportId : spcode`, `pcodeImportIdx : spcode` e
`pcodeImport : ipcode`, con i rispettivi `exec()`, e tre `case` in `makePCode()`:
`P_IMPORTID`, `P_IMPORTIDX`, `P_IMPORT`. Alternativa considerata: riusare `pcodeStrConst` per
`IMPORTID` — rifiutata perché il valore deve essere una stringa nuova, non l'oggetto condiviso
della costante, e perché `IMPORTIDX`/`IMPORT` hanno comunque bisogno di classi proprie.
Non si aggiungono voci a `pcodejump[]`: la duplicazione del dispatch è voluta e questi p-code
non sono nel ciclo caldo.

### IMPORTID costruisce una stringa nuova

`pcodeImportId::exec()` fa `sp++` e `stack.push_back(new strObj(value))`. Il valore è già
nell'operando del p-code (letto dal `.pcd` da `spcode`), quindi non serve interning né una
tabella ausiliaria. La stringa è tracciata dallo stack GC come qualunque altro valore.

### IMPORTIDX concatena precedente, punto e nuovo identificatore

`pcodeImportIdx::exec()` spinge prima `new strObj(value)` come richiesto, poi legge le due
stringhe in cima, verifica con `dynamic_cast<const strObj*>` che siano davvero stringhe e
sostituisce la coppia con `new strObj(previous->value + "." + value)`, quindi fa un solo
`pop_back()` e un solo `sp--`. La profondità netta resta invariata. Alternativa considerata:
chiamare `obj::plus()` due volte (`previous + "."` poi `+ id`) — scartata perché crea un oggetto
intermedio in più e sposta la gestione dell'errore dentro `strObj`, mentre qui l'ordine delle
operande è esplicito. Se sotto l'identificatore non c'è una stringa si solleva
`domain_error`, come fanno gli altri p-code quando il tipo non è quello atteso; la grammatica
garantisce comunque che `IMPORTIDX` sia preceduto da un `IMPORTID`.

### IMPORT stampa e consuma N nomi, N dall'operando

`pcodeImport::exec()` legge N da `value` (l'operando intero emesso dalla grammatica come numero di
elementi della lista `import`), verifica che lo stack abbia almeno N elementi (`sp>=N-1`) e che le N
posizioni in cima siano `strObj`, altrimenti solleva `domain_error` prima di stampare. Poi stampa i
nomi dal basso verso la cima del gruppo, uno per riga (`cout << o->print() << endl`), ed esegue N
volte `pop_back()` e `sp--`, lasciando intatto ciò che sta sotto il gruppo. L'ordine di stampa è
quello della lista `import`. Il consumo è obbligatorio per due motivi: senza di esso `runPCode()`
violerebbe `assert(sp==-1)`, e un secondo `import` ritroverebbe i nomi del primo. Alternativa
considerata: contare le `strObj` consecutive in cima ignorando N — rifiutata perché N è già il numero
esatto di nomi prodotto dalla lista e l'operando è la fonte autorevole; contare lo stack rischierebbe
di consumare valori estranei.

### Output diagnostico volutamente minimale

I nomi vengono stampati da soli, uno per riga, senza prefissi: è un output temporaneo di controllo,
non un formato stabile. Quando arriverà l'inclusione vera, questo `cout` verrà sostituito dall'uso
effettivo dei nomi; il test completo andrà aggiornato in quel change. Alternativa considerata:
`import: <nome>` — scartata perché non richiesta e perché renderebbe più rumoroso il confronto con
l'output dei programmi veri.

### Test completo dedicato, `simple_import` invariato

Si aggiunge `test/import_stack.cpl` con import semplici, puntati e multipli e una stampa finale,
più `test/import_stack.out` con i nomi attesi (`a`, `a.b.c`, `a.b`, `c.d`) e la riga finale. Il test
è completo perché `runtest` confronta stdout byte per byte e quindi verifica insieme stack e output.
`test/simple_import.cpl` non riceve un `.out`: contiene `from`, che resta `pcodeNotImpl`, e deve
continuare a essere solo compilato.

## Risks / Trade-offs

- [`IMPORT` consuma i nomi, quindi non saranno disponibili per la futura inclusione] → è un
  comportamento temporaneo e voluto: la vera inclusione cambierà `IMPORT` e il suo test in un
  change separato; qui serve solo a tenere lo stack pulito.
- [`from` resta non implementato e un programma misto esplode ancora] → accettato e coperto dal
  test di sola compilazione esistente; la delta spec riguarda solo `import` senza `from`.
- [L'output sono i nomi nudi, poco leggibili in un programma reale] → è diagnostica temporanea;
  il test lo fissa esplicitamente così il cambio futuro è visibile.
- [N dell'operando non corrisponde alle stringhe sullo stack] → la grammatica garantisce una
  stringa per ogni nome della lista; in ogni caso `IMPORT` controlla profondità e tipo e solleva
  `domain_error` prima di stampare, invece di leggere fuori dallo stack.
- [Un `IMPORTIDX` senza stringa sotto solleva `domain_error`] → la grammatica non può produrlo
  oggi; l'errore è preferibile a una concatenazione silenziosa su un tipo sbagliato.
- [Nuove stringhe allocate a ogni esecuzione dei p-code] → gli `import` sono pochi e a livello
  di comando; il GC le recupera appena vengono poppate, quindi non vale la complessità di una
  cache come quella di `pcodeStrConst`.

## Migration Plan

Nessuna migrazione di dati o formati: il `.pcd` resta identico e i vecchi file compilati
funzionano senza ricompilare. Rollback: rimuovere i tre `case` da `makePCode()` e le tre classi;
i p-code tornano a `pcodeNotImpl`. Il test nuovo e la riga di `AGENTS.md` vanno rimossi insieme.
