// Gestione in comune della command line di compiler e interpreter.
//
// Questo file non e' un'unita' di traduzione: viene incluso da parser.c e da
// interpreter.cpp, che sono l'una in C e l'altra in C++, quindi qui si usano
// solo costrutti validi nei due linguaggi. Non alloca e non libera: la tabella
// delle opzioni, l'array dei posizionali e la struttura di scansione li fornisce
// il chiamante (D2).
//
// Il modulo conosce la FORMA di un'opzione (nome lungo, nome corto, alias con un
// solo trattino, valore) ma non il vocabolario di nessuno dei due strumenti: la
// tabella la scrive ogni main(). E' quello che permette a compiler e interpreter
// di avere elenchi di opzioni diversi senza che questo file debba cambiare.
// Nessuno dei due binari dichiara oggi un alias, ma la forma resta supportata e
// provata da cli_test.c: se un giorno serve, basta riempire il campo alias di una
// riga di tabella, senza toccare questo file.

#include <stdio.h>
#include <string.h>

// tipo del valore di un'opzione: CLI_OPZIONE e' un semplice interruttore,
// CLI_VALORE chiede anche un testo
#define CLI_OPZIONE 0
#define CLI_VALORE  1

// come la scansione ha potuto fermarsi. CLI_ERRORE_NESSUNO vuol dire che la
// command line e' stata letta tutta
#define CLI_ERRORE_NESSUNO     0
#define CLI_ERRORE_SCONOSCIUTA 1 // opzione che non e' in tabella
#define CLI_ERRORE_VALORE      2 // opzione che vuole un valore e non l'ha ricevuto
#define CLI_ERRORE_NONVALORE   3 // opzione che non vuole un valore e l'ha ricevuto

typedef struct {
  const char* lungo;       // nome lungo senza "--", NULL se non esiste
  char        corto;       // nome corto senza "-", 0 se non esiste
  const char* alias;       // altra grafia con un solo trattino, NULL se non esiste
  const char* descrizione; // cosa fa l'opzione, per le istruzioni d'uso
  int         tipo;        // CLI_OPZIONE oppure CLI_VALORE
  void*       dest;        // int* per CLI_OPZIONE, const char** per CLI_VALORE
} cliOpzione;

// stato della scansione, inizializzato da cliInizia()
typedef struct {
  int argc;
  char** argv;
  int indice;          // argv[indice] e' il prossimo da guardare
  int nOpzioni;        // opzioni riconosciute fin qui
  int nPosizionali;    // posizionali raccolti fin qui
  int errore;          // CLI_ERRORE_* : perche' la scansione si e' fermata
  const char* erroreNome;   // l'argomento che ha causato l'errore
  const char* erroreValore; // il valore che l'ha seguita, se c'era
} cliScansione;

// lunghezza del nome dell'opzione, cioe' la parte che precede un eventuale '='.
// Serve perche' un token come "-b=on" nomina l'opzione "-b" e porta con se' il
// valore "on": confrontare solo sul nome evita di scambiare -b per un'altra.
static size_t cliNomeLen(const char* token){
	const char* eq=strchr(token,'=');
	return eq!=NULL?(size_t)(eq-token):strlen(token);
}

// cerca nella tabella l'opzione scritta in token, per nome lungo, nome corto o
// alias. *valore riceve il testo che segue l'eventuale '=', NULL se non c'e'.
// Restituisce NULL se l'opzione non e' in tabella.
static const cliOpzione* cliTrova(const cliOpzione* tab,int n,const char* token,const char** valore){
	int lungo=token[0]=='-'&&token[1]=='-';   // "--nome" contro "-n"
	const char* nome=lungo?token+2:token+1;
	size_t len=cliNomeLen(nome);
	const char* eq=strchr(nome,'=');
	int k;
	*valore=eq!=NULL?eq+1:NULL;
	for (k=0;k<n;k++){
		// il nome lungo si confronta con tutto il nome, il nome corto solo con
		// un carattere: cosi' "-bxyz" non viene scambiato per "-b" (D7)
		if (lungo){
			if (tab[k].lungo!=NULL&&strlen(tab[k].lungo)==len&&strncmp(tab[k].lungo,nome,len)==0)
				return &tab[k];
		}else{
			if (len==1&&tab[k].corto!=0&&tab[k].corto==nome[0])
				return &tab[k];
			// l'alias e' una parola intera preceduta da un solo trattino, come
			// "-verbose": non e' un nome lungo ne' un nome corto
			if (tab[k].alias!=NULL&&strlen(tab[k].alias)==len&&strncmp(tab[k].alias,nome,len)==0)
				return &tab[k];
		}
	}
	return NULL;
}

