#include <stdio.h>
#include <string.h>
#include "pcodes.h"

#include "pcodes.c"
#include "emit.c"

int cpl2asm(FILE* f){
  char line[2000];
  fgets(line,2000,f);
  // dalla prima linea capisce il verso della traduzione
  if (strncmp(line,"#cpl2",5)==0) {
	  emit_asm=1;
  } else {
	  emit_asm=0;
  } 
  fgets(line,2000,f);
  while(!feof(f)){
	  if (strncmp(line,"da fare ",8)==0){
		printf("%s",line);  
	  } else if (emit_asm==1){
		  char c=line[0];
		  printf("%s %s",pcodetxt[c-31],line+1);
	  } else {
	  }
	  fgets(line,2000,f);
  }
}

int main() {
  initpcodetxt();	
  return cpl2asm(stdin);  
}
