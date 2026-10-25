#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "cpl2.h"
#include "pcodes.h"

const char* separators="+-*/%()[]{}!|<>= \t\r\n.:;,@?";
const char* blankchars=" \t\r\n";
const char* numberchars="0123456789";

// percorso del sorgente in compilazione, NULL in modalita' standard input
const char* srcFile=NULL;

// diagnostica su stderr, con il percorso del sorgente come prefisso quando e' noto.
// Tutte le diagnostiche del compilatore passano da qui: scanner, errore di sintassi
// della grammatica e gestione dei file indicati in command line.
void diag(const char* fmt, ...){
	va_list args;
	if (srcFile!=NULL)
		fprintf(stderr,"%s: ",srcFile);
	va_start(args,fmt);
	vfprintf(stderr,fmt,args);
	va_end(args);
}

int getTokenCodeFromString(const char* token){
	switch(token[0]){
		case '>': return(TOK_GT);
		case '<': return(TOK_LT);
		case '(': return(TOK_LPAR);
		case ')': return(TOK_RPAR);
		case '[': return(TOK_LSQ);
		case ']': return(TOK_RSQ);
		case '{': return(TOK_LBR);
		case '}': return(TOK_RBR);
		case '+': return(TOK_PLUS);
		case '-': return(TOK_MINUS);
		case '*': return(TOK_MULT);
		case '/': return(TOK_DIV);
		case '%': return(TOK_MOD);
		case '=': return(TOK_EQ);
		case '?': return(TOK_QUESTIONMARK);
		case '!': return(TOK_NOT);
		case '.': return(TOK_DOT);
		case ',': return(TOK_COMMA);
		case ':': return(TOK_COLON);
		case '|': return(TOK_VBAR);
		case ';': return(TOK_EOL);
		case '@': return(TOK_AT);
		case 'a':
		  if (strcmp(token,"and")==0) return TOK_AND;
		  if (strcmp(token,"array")==0) return TOK_ARRAY;
		  if (strcmp(token,"any")==0) return TOK_ANY;	
		  if (strcmp(token,"abstract")==0) return TOK_ABSTRACT;	  
		  if (strcmp(token,"assert")==0) return TOK_ASSERT;	  
		  if (strcmp(token,"assume")==0) return TOK_ASSUME;	  
		  break;
		case 'b':
		  if (strcmp(token,"bool")==0) return TOK_BOOL;
		  break;  
		case 'c':
		  if (strcmp(token,"class")==0) return TOK_CLASS;
		  if (strcmp(token,"case")==0) return TOK_CASE;
		  if (strcmp(token,"catch")==0) return TOK_CATCH;
		  if (strcmp(token,"const")==0) return TOK_CONST;
		  break;  
		case 'd':
		  if (strcmp(token,"dict")==0) return TOK_DICT;
		  if (strcmp(token,"date")==0) return TOK_DATE;
		  if (strcmp(token,"datetime")==0) return TOK_DATETIME;
		  if (strcmp(token,"downto")==0) return TOK_DOWNTO;
		  if (strcmp(token,"div")==0) return TOK_IDIV;
  		  break;  
		case 'e':
		  if (strcmp(token,"end")==0) return TOK_END;
		  if (strcmp(token,"else")==0) return TOK_ELSE;
		  if (strcmp(token,"each")==0) return TOK_EACH;
		  if (strcmp(token,"enum")==0) return TOK_ENUM;
		  if (strcmp(token,"elseif")==0) return TOK_ELSEIF;
		  if (strcmp(token,"event")==0) return TOK_EVENT;
		  if (strcmp(token,"ensure")==0) return TOK_ENSURE;
		 break;  
		case 'f':
		  if (strcmp(token,"func")==0) return TOK_FUNC;
		  if (strcmp(token,"for")==0) return TOK_FOR;
		  if (strcmp(token,"false")==0) return TOK_FALSE;
		  if (strcmp(token,"from")==0) return TOK_FROM;
		  if (strcmp(token,"float")==0) return TOK_FLOAT;
		  if (strcmp(token,"finally")==0) return TOK_FINALLY;
		  break;  
		case 'i':
		  if (strcmp(token,"int")==0) return TOK_INT;  
		  if (strcmp(token,"if")==0) return TOK_IF;  
		  if (strcmp(token,"iif")==0) return TOK_IIF;  
		  if (strcmp(token,"import")==0) return TOK_IMPORT;  
		  if (strcmp(token,"in")==0) return TOK_IN;  
		  if (strcmp(token,"is")==0) return TOK_IS;  
		  if (strcmp(token,"inherited")==0) return TOK_INHERITED;  
		  if (strcmp(token,"invariant")==0) return TOK_INVARIANT;  
		  break;
		case 'l':
		  if (strcmp(token,"local")==0) return TOK_LOCAL;  
		  break;
		case 'm':
		  if (strcmp(token,"mod")==0) return TOK_MOD;  
		  break;
		case 'n':
		  if (strcmp(token,"nil")==0) return TOK_NIL;  
		  if (strcmp(token,"not")==0) return TOK_NOT;  
		  break;
		case 'o':
		  if (strcmp(token,"or")==0) return TOK_OR;
		  if (strcmp(token,"observer")==0) return TOK_OBSERVER;
		  if (strcmp(token,"old")==0) return TOK_OLD;
		  break;
		case 'p':
		  if (strcmp(token,"proc")==0) return TOK_PROC;
		  if (strcmp(token,"pre")==0) return TOK_PRE;
		  if (strcmp(token,"post")==0) return TOK_POST;
		  break;
		case 'r':
		  if (strcmp(token,"repeat")==0) return TOK_REPEAT;
		  if (strcmp(token,"result")==0) return TOK_RESULT;
		  if (strcmp(token,"require")==0) return TOK_REQUIRE;
		  break;
		case 's':
		  if (strcmp(token,"str")==0) return TOK_STR;
		  if (strcmp(token,"switch")==0) return TOK_SWITCH;
		  if (strcmp(token,"step")==0) return TOK_STEP;
		  break;
		case 't':
		  if (strcmp(token,"time")==0) return TOK_TIME;
		  if (strcmp(token,"tuple")==0) return TOK_TUPLE;
		  if (strcmp(token,"to")==0) return TOK_TO;
		  if (strcmp(token,"true")==0) return TOK_TRUE;
		  if (strcmp(token,"type")==0) return TOK_TYPE;
		  if (strcmp(token,"try")==0) return TOK_TRY;
		  break;
		case 'u':
		  if (strcmp(token,"until")==0) return TOK_UNTIL;
		  break;		  
		case 'v':
		  if (strcmp(token,"var")==0) return TOK_VAR;
		  break;		  
		case 'w':
		  if (strcmp(token,"while")==0) return TOK_WHILE;
		  if (strcmp(token,"with")==0) return TOK_WITH;
		  break;		  
		case 'F':
		  if (strcmp(token,"False")==0) return TOK_FALSE;
		  if (strcmp(token,"FALSE")==0) return TOK_FALSE;
		  break;		  
		case 'T':
		  if (strcmp(token,"True")==0) return TOK_TRUE;
		  if (strcmp(token,"TRUE")==0) return TOK_TRUE;
		  break;		  		  
  }
  return TOK_ID;
}

