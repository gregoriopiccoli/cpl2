# Design

## Context

Tutta la catena è già in piedi tranne l'ultimo anello (vedi proposal.md - Why):

- `cpl2.gram`: `fact ::= MINUS fact. {emit(P_UMINUS);}` — `? -5` produce `INT_CONST 5`, `UMINUS`,
  `PRINT 1`; `? 3 - -2` produce `INT_CONST 3`, `INT_CONST 2`, `UMINUS`, `MINUS`.
- `interpreter.cpp`: `case P_UMINUS: return new pcodeUMinus();` in `makePCode()`, e
  `pcodeUMinus::exec` fa `stack[sp] = stack[sp]->uminus();`.
- `obj::uminus()` lancia `domain_error("uminus not implemented")`, e nessuna sottoclasse lo
  ridefinisce.

Vincolo importante: l'oggetto in cima allo stack spesso **non è di proprietà** del p-code. Per una
costante è il `theValue` condiviso di `pcodeIntConst` (lo stesso oggetto a ogni esecuzione di
quell'istruzione); per una variabile è lo stesso oggetto contenuto nel contesto.

## Goals / Non-Goals

**Goals:**
- `intObj::uminus()` che restituisce un nuovo `intObj` con il valore negato.

**Non-Goals:**
- Toccare `pcodeUMinus::exec`, `makePCode()`, la grammatica o `pcodejump[]`.
- Ottimizzazioni a compile time (piegare `INT_CONST 5` + `UMINUS` in `INT_CONST -5`): il
  compilatore emette p-code direttamente dalle azioni della grammatica, senza un passo di
  ottimizzazione, e il `case -1` dello `switch` già gestisce il proprio caso a parte.
- Meno unario sui float (vedi proposal.md).

## Decisions

**1. Nuovo oggetto, mai mutazione in place.** `uminus()` è `const` nella base e resta `const`
nell'override: restituisce un oggetto nuovo. Mutare `value` sull'oggetto in cima allo stack
corromperebbe la costante condivisa di `INT_CONST` (un ciclo con `? -5` stamperebbe `-5`, `5`,
`-5`, ...) o la variabile da cui è stato caricato il valore. Lo scenario "Costante rieseguita in un
ciclo" della spec esiste per inchiodare questo punto.

**2. Stesso schema di allocazione di `plus`/`minus`.** `intObj::plus` e `intObj::minus` riciclano
un `intObj` da `theIntCache` quando disponibile (`stdGC().addRecycled(v)`, `intrecycle++`) e
altrimenti fanno `new intObj(...)`. `uminus` usa lo stesso schema: è il terzo operatore intero
che può comparire in un ciclo caldo e tenerlo allineato agli altri due evita un caso speciale per
il GC. Alternativa scartata: solo `new intObj(-value)` come `mult`/`div` — funzionerebbe, ma
rinuncia al riciclo senza motivo.

**3. Overflow di `INT_MIN`.** `-value` con `value == INT_MIN` è comportamento indefinito in C++.
Il resto dell'aritmetica intera (`plus`, `minus`, `mult`) non controlla l'overflow, e il lexer non
permette nemmeno di scrivere `-2147483648` come costante senza passare da un'espressione. Si
calcola la negazione in aritmetica senza segno (`(int)(0u - (unsigned)value)`), che dà il wrap a
complemento a due (`-INT_MIN == INT_MIN`) senza UB, coerente con quello che `plus`/`minus` fanno
in pratica. Alternativa scartata: lanciare `domain_error` sull'overflow — sarebbe l'unico
operatore intero a controllarlo.

**4. Tipi non interi: nessun cambiamento.** Restano sulla `obj::uminus()` base, che lancia
`domain_error("uminus not implemented")`. Nessun override per `boolObj`/`strObj`: il meno unario
su di essi non ha significato in cpl2.

**5. Nessuna voce in `pcodejump[]`.** `UMINUS` passa dal fallback `default_pcode` →
`pcode::exec`. Non è abbastanza frequente da meritare il fast path (AGENTS.md: aggiungerlo solo
se "genuinely hot").

## Risks / Trade-offs

- [`print -5` viene letto come `print - 5`, cioè meno binario fra la funzione `print` e `5`, e
  fallisce a runtime con `minus not implemented`] → fuori scope (grammatica); il test usa `?`, che
  non ha questa ambiguità.
- [Il test non può coprire `INT_MIN`, perché non si può scrivere quella costante in un sorgente
  senza overflow in un'altra operazione] → la decisione 3 resta verificata solo per lettura del
  codice.
- [Uno scenario d'errore (`? -'a'`) non entra nel test completo, perché `runtest` confronta solo lo
  stdout e un'eccezione non catturata abortisce l'interprete] → verificato a mano in `tasks.md`.
