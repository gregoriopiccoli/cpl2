
int emit_asm=0;
		
void emit(int pcode){
	if (emit_asm)
	  printf("%s\n",pcodetxt[pcode]);
	else
	  printf("%c\n",(char)(pcode+31));  
}

void emit_i(int pcode, int v){
	if (emit_asm)
	  printf("%s %d\n",pcodetxt[pcode],v);
	else
	  printf("%c%d\n",(char)(pcode+31),v);  
}

void emit_s(int pcode, char* v){
	if (emit_asm)
	  printf("%s %s\n",pcodetxt[pcode],v);
	else
	  printf("%c%s\n",(char)(pcode+31),v);  
}

void emit_line(int v, char* line){
	if (emit_asm)
	  printf("%s %d --- %s",pcodetxt[P_LINE],v,line);
	else
	  printf("%c%d\n",(char)(P_LINE+31),v);  
}

void emit_ver(char* v) {
	if (emit_asm)
	  printf("CPL2 PCODE VER:%s\n",v);
	else
	  printf("#cpl2%s\n",v);  
}
		