typedef struct scannerStatus{
	const char* line;
	int i,l;
	char nextch;
	char token[2000];
	int tokenCode;	
	// questi due valori devono essere gestiti al di fuori dell'inizializzazione della singola linea
	int longComment; 
	int lineNo;   
	int nPar;
} scannerStatus;

void initScanner(scannerStatus* s){
    s->longComment=0;
    s->lineNo=0;
    s->nPar=0;
}
	
void initScannerLine(scannerStatus* s, const char* line){
	s->line=line;
	s->i=0;
	s->l=strlen(line);
	s->nextch=line[0];
	s->token[0]='\0';
	s->tokenCode=0;
	// longComment, lineNo e nPar non si devono inzializzare, altrimenti si perdono al cambio linea!
}
	
void tokenize(scannerStatus* s){
	int i,l,tokl;
	int tokenCode;
	char nextch,token[2000];
	const char* line;
	line=s->line;
	i=s->i;
	l=s->l;
	nextch=s->nextch;
	token[0]='\0';
    tokl=0;
    tokenCode=0;
    if (s->longComment==1) { 
		// lo scanner è nello stato "commento lungo"
		const char* p=strstr(line,"*/");
		if (p!=NULL){
			// il commento lungo è terminato
			s->longComment=0;
			initScannerLine(s,p+2);
			tokenize(s);
			return;
		} else {
			// il commento lungo continua, si procede alla prossima linea
			s->token[0]='\0';
			s->tokenCode=-1;
			return;			
		}
	}
	// in nextch c'è il prossimo carattere da analizzare, elimina i caratteri blank 
	while(i<l && strchr(blankchars,nextch)!=NULL) nextch=line[++i];
	if (i>=l) { // ha letto tutta la stringa
		s->i=i;
		s->token[0]='\0';
		s->tokenCode=-1;
		return;
	} 
	// potrebbe esserci un commento lungo 
	if (nextch=='/' && line[i+1]=='*') {
		const char* p=strstr(line+i,"*/");
		if (p!=NULL) {
			// il commento lungo finisce nella stessa linea, si riparte come se fosse una nuova stringa
			initScannerLine(s,p+2);
			tokenize(s);
			return;
		} else {
			// il commento lungo continua nelle prossime linee
			s->longComment=1;
			s->token[0]='\0';
			s->tokenCode=-1;
			return;
		}
	}
	// in nextch c'è il prossimo carattere da analizzare non blank
	if (strchr(separators,nextch)!=NULL) { // è un separatore
		token[0]=nextch;
		token[1]='\0';
		//tokl=1; // segnalato da cppcheck
		if ((nextch=='>' || nextch=='<' || nextch==':') && line[i+1]=='=') {
			// caso dei token >=,<=,:=
			token[1]='=';
			token[2]='\0';
			tokenCode=(nextch==':'?TOK_ASSIGN:(nextch=='>'?TOK_GE:TOK_LE));
			i++;
			//tokl=2;
		} else if (nextch=='<' && line[i+1]=='>') {
			// caso del token <>
			token[1]='>';
			token[2]='\0';
			i++;
			//tokl=2; // segnalato da cppcheck
			tokenCode=TOK_NE;
		} else if (nextch=='-' && line[i+1]=='-') {
			// commento in linea --
  		    strcpy(s->token,line+i);
		    s->tokenCode=-1;
		    return;				    
		} else if (nextch=='/' && line[i+1]=='/') {
			// commento in linea --
  		    strcpy(s->token,line+i);
		    s->tokenCode=-1;
		    return;				    
		} else {
			tokenCode=getTokenCodeFromString(token);
			if (nextch=='(' || nextch=='[' || nextch=='{') s->nPar++;
			if (nextch==')' || nextch==']' || nextch=='}') s->nPar--;
		}
		nextch=line[++i];
	} else if (nextch=='"' || nextch=='\'') { // è una stringa
		char stringterm=nextch;
		i++;
		while (i<l && line[i]!=stringterm){  // la cattura fino al corrispondete terminatore
			token[tokl++]=line[i++];
		}
		token[tokl]='\0';
		if (i<l) { // non ha sforato, la stringa è stata chiusa correttamente
			nextch=line[++i];
		} else {
			// ERR: stringa non terminata correttamente sulla stessa linea
			diag("stringa non terminata\n");
		}
		tokenCode=TOK_STR_CONST;
	} else if (strchr(numberchars,nextch)!=NULL) { // è un numero			
		while (strchr(numberchars,nextch)!=NULL || nextch=='_'){ // legge tutte le cifre che seguono, si possono usare i _ per separare le cifre
			if (nextch!='_') token[tokl++]=nextch; // se non è un _ lo mette nel testo del numero
			nextch=line[++i];
		}
		tokenCode=TOK_INT_CONST;
		// finito il numero, può essere un float
		if (nextch=='.') {
			nextch=line[++i];
			token[tokl++]='.';
			while (strchr(numberchars,nextch)!=NULL || nextch=='_'){ // legge le cifre dopo il punto decimale
				if (nextch!='_') token[tokl++]=nextch;
				nextch=line[++i];
			}
   			tokenCode=TOK_FLOAT_CONST;
		}
		// qui bisogna controllare che nextchar sia un separatore
		// ERR: il numero non è terminato in un separatore
		if (strchr(separators,nextch)==NULL)
			diag("numero terminato male\n");
			//  
		token[tokl]='\0';
	} else {
		while (strchr(separators,nextch)==NULL){ // non + un separatore
			token[tokl++]=nextch; // aggiunge il carattere al token
			i++;                  // sposta in avanti il punto di analisi della stringa
			nextch=line[i];       // mette il prossimo carattere da analizzare in nextch
		}
		// in token c'è quanto ha trovato, in nexttoken c'è il prossimo carattere da analizzare
		token[tokl]='\0'; // termina il token
		tokenCode=getTokenCodeFromString(token);
	}
	//printf("%s %d\n",token,tokenCode);
	s->i=i;
	s->nextch=nextch;
	strcpy(s->token,token);
	s->tokenCode=tokenCode;
}

