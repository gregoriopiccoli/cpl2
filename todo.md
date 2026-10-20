DA FARE: (12)

8. fare parametri posizionali
9. fare parametri per referenza
3. pCode delle classi
11. fare il check di tipo
10. fare esecuzione di import
4. la sintassi per i "tipi dipendenti"

FATTO:

7. non da errore quando dovrebbe aggiungere un nome che esiste già 
6. parametri con valori di default
1. specificare i file da compilare in cli
2. specificare i file da eseguire in cli
5. test test test test -> runner dei test `runtest` (change add-test-runner): coppia `.cpl`+`.out`
   per directory, confronto byte per byte sullo standard output, codice di uscita 0/1.
   Restano fuori perimetro: un formato di test piu' ricco (esito atteso, argomenti, stdin,
   timeout), il confronto dei listing p-code dentro il sorgente (resta manuale con `./asm < x.pcd`),
   il difetto noto di `benchPcd()` e il ramo `--test`/`-t` dell'interprete (convive con `runtest`).

