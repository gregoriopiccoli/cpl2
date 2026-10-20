# Design

## Context

Motivazione nel `proposal.md` - Why. Il punto di partenza e' `runtest.c`:

- `raccogli()` classifica i file della directory in due elenchi (`raccolta`): `test` (coppie
  `.cpl`+`.out`) e `orfani` (`.out` senza `.cpl`), entrambi ordinati per nome per garantire
  ripetibilita' (5.3).
- `eseguiTest()` fa tutto di fila per ogni test completo: copia il `.cpl` nella directory di
  appoggio (`mkdtemp`, D5), compila con `./compiler` sul file copiato, esegue con
  `./interpreter` catturando il solo stdout, confronta byte per byte con il `.out`.
- Il rapporto e' un `rapporto{superati,falliti}`; ogni passaggio fallito nomina il test su
  stderr via `fallito()` e il giro continua.

Vincoli da rispettare: la directory dei test non viene mai toccata (il `.pcd` va scritto
nell'appoggio perche' il compilatore lo scrive accanto al sorgente), il rapporto finisce su
stderr, l'ordine di esecuzione e' deterministico.

## Goals / Non-Goals

**Goals:**

- Un `.cpl` senza `.out` viene compilato, mai eseguito, e conta nel rapporto (superato se
  compila, fallito se no).
- Invariati: invarianti D3-D6 del runner (confronto byte per byte, appoggio, rapporto su
  stderr, codice di uscita), la coppia `.cpl`+`.out`, l' `.out` orfano, l'assenza di ricorsione.

**Non-Goals:**

- Nessuna opzione nuova di `runtest` (la tabella `tabOpzioni` resta vuota).
- Nessun cambiamento a `compiler`, `interpreter`, `cli.c` o al grammar.
- Nessuna distinzione nel rapporto fra i due tipi di test (stesso contatore, stesso formato).

## Decisions

**D1 — Terzo elenco in `raccolta`.** A `test` e `orfani` si aggiunge `soloCompilazione`: le
nomi base dei `.cpl` senza `.out`, raccolti nello stesso passaggio di `raccogli()` e ordinati
con lo stesso `confrontaNomi`. *Alternativa*: un elenco unico con un flag per voce — rifiutata:
spreca la struttura gia' esistente e costringe `eseguiTest()` a discriminare a ogni passaggio.

**D2 — Flag `soloCompilazione` su `eseguiTest()`.** La fase di copia + accesso a `./compiler`
+ compilazione e' identica per i due tipi di test: si passa un flag e, appena la compilazione
ha esito 0, il test di sola compilazione viene contato come superato e la funzione torna senza
toccare `./interpreter`. *Alternativa*: una funzione separata per i test di sola compilazione —
rifiutata: duplica copia, controllo del binario e diagnostiche di compilazione, e le due
versioni divergerebbero.

**D3 — Stesso contatore, stessa etichetta.** Un test di sola compilazione riuscito fa
`rap->superati++` e stampa `SUPERATO %s` come ogni altro; uno fallito passa da `fallito()`.
*Alternativa*: contatori separati (`compilati`/`compilatiFalliti`) — rifiutate: cambierebbero
il formato del rapporto finale e il codice di uscita senza bisogno, e la spec li conta tutti
fra i superati.

**D4 — Copia nell'appoggio anche per i test di sola compilazione.** Il compilatore scrive il
`.pcd` accanto al sorgente che riceve: compilare il `.cpl` della directory dei test violerebbe
l'invariante D5 ("la directory di test non viene toccata"). Si riusa quindi lo stesso passo di
copia di `eseguiTest()`, e il `.pcd` finisce nell'appoggio che viene comunque rimosso alla
fine del giro.

**D5 — Esecuzione in due fasi, ciascuna ordinata.** I test completi girano prima (come oggi),
poi i test di sola compilazione; entrambi gli elenchi sono gia' ordinati per nome, quindi il
giro resta deterministico e due giri consecutivi riportano lo stesso esito. *Alternativa*:
un unico elenco unito e riordinato per nome — non serve: la spec non richiede l'interleaving
fra i due tipi.

## Risks / Trade-offs

- [Gli scratchpad attuali diventano test: un `.cpl` di prova non compilabile fallisce la
  suite] → E' il comportamento voluto; tutti e sei i `.cpl` attuali senza `.out` compilano con
  esito 0 (verificato a mano prima di proporre il cambiamento). Chi lascia mezza prova nella
  directory dei test deve tenerla compilabile o cancellarla.
- [Un test di sola compilazione puo' produrre un `.pcd` non eseguibile senza che la suite se
  ne accorga] → Limite noto e accettato: coprire anche il runtime richiede un `.out` e un test
  completo; la categoria esiste proprio per i sorgenti le cui istruzioni p-code non sono
  implementate.
- [Duplicazione residua nella costruzione dei percorsi copia/compilazione] → Accettata: vale
  due passaggi e un flag, non un secondo ciclo di vita.

## Open Questions

<!-- nessuna: le scelte qui sopra non cambiano i requisiti della delta -->
