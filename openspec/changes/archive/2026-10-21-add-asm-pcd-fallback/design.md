# Design

## Context

`asm.c` `main()` riceve al massimo un percorso da `cliScansiona()`; senza percorso (o col
solo `-`) legge lo standard input, altrimenti fa `fopen(pos[0],"r")` e in caso di NULL
stampa l'errore ed esce con 1. L'interprete, per lo stesso problema, ha già la regola
D2 in `pcodePathFor()` (`interpreter.cpp`): se il nome di base non contiene punti,
accoda `.pcd`. La differenza è che l'interprete *deriva* il percorso (senza punto
accoda sempre, senza mai provare il nome digitato), mentre qui il fallback richiesto
prova prima il nome digitato e accoda `.pcd` solo se quel file non esiste. Motivo: per
`asm` esistono `.pcd` *e* listing testuali con la stessa radice, quindi il nome senza
estensione è ambiguo e la prima prova va lasciata al nome com'è scritto; per
l'interprete invece solo un `.pcd` è un input valido, quindi derivare direttamente non
perde niente.

## Goals / Non-Goals

**Goals:**

- `./asm mio` carica `mio.pcd` quando `mio` non esiste e `mio.pcd` sì
- zero cambio di comportamento quando il percorso digitato esiste, in modalità stdin,
  nei codici di uscita e nell'output (solo stdout, nessun file creato)
- la diagnostica d'errore nomina ancora il percorso digitato

**Non-Goals:**

- non si cambia l'interprete o il compilatore (la loro regola resta quella che è)
- non si accetta più di un percorso, non si dichiarano opzioni, non si tocca la
  conversione in entrambi i versi
- non si segue il link simbolico o altri trucchi: `fopen` e `fopen` basta

## Decisions

- **Fallback, non derive.** Si prova `fopen(pos[0])` e solo su NULL con errno ENOENT e
  nome di base senza punti si prova `pos[0]+".pcd"`. Alternativa considerata: estrarre
  una `pcodePathFor()` condivisa come quella dell'interprete e usarla da entrambi —
  rifiutata perché cambierebbe il comportamento dell'interprete (che non prova mai il
  nome digitato) violando il requisito `interpreter-cli` "Diagnostica e codice di uscita
  per un file non caricabile", che nomina il percorso *derivato*. Due tool, due regole
  diverse, documentate qui.
- **Solo ENOENT.** Il fallback scatta se e solo se `fopen` fallisce perché il file non
  esiste; un errore diverso (permessi) non innesca la seconda prova e viene riportato
  come prima. Alternativa: riprovare sempre — rifiutata perché con un file inesistente
  per motivi di permesso la seconda prova maschera il motivo vero.
- **Regola del punto uguale a D2.** Il punto si cerca nel solo nome di base (dopo
  l'ultimo `/`), quindi `dir.d/mio` prova `dir.d/mio` poi `dir.d/mio.pcd`. Il controllo
  è lo stesso tre righe di `pcodePathFor()`: piccolo, locale in `main()`, senza
  includere nulla dall'interprete (che è C++ e non è linkabile da `asm` in C).
- **Errore sul nome digitato.** Se falliscono entrambe le prove si stampa il percorso
  com'è stato scritto, come oggi. Le due `fopen` condividono il messaggio: un solo
  punto di stampa, due nomi possibili. Alternativa: nominare anche la variante `.pcd`
  provata — rifiutata per semplicità, il nome digitato basta a capire.

## Risks / Trade-offs

- [Un listing salvato senza estensione potrebbe essere mascherato da un `.pcd`] →
  accettabile e reversibile: digitando l'estensione esplicita (`./asm mio.txt`) si
  torna al comportamento di prima, e la prima prova è comunque il nome digitato.
- [Copiare la regola del punto invece di condividerla può farle divergere in futuro] →
  rischio basso (tre righe, la regola è stabile dal 2025); la commento in `asm.c`
  richiamando D2 come fa già `pcodePathFor()`.
- [`cli.c` resta intatto, quindi il fallback vive solo in `asm.c`] → voluto: la scansione
  delle opzioni non c'entra, e toccare `cli.c` richiederebbe la prova C/C++ dual
  language per un beneficio nullo.
