# Proposal

## Why

Oggi `asm` si può usare solo in pipeline (`./asm < fib.pcd`): il file da tradurre va
obbligato sulla command line dello shell. Un parametro file, come già lo ha il
compilatore e l'interprete, elimina la redirezione e rende lo strumento comodo da
invocare a mano e dagli script.

## What Changes

- `asm` accetta al massimo un percorso come argomento e legge da quel file invece che
  dallo standard input.
- Senza argomenti il comportamento resta identico a oggi: lettura da stdin.
- Lo standard output resta l'unica destinazione dell'output, in entrambe le modalità:
  niente file di output, niente cambi al formato della listing.
- Un file che non si può aprire viene segnalato su stderr e il programma termina con
  codice di uscita 1 senza produrre output.
- Un argomento che inizia con `-` (e non è il solo `-`, né dopo `--`) è un'opzione
  sconosciuta: `asm` non dichiara opzioni, quindi la segnala su stderr e termina con
  codice 1; il solo `-` e il terminatore `--` indicano esplicitamente il trattino o il
  proseguimento come percorso.

## Capabilities

### New Capabilities
- `asm-cli`: il contratto della command line di `asm` — da dove legge (file o stdin),
  dove scrive (sempre stdout), come riporta gli errori di apertura/uso e con quale
  codice di uscita termina.

### Modified Capabilities

## Impact

- `asm.c` — `main()` legge `argv` e apre il file passato; `cpl2asm(FILE*)` e il resto
  restano invariati (la funzione già prende un `FILE*`).
- Nessuna change a `pcodes.*`, `emit.c`, al Makefile o al formato `.pcd`.
- Nessuna change a `compiler`, `interpreter` o ai test di `test/`.
- `AGENTS.md` (sezione "asm is a checker...") andrà aggiornato con la nuova forma
  d'uso.