int GetNextToken(scannerStatus* s,int* tokenCode,char** token){
	tokenize(s);
	*tokenCode=s->tokenCode;
	*token=s->token;
	return s->tokenCode!=-1;
}

#include <stdlib.h>

void dispose(char** value){
	free(*value);
	*value=0;
}

typedef struct parserState {
	int labelCnt;
	int errors;
	int lineNo;
} parserState;

void initParserState(parserState* s){
	s->labelCnt=0;
	s->errors=0;
	s->lineNo=0;
}

typedef struct ifelseif {
	int start;
	int last;
} ifelseif;

typedef struct forloop {
	int start;          // label del test
	int last;           // label di uscita
	char* name;         // variabile di ciclo, liberata alla fine del for
} forloop;

typedef struct foreachloop {
	int start;          // label del test
	int last;           // label di uscita
	char* name;         // variabile del valore
	char* key;          // variabile della chiave, NULL se assente
	int mark;           // comprehension: posizione nel buffer della fine dell'espressione
	int n;              // numero di collezioni: 1, o 2 per la visita parallela (dict comprehension)
} foreachloop;
		
#include "cli.c"
#include "pcodes.c"
#include "emit.c"

// testa del ciclo for each, comune allo statement e alle comprehension: la collezione (o le
// due della visita parallela, n==2) e' gia' sullo stack, la trasforma in iteratore, registra le variabili (liberandone i nomi),
// assegna le due label e apre il ciclo
void foreachTesta(foreachloop* n,parserState* sState){
	if (n->n==2) emit_i(P_FOREACH_INIT,2); else emit(P_FOREACH_INIT);
	emit_s(P_FOREACH_VAR,n->name);
	if (n->key) emit_s(P_FOREACH_KEY,n->key);
	dispose(&n->name);
	if (n->key) dispose(&n->key);
	n->start=sState->labelCnt++;
	n->last=sState->labelCnt++;
	emit_i(P_LABEL,n->start);
	emit_i(P_FOREACH_NEXT,n->last);
}

