#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

int emit_asm=0;

// destinazione di tutte le funzioni di emissione: stdout in modalita' stdin, il file .pcd
// aperto in modalita' command line. Non puo' essere inizializzato qui con stdout: in C non
// e' un'espressione costante e l'inizializzatore a scope di file non compilerebbe. Ogni
// main() che include emit.c lo assegna per primo (vedi D1 in openspec).
FILE* emit_out;

// il p-code non va subito su emit_out ma si accumula in questo buffer, scritto da emit_flush()
// alla fine di parse(): cosi' un blocco gia' emesso si puo' spostare (emit_rotate), come serve
// alle comprehension, dove l'espressione arriva prima della collezione ma va eseguita nel ciclo.
static char* emit_buf=NULL;
static int emit_len=0,emit_cap=0;

static void emit_printf(const char* fmt, ...){
	va_list ap;
	int n;
	va_start(ap,fmt);
	n=vsnprintf(NULL,0,fmt,ap);
	va_end(ap);
	if (emit_len+n+1>emit_cap){
	  emit_cap=(emit_len+n+1)*2;
	  emit_buf=(char*)realloc(emit_buf,emit_cap);
	}
	va_start(ap,fmt);
	vsnprintf(emit_buf+emit_len,n+1,fmt,ap);
	va_end(ap);
	emit_len+=n;
}

// posizione del prossimo p-code nel buffer
int emit_pos(void){
	return emit_len;
}

// porta il blocco [from,mid) in fondo al buffer, dopo [mid,fine); la lunghezza non cambia,
// quindi le posizioni prese fuori da [from,fine) restano valide
void emit_rotate(int from, int mid){
	int n=mid-from;
	char* tmp;
	if (n<=0 || mid>=emit_len) return;
	tmp=(char*)malloc(n);
	memcpy(tmp,emit_buf+from,n);
	memmove(emit_buf+from,emit_buf+mid,emit_len-mid);
	memcpy(emit_buf+emit_len-n,tmp,n);
	free(tmp);
}

// scrive su emit_out quanto accumulato e svuota il buffer
void emit_flush(void){
	if (emit_len>0) fwrite(emit_buf,1,emit_len,emit_out);
	emit_len=0;
}

void emit(int pcode){
	if (emit_asm)
	  emit_printf("%s\n",pcodetxt[pcode]);
	else
	  emit_printf("%c\n",(char)(pcode+31));
}

void emit_i(int pcode, int v){
	if (emit_asm)
	  emit_printf("%s %d\n",pcodetxt[pcode],v);
	else
	  emit_printf("%c%d\n",(char)(pcode+31),v);
}

void emit_s(int pcode, const char* v){
	if (emit_asm)
	  emit_printf("%s %s\n",pcodetxt[pcode],v);
	else
	  emit_printf("%c%s\n",(char)(pcode+31),v);
}

void emit_line(int v, const char* line){
	if (emit_asm)
	  emit_printf("%s %d --- %s",pcodetxt[P_LINE],v,line);
	else
	  emit_printf("%c%d\n",(char)(P_LINE+31),v);
}

void emit_ver(const char* v) {
	if (emit_asm)
	  emit_printf("CPL2 PCODE VER:%s\n",v);
	else
	  emit_printf("#cpl2%s\n",v);
}
