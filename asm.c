#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "pcodes.h"

#include "pcodes.c"
#include "emit.c"
#include "cli.c"

int FindPCode(const char* l){
	int pcd=0,i;
	for(i=1;i<=MAX_PCODE;i++){
	  if (pcd==0 && strcmp(l,pcodetxt[i])==0)
	    pcd=i;	  
	}
	return pcd;
}

int cpl2asm(FILE* f){
  char line[2000];
  fgets(line,2000,f);
  // dalla prima linea capisce il verso della traduzione
  if (strncmp(line,"#cpl2",5)==0) {
	  emit_asm=1;
	  printf("CPL2 PCODE VER:%s",line+5);
  } else if (strncmp(line,"CPL2 PCODE VER:",15)==0){	  
	  emit_asm=0;
	  printf("#cpl2%s",line+15);
  } else {
	  return 0;
  } 
  fgets(line,2000,f);
  while(!feof(f)){
	  if (strncmp(line,"da fare ",8)==0){
		printf("%s",line);  
	  } else if (emit_asm==1){
		  // stampa il testo del pcode e il resto della linea
		  unsigned char c=line[0];
		  printf("%s %s",pcodetxt[c-31],line+1);
	  } else {
		  // trova il primo spazio
		char* p=strchr(line,' '); // posizione del primo spazio		  
		if (p!=NULL) {
		  p[0]='\0'; // separa la linea nei due componenti
		  p++;  
		} else {
		  p="\n"; // pcode senza parte secondaria  
		}
		// trova il codice del pcode
		int pcd=FindPCode(line);
		//if (pcd==P_LINE) printf("LINE %s\n",p);
		printf("%c%s",(unsigned char)(pcd+31),p);
	  }
	  fgets(line,2000,f);
  }
  return emit_asm+1;
}

// come il compilatore, anche asm non dichiara opzioni: la riga inerte serve solo
// a dare al ciclo condiviso una tabella su cui camminare, e fa si' che un
// argomento che comincia con "-" non venga scambiato per un percorso
static const cliOpzione tabOpzioni[1]={{NULL,0,NULL,NULL,CLI_OPZIONE,NULL}};

int main(int argc,char** argv) {
  const char* pos[1]; // al massimo un percorso: cliScansiona conta i troppi
  cliScansione sc;
  FILE* f;
  int rc;
  emit_out=stdout; // emit.c e' incluso anche qui, l'emissione resta su stdout
  initpcodetxt();	
  cliInizia(&sc,argc,argv);
  cliScansiona(&sc,tabOpzioni,1,pos,1);
  if (sc.errore!=CLI_ERRORE_NESSUNO) {
    fprintf(stderr,"opzione sconosciuta: %s\n",sc.erroreNome);
    return 1;
  }
  if (sc.nPosizionali>1) {
    // pos[] tiene solo il primo percorso: gli altri sono nominati da argv, che
    // e' l'ordine in cui sono stati digitati
    fprintf(stderr,"un file alla volta: non posso leggere %s e %s insieme\n",pos[0],argv[argc-1]);
    return 1;
  }
  // nessun percorso, o il solo "-": il percorso di sempre, lo standard input
  if (sc.nPosizionali==0||strcmp(pos[0],"-")==0)
    return cpl2asm(stdin);
  f=fopen(pos[0],"r");
  // Come pcodePathFor() dell'interprete (regola D2) il punto si cerca solo nel
  // nome di base, mai nella directory. A differenza di la' qui non si deriva il
  // percorso ma si prova prima il nome com'e' scritto, perche' asm accetta sia
  // un .pcd sia un listing testuale: il suffisso serve solo da seconda prova
  // quando il file non esiste. Se entrambe le prove falliscono l'errore nomina
  // il percorso digitato.
  if (f==NULL && errno==ENOENT) {
    const char* slash=strrchr(pos[0],'/');
    const char* base=slash==NULL?pos[0]:slash+1;
    if (strchr(base,'.')==NULL) {
      char fallback[2000];
      snprintf(fallback,sizeof(fallback),"%s.pcd",pos[0]);
      f=fopen(fallback,"r");
    }
  }
  if (f==NULL) {
    fprintf(stderr,"impossibile aprire il file: %s: %s\n",pos[0],strerror(errno));
    return 1;
  }
  rc=cpl2asm(f);
  fclose(f);
  return rc;
}
