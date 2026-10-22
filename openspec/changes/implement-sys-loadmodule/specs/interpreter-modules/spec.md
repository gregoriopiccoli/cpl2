# Spec Delta

## Purpose

Definisce come l'interprete risolve, carica ed esegue i moduli p-code richiesti da un comando
`import`, con una cache per nome e un contesto di esecuzione separato per ogni modulo.

## ADDED Requirements

### Requirement: Derivazione del percorso di un modulo dal nome

Un nome di modulo è una sequenza di identificatori separati da punto. L'interprete SHALL
derivare il percorso del modulo sostituendo ogni punto con un separatore di directory,
accodando il suffisso `.pcd` e cercando il risultato nella directory del p-code che contiene
il comando `import`.

#### Scenario: Nome semplice

- **WHEN** un programma eseguito da `dir/mio.pcd` contiene `import a`
- **THEN** il modulo cercato è `dir/a.pcd`

#### Scenario: Nome puntato

- **WHEN** un programma eseguito da `dir/mio.pcd` contiene `import a.b.c`
- **THEN** il modulo cercato è `dir/a/b/c.pcd`

#### Scenario: Import dentro un modulo

- **WHEN** il modulo `dir/x/mod.pcd` contiene a sua volta `import a`
- **THEN** il modulo cercato è `dir/x/a.pcd`, cioè nella directory del modulo che importa e
  non in quella del programma principale

### Requirement: Caricamento del p-code del modulo

Prima di eseguirlo, l'interprete SHALL caricare il modulo come un qualsiasi file `.pcd`, con
l'intestazione e la terminazione richieste dal formato. Se il file non si può aprire, non è
p-code cpl2 o è incompleto, l'interprete SHALL riportare la diagnosi su stderr e SHALL
considerare fallito il caricamento, senza eseguire il modulo.

#### Scenario: Modulo valido

- **WHEN** il percorso derivato dal nome esiste, è p-code cpl2 valido e termina con il p-code
  di fine programma
- **THEN** il caricamento riesce e il modulo può essere eseguito

#### Scenario: Modulo assente

- **WHEN** il percorso derivato dal nome non esiste
- **THEN** l'interprete segnala il percorso su stderr e il modulo non viene eseguito

#### Scenario: File che non è p-code cpl2

- **WHEN** il percorso esiste ma non ha l'intestazione del formato
- **THEN** l'interprete segnala che non è un file cpl2 compilato e il modulo non viene eseguito

#### Scenario: P-code incompleto

- **WHEN** il percorso esiste ed è p-code cpl2 ma non termina con il p-code di fine programma
- **THEN** l'interprete segnala che il file ha errori e il modulo non viene eseguito

### Requirement: Esecuzione del modulo in un contesto separato

Ogni modulo SHALL essere eseguito in un contesto di esecuzione proprio, creato per il modulo,
che ha i builtin come superlivello. I nomi definiti al livello superiore del modulo SHALL NOT
diventare visibili nel contesto del programma che lo importa, e il contesto del modulo SHALL
restare vivo finché il processo esegue.

#### Scenario: I nomi del modulo non entrano nell'importatore

- **WHEN** un modulo definisce al livello superiore una variabile `x` e il programma che lo
  importa usa un nome `x` che non ha definito
- **THEN** il programma fallisce con l'errore di nome non trovato, perché il nome del modulo
  non è stato aggiunto al suo contesto

#### Scenario: Due moduli con lo stesso nome

- **WHEN** due moduli diversi definiscono al livello superiore una variabile con lo stesso
  nome e vengono importati dallo stesso programma
- **THEN** entrambi si eseguono senza l'errore di nome già presente nel contesto

#### Scenario: I builtin restano raggiungibili

- **WHEN** un modulo usa un builtin senza averlo definito
- **THEN** il modulo lo trova nel contesto dei builtin come superlivello del proprio contesto

#### Scenario: Stack del modulo vuoto

- **WHEN** l'esecuzione di un modulo termina
- **THEN** lo stack di esecuzione del modulo è vuoto, come al termine del programma principale

### Requirement: Cache dei moduli e import ciclici

L'interprete SHALL memorizzare in una cache, per nome, il programma e il contesto di ogni
modulo caricato. Un successivo `import` dello stesso nome SHALL NOT rieseguire il modulo.
L'iscrizione in cache SHALL avvenire prima dell'esecuzione, così un import diretto o indiretto
dello stesso modulo non causa esecuzione ricorsiva.

#### Scenario: Doppio import

- **WHEN** un programma esegue due volte `import a` e `a` produce un output quando viene
  eseguito
- **THEN** quell'output compare una sola volta

#### Scenario: Auto-import

- **WHEN** il modulo `a` contiene `import a` e un programma lo importa
- **THEN** il modulo si esegue una sola volta e il programma termina

#### Scenario: Import incrociato

- **WHEN** il modulo `a` importa `b` e il modulo `b` importa `a`, e un programma importa `a`
- **THEN** entrambi i moduli si eseguono una sola volta e il programma termina
