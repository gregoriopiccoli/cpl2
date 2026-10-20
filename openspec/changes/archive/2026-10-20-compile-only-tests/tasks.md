# Tasks

## 1. Raccolta dei test di sola compilazione

- [x] 1.1 In `runtest.c` aggiungere a `raccolta` l'elenco `soloCompilazione` (con il suo
      contatore), raccogliervi i `.cpl` senza `.out` in `raccogli()` e ordinarlo con lo stesso
      `confrontaNomi`; verificare con `make && ./runtest` che il giro resti verde e che i sei
      `.cpl` senza `.out` di `test/` siano conteggiati.
- [x] 1.2 Aggiornare in testa a `runtest.c` il commento che definisce "un `.cpl` senza `.out`
      non e' un test", verificando con `grep -n "scratchpad" runtest.c` che la vecchia
      formulazione non sopravviva.

## 2. Esecuzione in sola compilazione

- [x] 2.1 Passare a `eseguiTest()` il flag `soloCompilazione`: dopo la compilazione con esito
      0 il test conta fra i superati (con la solita riga `SUPERATO %s`) e la funzione torna
      senza invocare `./interpreter`; verificare con `./runtest` che i test completi siano
      invariati e che `test_1` sia ancora confrontato col suo `.out`.
- [x] 2.2 Verificare il percorso di fallimento: creare in `tmp/prove` un `.cpl` non compilabile
      senza `.out` accanto, eseguire `./runtest tmp/prove` e controllare che il rapporto lo
      nomini come fallito, che il codice di uscita sia 1 e che `tmp/prove` non contenga file
      nuovi.

## 3. Documentazione

- [x] 3.1 Riscrivere in `AGENTS.md` (sezione "Tests") la regola sui `.cpl` senza `.out` come
      test di sola compilazione contati nel rapporto, e aggiornare l'elenco dei file che non
      sono test (`test_expr1.cpl`, `test_stm1.cpl`, `test_var1.cpl` lo sono diventati);
      verificare che il testo descriva il comportamento osservato di `./runtest`.

## 4. Verifica di integrazione

- [x] 4.1 `make && ./runtest > /dev/null`: tutti i test superati, codice di uscita 0, e nel
      rapporto compaiono anche `simple_class`, `simple_func`, `simple_import`, `test_expr1`,
      `test_stm1`, `test_var1`.
- [x] 4.2 Eseguire `./runtest` due volte di seguito e verificare gli identici conteggi e
      nessun file lasciato nella directory dei test (`git status test/` pulito tranne i
      `.pcd` eventualmente preesistenti).

## Workflow follow-up

- Archiviare il cambio (`openspec archive`) dopo la revisione, per aggiornare la
  specifica principale di `test-suite`.