#include "cpl2.c"

int stackLevel(yyParser* p){
    yyStackEntry* t=p->yytos;            
    const yyStackEntry* s=p->yystack; 
    //printf("yytos: %d %d\n",t->stateno,t->major);      
    //printf("yyend: %d %d\n",s->stateno,s->major);      
    int n=0;while(t>s) {t--;n++;}     
    //printf("stk: %d \n",n);
	return n;
}

int parse(FILE* f){
	// parte mia
	initpcodetxt();
	emit_asm=0;
	emit_ver("0.0.1");
    char line[2000];
    // se il file e' vuoto la prima fgets fallisce: senza questo controllo line
    // resterebbe non inizializzata e verrebbe trattata come una stringa
    if (fgets(line,2000,f)==NULL) line[0]='\0';
    scannerStatus s;
    initScanner(&s);
    initScannerLine(&s,line);
    //int stkl;
    // parte lemon
    char* sToken=0;
    int hTokenId;
    yyParser *pParser;
    parserState sState;
    pParser = ParseAlloc(malloc);
    initParserState(&sState);
    //ParseTrace(stdout,"TRACE:");
    while (!feof(f) && strcmp(line,"eof\n")!=0 && strcmp(line,"EOF\n")!=0) {
	  // setta il numero di riga per riportare la posizione degli errori
	  s.lineNo++;
	  sState.lineNo++;
	  emit_line(s.lineNo,line);
	  // inizializza la linea
      initScannerLine(&s,line);		
      while (GetNextToken(&s, &hTokenId, &sToken)){
        //printf("token = %s\n",sToken);
		// i token che devono essere stampati sul p-code vanno salvati in memoria, possono essere processati dopo il prossimo token
        if (hTokenId==TOK_INT_CONST || hTokenId==TOK_STR_CONST || hTokenId==TOK_FLOAT_CONST|| hTokenId==TOK_ID) {
		  char* sss=malloc(strlen(sToken)+1);strcpy(sss,sToken);sToken=sss;
	    }
	    Parse(pParser, hTokenId, sToken, &sState);
      }
      //terminata la linea, deve passare alla prossima
      if (s.nPar==0) // il fine linea è sospeso quando le parentesi sono sbilanciate
        Parse(pParser,TOK_EOL,"",&sState);
      //stkl=stackLevel(pParser);
      //if (stkl>3) printf("> ");
      fgets(line,2000,f);
    }
    Parse(pParser, 0, sToken, &sState);
    //stkl=stackLevel(pParser);
    ParseFree(pParser, free );
    // il p-code e' stato accumulato in memoria (vedi emit.c): ora va su emit_out
    emit_flush();
    return sState.errors;
}

