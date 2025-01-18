#include <stdio.h>
#include <string.h>
#include "pcodes.h"

#include "pcodes.c"
#include "emit.c"

int FindPCode(char* l){
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
		  if (strncmp(line,"da fare ",8)==0) {
			printf("%s",line);  
		  } else {
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
		    printf("%c%s",(char)(pcd+31),p);
		}
	  }
	  fgets(line,2000,f);
  }
  return emit_asm+1;
}

int main() {
  initpcodetxt();	
  return cpl2asm(stdin);  
}
