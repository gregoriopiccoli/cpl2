
int emit_asm=0;

// destinazione di tutte le funzioni di emissione: stdout in modalita' stdin, il file .pcd
// aperto in modalita' command line. Non puo' essere inizializzato qui con stdout: in C non
// e' un'espressione costante e l'inizializzatore a scope di file non compilerebbe. Ogni
// main() che include emit.c lo assegna per primo (vedi D1 in openspec).
FILE* emit_out;
		
void emit(int pcode){
	if (emit_asm)
	  fprintf(emit_out,"%s\n",pcodetxt[pcode]);
	else
	  fprintf(emit_out,"%c\n",(char)(pcode+31));  
}

void emit_i(int pcode, int v){
	if (emit_asm)
	  fprintf(emit_out,"%s %d\n",pcodetxt[pcode],v);
	else
	  fprintf(emit_out,"%c%d\n",(char)(pcode+31),v);  
}

void emit_s(int pcode, const char* v){
	if (emit_asm)
	  fprintf(emit_out,"%s %s\n",pcodetxt[pcode],v);
	else
	  fprintf(emit_out,"%c%s\n",(char)(pcode+31),v);  
}

void emit_line(int v, const char* line){
	if (emit_asm)
	  fprintf(emit_out,"%s %d --- %s",pcodetxt[P_LINE],v,line);
	else
	  fprintf(emit_out,"%c%d\n",(char)(P_LINE+31),v);  
}

void emit_ver(const char* v) {
	if (emit_asm)
	  fprintf(emit_out,"CPL2 PCODE VER:%s\n",v);
	else
	  fprintf(emit_out,"#cpl2%s\n",v);  
}
		