// restituisce l'ultimo '.' presente nel solo nome di base del percorso, NULL se
// il nome di base non ne contiene: la directory non viene considerata, quindi
// "dir.d/x" non ha estensione. Usata sia da pcodePath() sia dalla risoluzione del
// sorgente in command line, cosi' la regola sta in un posto solo.
const char* baseDot(const char* path){
	const char* slash=strrchr(path,'/');
	const char* base=slash!=NULL?slash+1:path;  // nome di base
	return strrchr(base,'.');
}

// ricava il percorso del file p-code da quello del sorgente: si toglie
// l'eventuale estensione dal solo nome di base e si accoda ".pcd", quindi la
// directory del sorgente resta conservata (vedi D2 e la specifica sulla derivazione).
// fib.cpl -> fib.pcd, test/a.cpl -> test/a.pcd, dir.d/x -> dir.d/x.pcd
void pcodePath(const char* src, char* out){
	const char* dot=baseDot(src);             // ultima estensione, solo nel nome di base
	size_t stem=dot!=NULL?(size_t)(dot-src):strlen(src);
	memcpy(out,src,stem);
	strcpy(out+stem,".pcd");
}

// apre il sorgente indicato in command line. Se il percorso digitato non si
// apre perche' non esiste e il nome di base non ha estensione, si prova anche
// accodando ".cpl". *resolved riceve il percorso effettivamente aperto, che
// resta valido almeno fino alla chiamata successiva: in modalita' file e' la
// sorgente di srcFile e di pcodePath(). Il percorso digitato ha la precedenza:
// se si apre, il ripiego non viene neppure provato, e se non si apre per un
// motivo diverso dall'assenza (permessi) il ripiego non viene provato affatto,
// perche' tradurre un file diverso da quello nominato e' peggio che fallire.
FILE* openSource(const char* arg, const char** resolved){
	FILE* fIn=fopen(arg,"r");
	if (fIn!=NULL) {
		*resolved=arg;   // il percorso digitato vince
		return fIn;
	}
	int firstErrno=errno;   // salvato prima del secondo tentativo, altrimenti
	                        // strerror(errno) riporterebbe l'errore sbagliato
	// il ripiego vale solo per un nome senza estensione, e il punto si cerca
	// solo nel nome di base: "dir.d/x.cpl" e' il candidato di "dir.d/x"
	if (firstErrno!=ENOENT || baseDot(arg)!=NULL) {
		errno=firstErrno;
		return NULL;
	}
	// il buffer e' statico perche' *resolved viene usato dopo la ritornata,
	// come srcFile e come argomento di pcodePath(): sta in pila morirebbe qui
	static char cand[2000];   // stessa dimensione di pcdPath e line
	snprintf(cand,sizeof(cand),"%s.cpl",arg);
	fIn=fopen(cand,"r");
	if (fIn!=NULL) {
		*resolved=cand;   // aperto il ripiego con ".cpl"
		return fIn;
	}
	errno=firstErrno;   // se anche il ripiego e' irraggiungibile il motivo che
	                    // l'utente capisce e' che il nome digitato non esiste
	return NULL;
}

// D6: la tabella delle opzioni del compilatore parte vuota, e non e' una difesa.
// Il ciclo condiviso gira lo stesso e quindi un argomento che comincia con "-" e
// non e' il solo "-" non arriva piu' fra i sorgenti. La riga inerte serve solo a
// dare al ciclo una tabella su cui camminare: quando arrivera' il primo flag, il
// posto in cui scriverlo e' gia' qui.
static const cliOpzione tabOpzioni[1]={{NULL,0,NULL,NULL,CLI_OPZIONE,NULL}};