// inizializza la scansione: si comincia dal primo argomento, cioe' argv[1]
void cliInizia(cliScansione* s,int argc,char** argv){
	s->argc=argc;
	s->argv=argv;
	s->indice=1;
	s->nOpzioni=0;
	s->nPosizionali=0;
	s->errore=CLI_ERRORE_NESSUNO;
	s->erroreNome=NULL;
	s->erroreValore=NULL;
}

// scansiona tutta la command line: riconosce le opzioni dichiarate in tab,
// scrive i valori nei dest indicati e raccoglie in pos[] gli argomenti che
// sono percorsi. Restituisce quanti percorsi ha trovato: un numero maggiore di
// max vuol dire che sono troppi, e in quel caso in pos[] ne sono salvati solo i
// primi max. Le opzioni si riconoscono in qualunque posizione e "--" termina il
// riconoscimento (D3).
int cliScansiona(cliScansione* s,const cliOpzione* tab,int n,const char** pos,int max){
	while (s->indice<s->argc&&s->errore==CLI_ERRORE_NESSUNO){
		const char* a=s->argv[s->indice];
		const char* valore;
		const cliOpzione* op;
		// "--" dice che da qui in avanti non ci sono piu' opzioni
		if (strcmp(a,"--")==0){
			s->indice++;
			break;
		}
		// un argomento che comincia con "-" ed ha almeno un altro carattere e'
		// un'opzione; il solo "-" non lo e' e resta un percorso (D3)
		if (a[0]=='-'&&a[1]!='\0'){
			op=cliTrova(tab,n,a,&valore);
			if (op==NULL){
				s->errore=CLI_ERRORE_SCONOSCIUTA;
				s->erroreNome=a;
				s->erroreValore=valore;
				return s->nPosizionali;
			}
			if (op->tipo==CLI_VALORE){
				// il valore e' quello dopo l'equale, altrimenti quello
				// immediatamente successivo; se non c'e', e' un errore (D4)
				if (valore==NULL){
					if (s->indice+1>=s->argc){
						s->errore=CLI_ERRORE_VALORE;
						s->erroreNome=a;
						s->erroreValore=NULL;
						return s->nPosizionali;
					}
					s->indice++;
					valore=s->argv[s->indice];
				}
				// il valore resta grezzo: chi lo converte e' lo strumento (D9)
				*(const char**)op->dest = valore;
			}else{
				if (valore!=NULL){
					s->errore=CLI_ERRORE_NONVALORE;
					s->erroreNome=a;
					s->erroreValore=valore;
					return s->nPosizionali;
				}
				*(int*)op->dest = 1;
			}
			s->nOpzioni++;
		}else{
			if (s->nPosizionali<max) pos[s->nPosizionali]=a;
			s->nPosizionali++;
		}
		s->indice++;
	}
	// quello che segue "--" e' tutto posizionale, quindi viene preso per intero
	while (s->indice<s->argc){
		if (s->nPosizionali<max) pos[s->nPosizionali]=s->argv[s->indice];
		s->nPosizionali++;
		s->indice++;
	}
	return s->nPosizionali;
}

// istruzioni d'uso delle opzioni: una riga per ogni riga della tabella, costruita
// dagli stessi campi che usa la scansione, cosi' l'elenco delle opzioni non viene
// scritto due volte (D8)
void cliOpzioni(const cliOpzione* tab,int n,FILE* out){
	int k;
	for (k=0;k<n;k++){
		int primo=1;
		fprintf(out,"  ");
		if (tab[k].lungo!=NULL){fprintf(out,"--%s",tab[k].lungo);primo=0;}
		if (tab[k].corto!=0){fprintf(out,"%s-%c",primo?"":", ",tab[k].corto);primo=0;}
		// l'alias si scrive con un solo trattino, quindi va stampato con il
		// trattino: e' la grafia che l'utente scrive, non il nome interno
		if (tab[k].alias!=NULL){fprintf(out,"%s-%s",primo?"":", ",tab[k].alias);primo=0;}
		if (tab[k].tipo==CLI_VALORE) fprintf(out," <valore>");
		if (tab[k].descrizione!=NULL) fprintf(out,"  %s",tab[k].descrizione);
		fprintf(out,"\n");
	}
}