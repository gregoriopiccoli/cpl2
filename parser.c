#include <stdio.h>
#include <string.h>

#include "cpl2.h"

const char* separators="+-*/%()[]{}!|<>= \t\r\n.:;,@?";
const char* blankchars=" \t\r\n";
const char* numberchars="0123456789";

int getTokenCodeFromString(char* token){
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
		  break;
		case 'b':
		  if (strcmp(token,"bool")==0) return TOK_BOOL;
		  break;  
		case 'c':
		  if (strcmp(token,"class")==0) return TOK_CLASS;
		  if (strcmp(token,"case")==0) return TOK_CASE;
		  if (strcmp(token,"catch")==0) return TOK_CATCH;
		  break;  
		case 'd':
		  if (strcmp(token,"dict")==0) return TOK_DICT;
		  if (strcmp(token,"date")==0) return TOK_DATE;
		  if (strcmp(token,"datetime")==0) return TOK_DATETIME;
  		  break;  
		case 'e':
		  if (strcmp(token,"end")==0) return TOK_END;
		  if (strcmp(token,"else")==0) return TOK_ELSE;
		  if (strcmp(token,"each")==0) return TOK_EACH;
		  if (strcmp(token,"enum")==0) return TOK_ENUM;
		  if (strcmp(token,"elseif")==0) return TOK_ELSEIF;
		  if (strcmp(token,"event")==0) return TOK_EVENT;
		 break;  
		case 'f':
		  if (strcmp(token,"func")==0) return TOK_FUNC;
		  if (strcmp(token,"for")==0) return TOK_FOR;
		  if (strcmp(token,"from")==0) return TOK_FROM;
		 break;  
		case 'i':
		  if (strcmp(token,"int")==0) return TOK_INT;  
		  if (strcmp(token,"if")==0) return TOK_IF;  
		  if (strcmp(token,"iif")==0) return TOK_IIF;  
		  if (strcmp(token,"import")==0) return TOK_IMPORT;  
		  if (strcmp(token,"in")==0) return TOK_IN;  
		  if (strcmp(token,"is")==0) return TOK_IS;  
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
		  break;
		case 'p':
		  if (strcmp(token,"proc")==0) return TOK_PROC;
		  break;
		case 'r':
		  if (strcmp(token,"repeat")==0) return TOK_REPEAT;
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
  }
  return TOK_ID;
}

typedef struct scannerStatus{
	const char* line;
	int i,l;
	char nextch;
	char token[200];
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
	char nextch,token[200];
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
		char* p=strstr(line,"*/");
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
		char* p=strstr(line+i,"*/");
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
		tokl=1;
		if ((nextch=='>' || nextch=='<' || nextch==':') && line[i+1]=='=') {
			// caso dei token >=,<=,:=
			token[1]='=';
			token[2]='\0';
			tokenCode=(nextch==':'?TOK_ASSIGN:(nextch=='>'?TOK_GE:TOK_LE));
			i++;
			tokl=2;
		} else if (nextch=='<' && line[i+1]=='>') {
			// caso del token <>
			token[1]='>';
			token[2]='\0';
			i++;
			tokl=2;
			tokenCode=TOK_NE;
		} else if (nextch=='-' && line[i+1]=='-') {
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
			printf("stringa non terminata\n");
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
			printf("numero terminato male\n");
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
		
#include "cpl2.c"

int main(){
	// parte mia
    char line[200];
    fgets(line,200,stdin);
    scannerStatus s;
    initScanner(&s);
    initScannerLine(&s,line);
    // parte lemon
    char* sToken;
    int hTokenId;
    void *pParser;
    parserState sState;
    pParser = ParseAlloc(malloc);
    initParserState(&sState);
    //ParseTrace(stdout,"TRACE:");
    while (!feof(stdin) && strcmp(line,"eof\n")!=0 && strcmp(line,"EOF\n")!=0) {
	  // setta il numero di riga per riportare la posizione degli errori
	  s.lineNo++;
	  sState.lineNo++;
	  printf("LINE %d # %s",s.lineNo,line);
	  // inizializza la linea
      initScannerLine(&s,line);		
      while( GetNextToken(&s, &hTokenId, &sToken) ){
        //printf("token = %s\n",sToken);
		// i token che devono essere stampati sul p-code vanno salvati in memoria, possono essere processati dopo il prossimo token
        if(hTokenId==TOK_INT_CONST || hTokenId==TOK_STR_CONST || hTokenId==TOK_FLOAT_CONST|| hTokenId==TOK_ID) {
		  char* sss=malloc(strlen(sToken)+1);strcpy(sss,sToken);sToken=sss;
	    }
	    Parse(pParser, hTokenId, sToken, &sState);
      }
      //printf("token = %s\n",sToken);
      if (s.nPar==0) // il fine linea è sospeso quando le parentesi sono sbilanciate
	    Parse(pParser,TOK_EOL,"",&sState);
      fgets(line,200,stdin);
    }
    Parse(pParser, 0, sToken, &sState);
    ParseFree(pParser, free );
}