// la diagnostica dell'errore di scansione dice quale opzione e' stata rifiutata e
// perche': senza questo la tabella vuota renderebbe "-x" e "--out" indistinguibili
// fra loro.
static void diagOpzione(cliScansione* sc){
	switch (sc->errore) {
		case CLI_ERRORE_VALORE:
			diag("l'opzione %s richiede un valore\n",sc->erroreNome);
			break;
		case CLI_ERRORE_NONVALORE:
			diag("l'opzione %s non accetta un valore: %s\n",sc->erroreNome,sc->erroreValore);
			break;
		default:
			diag("opzione sconosciuta: %s\n",sc->erroreNome);
			break;
	}
}

int main(int argc, char** argv){
	// con i file in command line ogni sorgente produce il proprio .pcd e su
	// stdout non viene scritto nulla; gli errori si sommano su tutti i file (D7)
	int errors=0;
	// la scansione separa le opzioni dai sorgenti: i sorgenti sono cioe' quello
	// che resta e vengono tradotti nell'ordine in cui sono stati indicati. Il
	// modulo non alloca, quindi il vettore dei sorgenti e' qui
	int maxArg=argc>1?argc:1;   // argc<2 non ha argomenti: una cella basta lo stesso
	const char** sorgenti=(const char**)malloc((size_t)maxArg*sizeof(char*));
	cliScansione sc;
	int nSorgenti=0;
	int i;
	if (sorgenti==NULL) {
		fprintf(stderr,"memoria insufficiente per gli argomenti\n");
		return 1;
	}
	cliInizia(&sc,argc,argv);
	nSorgenti=cliScansiona(&sc,tabOpzioni,1,sorgenti,maxArg);
	if (sc.errore!=CLI_ERRORE_NESSUNO) {
		// un'opzione sconosciuta riguarda tutta l'invocazione, quindi nessun
		// sorgente viene tradotto: e' diverso da un sorgente che non si puo'
		// aprire, che non ferma gli altri. srcFile e' ancora NULL, quindi la
		// diagnostica esce senza prefisso come nella modalita' standard input
		diagOpzione(&sc);
		free(sorgenti);
		return 1;
	}
	if (nSorgenti==0) {
		// nessun sorgente in command line: percorso di sempre, il p-code va su
		// stdout e le diagnostiche escono senza prefisso (vedi D5)
		srcFile=NULL;
		emit_out=stdout;
		errors=parse(stdin);
		free(sorgenti);
		return errors;
	}
	for (i=0;i<nSorgenti;i++) {
		const char* arg=sorgenti[i];
		const char* resolved=NULL;
		FILE* fIn;
		FILE* fOut;
		char pcdPath[2000];
		// "-" e' il modo consueto di dire "leggere dallo standard input": si
		// riusa lo stesso ramo di quando i sorgenti non sono indicati (D5)
		if (strcmp(arg,"-")==0) {
			srcFile=NULL;
			emit_out=stdout;
			errors+=parse(stdin);
			emit_out=stdout;
			continue;
		}
		// srcFile non deve mai essere quello del file precedente: nel ramo di
		// errore non esiste un percorso risolto e il prefisso delle diagnostiche
		// resta quello digitato, come prima della risoluzione
		srcFile=arg;
		fIn=openSource(arg,&resolved);
		if (fIn==NULL) {
			// openSource tenta anche il ripiego solo se errno e' ENOENT e il nome
			// di base non ha estensione, quindi la stessa condizione dice se i
			// percorsi tentati sono due o uno solo: nel ramo dei permessi
			// fib.cpl non e' mai stato provato e non va citato
			if (errno==ENOENT && baseDot(arg)==NULL)
				diag("impossibile aprire il sorgente: %s, %s.cpl: %s\n",arg,arg,strerror(errno));
			else
				diag("impossibile aprire il sorgente: %s: %s\n",arg,strerror(errno));
			errors++;
			continue;
		}
		// dal percorso risolto derivano diagnostiche e nome del .pcd: per "fib"
		// risolto in "fib.cpl" il prefisso e' fib.cpl e il p-code resta fib.pcd
		srcFile=resolved;
		pcodePath(resolved,pcdPath);
		fOut=fopen(pcdPath,"w");
		if (fOut==NULL) {
			diag("impossibile aprire il p-code: %s\n",strerror(errno));
			fclose(fIn);
			errors++;
			continue;
		}
		emit_out=fOut;
		errors+=parse(fIn);
		emit_out=stdout;
		fclose(fIn);
		fclose(fOut);
	}
	free(sorgenti);
	return errors;
}
