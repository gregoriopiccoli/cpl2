// Runner dei test del progetto cpl2 (decisione D1 del design).
//
// Un test e' un file .cpl della directory dei test. Se ha accanto un .out con
// lo stesso nome di base e' un test completo (D1): il sorgente si compila con
// ./compiler, il p-code prodotto si esegue con ./interpreter catturando il solo
// standard output, e quel contenuto si confronta byte per byte con il .out
// (D3). Se il .out manca, e' un test di sola compilazione: si compila con
// ./compiler e basta, mai eseguito, ma entra lo stesso nel rapporto. Un .out
// senza .cpl e' un'incoerenza nominata nel rapporto che conta come fallimento.
//
// Come parser.c e interpreter.cpp, anche qui la command line la legge la forma
// condivisa di cli.c con una tabella delle opzioni vuota (D8): l'unico
// argomento e' la directory dei test, con "test" come default.
//
// I .pcd e gli output catturati non finiscono mai nella directory dei test: il
// sorgente viene copiato in una directory temporanea (mkdtemp, schema in
// TMPDIR) che alla fine del giro viene rimossa con tutto il suo contenuto,
// anche in caso di fallimenti (D5). La directory dei test non viene toccata.
//
// Il rapporto finisce interamente su stderr (D6), perche' il codice di uscita —
// 0 se e solo se nessun test e' fallito — deve restare leggibile da uno script
// anche con lo standard output rediretto su /dev/null.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "cli.c"

// dimensione unica dei percorsi costruiti: il nome di un test sta nel limite di
// NAME_MAX e la directory di appoggio e' un percorso breve dentro TMPDIR
#define PERCORSO 4096

// D6: tabella delle opzioni di runtest, vuota come quella di parser.c: oggi
// l'unico argomento e' la directory dei test. Il giorno in cui servira' un flag,
// la riga in tabella compare anche nell'uso senza scrivere due volte l'elenco.
static const cliOpzione tabOpzioni[1]={{NULL,0,NULL,NULL,CLI_OPZIONE,NULL}};

// diagnostica di una scansione fallita, come in parser.c: senza questa la
// tabella vuota renderebbe un'opzione sconosciuta irriconoscibile nel messaggio
static void diagOpzione(cliScansione* sc){
	switch (sc->errore){
		case CLI_ERRORE_VALORE:
			fprintf(stderr,"l'opzione %s richiede un valore\n",sc->erroreNome);
			break;
		case CLI_ERRORE_NONVALORE:
			fprintf(stderr,"l'opzione %s non accetta un valore: %s\n",sc->erroreNome,sc->erroreValore);
			break;
		default:
			fprintf(stderr,"opzione sconosciuta: %s\n",sc->erroreNome);
			break;
	}
}

// struttura del rapporto del giro: due contatori decidono il codice di uscita,
// e ogni test fallito viene nominato nel momento in cui fallisce (D6)
typedef struct {
	int superati;
	int falliti;
} rapporto;

// una riga del rapporto per un test fallito: il nome va su stderr insieme al
// motivo, e il conteggio sale qui, cosi' il codice di uscita segue da solo
static void fallito(rapporto* rap,const char* nome,const char* motivo){
	fprintf(stderr,"FALLITO %s: %s\n",nome,motivo);
	rap->falliti++;
}

// il percorso esiste ed e' un file regolare
static int eFileRegolare(const char* p){
	struct stat st;
	return stat(p,&st)==0&&S_ISREG(st.st_mode);
}

// la stringa finisce con il suffisso indicato
static int finisceCon(const char* nome,const char* suffisso){
	size_t n=strlen(nome),s=strlen(suffisso);
	return n>s&&strcmp(nome+n-s,suffisso)==0;
}

// ------------------------------------------------------------------
// raccolta dei test (requisito "Selezione dei test in una directory")
// ------------------------------------------------------------------

// elenco di stringhe che cresce a mano: la raccolta non sa quante coppie ci
// sono finche' non ha scansionato la directory. Si copia il testo, perche' i
// nomi degli elenchi vivono fino alla fine del giro
static int aggiungi(char*** elenco,int* n,const char* s){
	char* c=strdup(s);
	char** p;
	if (c==NULL) return -1;
	p=(char**)realloc(*elenco,(size_t)(*n+1)*sizeof(char*));
	if (p==NULL){free(c);return -1;}
	p[*n]=c;
	*elenco=p;
	(*n)++;
	return 0;
}

static void liberaElenco(char** e,int n){
	int k;
	for (k=0;k<n;k++) free(e[k]);
	free(e);
}

// l'ordine dei nomi e' quello dei byte: due giri consecutivi devono riportare
// lo stesso esito, e l'ordine di readdir non e' un contratto (5.3)
static int confrontaNomi(const void* a,const void* b){
	return strcmp(*(const char* const*)a,*(const char* const*)b);
}

typedef struct {
	char** test;    // nomi base dei .cpl che hanno il .out accanto, ordinati
	int nTest;
	char** orfani;  // nomi dei .out che non hanno il .cpl accanto, ordinati
	int nOrfani;
	char** soloCompilazione;  // nomi base dei .cpl senza .out accanto, ordinati
	int nSoloCompilazione;
} raccolta;

// raccoglie i file della directory indicata in tre elenchi: un .cpl con il .out
// accanto e' un test completo; un .cpl senza .out e' un test di sola
// compilazione, compilato ma mai eseguito e comunque contato; un .out senza .cpl
// e' un'incoerenza che conta come fallimento (requisito "Output atteso senza
// sorgente"). Si guarda solo dentro la directory: nessuna ricorsione nelle
// sottodirectory
static int raccogli(const char* dirTest,raccolta* rc){
	DIR* d=opendir(dirTest);
	struct dirent* e;
	rc->test=NULL;rc->nTest=0;rc->orfani=NULL;rc->nOrfani=0;
	rc->soloCompilazione=NULL;rc->nSoloCompilazione=0;
	if (d==NULL){
		fprintf(stderr,"runtest: impossibile aprire la directory %s: %s\n",dirTest,strerror(errno));
		return -1;
	}
	while ((e=readdir(d))!=NULL){
		char percorso[PERCORSO];
		struct stat st;
		if (strcmp(e->d_name,".")==0||strcmp(e->d_name,"..")==0) continue;
		snprintf(percorso,sizeof percorso,"%s/%s",dirTest,e->d_name);
		// non e' un file regolare (directory comprese): fuori dalla raccolta, e
		// nessuna sottodirectory viene guardata
		if (stat(percorso,&st)!=0||!S_ISREG(st.st_mode)) continue;
		if (finisceCon(e->d_name,".cpl")){
			// il .cpl con il .out accanto e' un test completo, altrimenti
			// e' un test di sola compilazione
			size_t baseLen=strlen(e->d_name)-4;
			char atteso[PERCORSO];
			snprintf(atteso,sizeof atteso,"%s/%.*s.out",dirTest,(int)baseLen,e->d_name);
			e->d_name[baseLen]='\0';
			if (eFileRegolare(atteso)){
				if (aggiungi(&rc->test,&rc->nTest,e->d_name)!=0) goto memoria;
			}else{
				if (aggiungi(&rc->soloCompilazione,&rc->nSoloCompilazione,e->d_name)!=0) goto memoria;
			}
		}else if (finisceCon(e->d_name,".out")){
			// il .out senza .cpl corrispondente e' un'incoerenza segnalata
			size_t baseLen=strlen(e->d_name)-4;
			char sorgente[PERCORSO];
			snprintf(sorgente,sizeof sorgente,"%s/%.*s.cpl",dirTest,(int)baseLen,e->d_name);
			if (!eFileRegolare(sorgente))
				if (aggiungi(&rc->orfani,&rc->nOrfani,e->d_name)!=0) goto memoria;
		}
	}
	closedir(d);
	if (rc->nTest>1) qsort(rc->test,(size_t)rc->nTest,sizeof(char*),confrontaNomi);
	if (rc->nOrfani>1) qsort(rc->orfani,(size_t)rc->nOrfani,sizeof(char*),confrontaNomi);
	if (rc->nSoloCompilazione>1) qsort(rc->soloCompilazione,(size_t)rc->nSoloCompilazione,sizeof(char*),confrontaNomi);
	return 0;

memoria:
	closedir(d);
	fprintf(stderr,"runtest: memoria insufficiente per raccogliere i test\n");
	liberaElenco(rc->test,rc->nTest);
	liberaElenco(rc->orfani,rc->nOrfani);
	liberaElenco(rc->soloCompilazione,rc->nSoloCompilazione);
	rc->test=NULL;rc->nTest=0;rc->orfani=NULL;rc->nOrfani=0;
	rc->soloCompilazione=NULL;rc->nSoloCompilazione=0;
	return -1;
}

// ------------------------------------------------------------------
// directory di appoggio (D5)
// ------------------------------------------------------------------

// la directory di appoggio del giro: schema "runtest-XXXXXX" in TMPDIR (o in
// /tmp se TMPDIR non e' valorizzato), creata prima di eseguire e rimossa alla
// fine con tutto il suo contenuto, anche quando ci sono stati fallimenti
static int creaAppoggio(char* buf,size_t dim){
	const char* tmp=getenv("TMPDIR");
	if (tmp==NULL||tmp[0]=='\0') tmp="/tmp";
	snprintf(buf,dim,"%s/runtest-XXXXXX",tmp);
	if (mkdtemp(buf)==NULL){
		fprintf(stderr,"runtest: impossibile creare la directory di appoggio %s: %s\n",buf,strerror(errno));
		return -1;
	}
	return 0;
}

// rimozione della directory di appoggio: il runner ci mette solo file, quindi
// basta svuotarla e togliere la directory. Un processo interrotto prima di qui
// lascia una directory orfana in TMPDIR, mai un file nella directory dei test
static void rimuoviAppoggio(const char* dir){
	DIR* d=opendir(dir);
	struct dirent* e;
	if (d==NULL) return;
	while ((e=readdir(d))!=NULL){
		char percorso[PERCORSO];
		if (strcmp(e->d_name,".")==0||strcmp(e->d_name,"..")==0) continue;
		snprintf(percorso,sizeof percorso,"%s/%s",dir,e->d_name);
		unlink(percorso);
	}
	closedir(d);
	if (rmdir(dir)!=0)
		fprintf(stderr,"runtest: attenzione: non riesco a rimuovere %s: %s\n",dir,strerror(errno));
}

// copia il sorgente nella directory di appoggio: e' la copia a tenere intatta la
// directory dei test, perche' il compilatore scrivera' il .pcd accanto al file
// che gli diamo, e quello non deve essere un file dell'utente (D5)
static int copiaFile(const char* src,const char* dst){
	FILE* in=fopen(src,"rb");
	FILE* out;
	char buf[8192];
	size_t n;
	int ok=1;
	if (in==NULL) return -1;
	out=fopen(dst,"wb");
	if (out==NULL){fclose(in);return -1;}
	while ((n=fread(buf,1,sizeof buf,in))>0){
		if (fwrite(buf,1,n,out)!=n){ok=0;break;}
	}
	if (ferror(in)) ok=0;
	fclose(in);
	if (fclose(out)!=0) ok=0;
	return ok?0:-1;
}

// ------------------------------------------------------------------
// esecuzione dei processi (D2, D4)
// ------------------------------------------------------------------

// esegue un comando e ne restituisce il codice di uscita, o -1 se il processo
// non e' terminato normalmente (segnale, comando non partito)
static int esegui(const char* comando){
	int stato=system(comando);
	if (stato==-1||!WIFEXITED(stato)) return -1;
	return WEXITSTATUS(stato);
}

// esegue un comando catturando il solo standard output su un file (D4): stderr
// non viene toccato, quindi le diagnostiche dei due binari escono come escono
// da una normale esecuzione a mano. L'output catturato non viene mostrato nel
// rapporto quando il test supera: si mostra solo la differenza, se fallisce (D3)
static int eseguiCatturando(const char* comando,const char* outPath){
	char cmd[PERCORSO*2];
	snprintf(cmd,sizeof cmd,"%s > '%s'",comando,outPath);
	return esegui(cmd);
}

// ------------------------------------------------------------------
// confronto ed esito (D3)
// ------------------------------------------------------------------

// lettura intera di un file in memoria: il confronto e' un confronto di due
// contenuti, e lo stesso contenuto alimenta la differenza mostrata in caso di
// fallimento. NULL se il file non si puo' leggere
static char* leggiFile(const char* p,size_t* dim){
	FILE* f=fopen(p,"rb");
	char* buf;
	long n;
	if (f==NULL) return NULL;
	if (fseek(f,0,SEEK_END)!=0){fclose(f);return NULL;}
	n=ftell(f);
	if (n<0){fclose(f);return NULL;}
	rewind(f);
	buf=(char*)malloc((size_t)n+1);
	if (buf==NULL){fclose(f);return NULL;}
	if (fread(buf,1,(size_t)n,f)!=(size_t)n){free(buf);fclose(f);return NULL;}
	fclose(f);
	buf[n]='\0';
	*dim=(size_t)n;
	return buf;
}

// prossima riga del buffer: *da e' l'inizio della riga, la lunghezza restituita
// esclude il ritorno a capo e *da avanza oltre di esso; 0 con *da>=dim significa
// fine file
static size_t prossimaRiga(const char* b,size_t dim,size_t* da){
	size_t i=*da,inizio;
	if (i>=dim) return 0;
	inizio=i;
	while (i<dim&&b[i]!='\n') i++;
	*da=i<dim?i+1:dim;
	return i-inizio;
}

// la differenza fra il .out atteso e l'output ottenuto, mostrata riga per riga:
// "-" e' il file .out, "+" e' cio' che l'esecuzione ha prodotto. Non e' un diff
// minimal: e' la prima cosa che rende utile un fallimento (D3)
static void mostraDifferenza(const char* atteso,size_t na,const char* ottenuto,size_t nb){
	size_t ia=0,ib=0;
	int mostrate=0;
	fprintf(stderr,"  differenza fra il .out atteso e l'output ottenuto:\n");
	while (ia<na||ib<nb){
		size_t sa=ia,sb=ib;
		size_t la=prossimaRiga(atteso,na,&ia);
		size_t lb=prossimaRiga(ottenuto,nb,&ib);
		// le righe che combaciano non interessano; quando un file e' finito la
		// sua riga "mancante" va comunque mostrata dall'altro lato
		if (sa<na&&sb<nb&&la==lb&&memcmp(atteso+sa,ottenuto+sb,la)==0) continue;
		if (sa<na) fprintf(stderr,"  - atteso : %.*s\n",(int)la,atteso+sa);
		if (sb<nb) fprintf(stderr,"  + ottenuto: %.*s\n",(int)lb,ottenuto+sb);
		mostrate++;
		if (mostrate>=10){fprintf(stderr,"  ... altre differenze\n");break;}
	}
	// righe identiche ma byte diversi (per esempio il ritorno a capo finale):
	// la differenza va detta lo stesso, altrimenti il fallimento resta senza
	// nessuna spiegazione visibile
	if (mostrate==0)
		fprintf(stderr,"  stesse righe ma byte diversi: atteso %ld, ottenuto %ld\n",(long)na,(long)nb);
}

// ------------------------------------------------------------------
// un singolo test (D2, D5, D6)
// ------------------------------------------------------------------

// esegue un test e aggiorna il rapporto: copia il .cpl nell'appoggio, compila
// con ./compiler, e per un test completo esegue con ./interpreter catturando il
// solo stdout e confronta con il .out originale. Con soloCompilazione la fase
// di esecuzione salta: il test e' superato appena la compilazione ha esito 0
// (requisito "Test di sola compilazione"). Un qualsiasi passaggio che non riesce
// rende il test fallito e il giro continua con gli altri (requisito "Isolamento
// ed esito di ogni test")
static void eseguiTest(const char* dirTest,const char* appoggio,const char* nome,int soloCompilazione,rapporto* rap){
	char src[PERCORSO],dst[PERCORSO],cmd[PERCORSO],catturato[PERCORSO],atteso[PERCORSO];
	char motivo[PERCORSO+128];
	int rc;

	// copia del sorgente nella directory di appoggio (D5)
	snprintf(src,sizeof src,"%s/%s.cpl",dirTest,nome);
	snprintf(dst,sizeof dst,"%s/%s.cpl",appoggio,nome);
	if (copiaFile(src,dst)!=0){
		snprintf(motivo,sizeof motivo,"impossibile copiare il sorgente: %s",strerror(errno));
		fallito(rap,nome,motivo);
		return;
	}

	// compilazione con il binario, come farebbe l'utente dalla command line
	// (D2): se il binario non si puo' eseguire, il motivo finisce nel rapporto
	if (access("./compiler",X_OK)!=0){
		snprintf(motivo,sizeof motivo,"binario ./compiler non eseguibile: %s",strerror(errno));
		fallito(rap,nome,motivo);
		return;
	}
	snprintf(cmd,sizeof cmd,"./compiler '%s'",dst);
	rc=esegui(cmd);
	if (rc!=0){
		if (rc<0) snprintf(motivo,sizeof motivo,"compilazione terminata in modo anomalo");
		else snprintf(motivo,sizeof motivo,"compilazione fallita (codice %d)",rc);
		fallito(rap,nome,motivo);
		return;
	}

	// test di sola compilazione: la riuscita della compilazione e' l'esito, e
	// ./interpreter non viene mai toccato (requisito "Test di sola
	// compilazione")
	if (soloCompilazione){
		rap->superati++;
		fprintf(stderr,"SUPERATO %s\n",nome);
		return;
	}

	// esecuzione con ./interpreter in modalita' normale e senza nessuna opzione
	// di misura (D4): la tabella di nanobench non entra mai nel confronto
	if (access("./interpreter",X_OK)!=0){
		snprintf(motivo,sizeof motivo,"binario ./interpreter non eseguibile: %s",strerror(errno));
		fallito(rap,nome,motivo);
		return;
	}
	snprintf(catturato,sizeof catturato,"%s/%s.stdout",appoggio,nome);
	// il .pcd glielo si da senza estensione: e' l'interprete ad aggiungere
	// ".pcd", sempre sul file appena prodotto dal compilatore (D2, 3.2)
	snprintf(cmd,sizeof cmd,"./interpreter '%s/%s'",appoggio,nome);
	rc=eseguiCatturando(cmd,catturato);
	if (rc!=0){
		if (rc<0) snprintf(motivo,sizeof motivo,"esecuzione terminata in modo anomalo");
		else snprintf(motivo,sizeof motivo,"esecuzione fallita (codice %d)",rc);
		fallito(rap,nome,motivo);
		return;
	}

	// confronto byte per byte con il .out originale, nella directory dei test
	// (D3): nessuna normalizzazione, nessuna tolleranza sugli spazi
	snprintf(atteso,sizeof atteso,"%s/%s.out",dirTest,nome);
	{
		size_t na=0,nb=0;
		char* a=leggiFile(atteso,&na);
		char* b=leggiFile(catturato,&nb);
		if (a==NULL||b==NULL){
			fallito(rap,nome,"impossibile leggere il .out o l'output catturato");
		}else if (na==nb&&memcmp(a,b,na)==0){
			rap->superati++;
			fprintf(stderr,"SUPERATO %s\n",nome);
		}else{
			fallito(rap,nome,"output diverso dal .out");
			mostraDifferenza(a,na,b,nb);
		}
		free(a);
		free(b);
	}
}

int main(int argc,char** argv){
	// la directory dei test e' l'unico posizionale, "test" come default, cosi'
	// l'uso tipico resta il semplice "./runtest" (D8)
	const char* dirTest="test";
	const char* pos[1];
	cliScansione sc;
	int nPos;
	rapporto rap={0,0};
	raccolta rc;
	char appoggio[PERCORSO];
	int k;

	cliInizia(&sc,argc,argv);
	nPos=cliScansiona(&sc,tabOpzioni,1,pos,1);
	if (sc.errore!=CLI_ERRORE_NESSUNO){
		diagOpzione(&sc);
		return 1;
	}
	if (nPos>1){
		fprintf(stderr,"runtest accetta al piu' una directory\n");
		return 1;
	}
	if (nPos==1) dirTest=pos[0];

	if (raccogli(dirTest,&rc)!=0) return 1;

	// un .out senza .cpl e' un incoerenza nominata che conta come fallimento:
	// e' un output atteso che nessuno verifica piu' (requisito "Output atteso
	// senza sorgente")
	for (k=0;k<rc.nOrfani;k++)
		fallito(&rap,rc.orfani[k],"output atteso senza .cpl corrispondente");

	// la directory di appoggio del giro: creata prima di eseguire, rimossa alla
	// fine con tutto il suo contenuto, anche in presenza di fallimenti (D5)
	if (creaAppoggio(appoggio,sizeof appoggio)!=0){
		liberaElenco(rc.test,rc.nTest);
		liberaElenco(rc.orfani,rc.nOrfani);
		liberaElenco(rc.soloCompilazione,rc.nSoloCompilazione);
		return 1;
	}
	{
		DIR* d2=opendir(dirTest);
		if (d2!=NULL){
			struct dirent* e;
			while((e=readdir(d2))!=NULL){
				if (strcmp(e->d_name,".")==0||strcmp(e->d_name,"..")==0) continue;
				if (finisceCon(e->d_name,".cpl")||finisceCon(e->d_name,".pcd")){
					char s[1024],t[1024];
					snprintf(s,sizeof s,"%s/%s",dirTest,e->d_name);
					snprintf(t,sizeof t,"%s/%s",appoggio,e->d_name);
					copiaFile(s,t);
				}
			}
			closedir(d2);
		}
	}
	// prima i test completi, poi i test di sola compilazione (D5): entrambi gli
	// elenchi sono gia' ordinati per nome, quindi il giro resta deterministico
	for (k=0;k<rc.nTest;k++)
		eseguiTest(dirTest,appoggio,rc.test[k],0,&rap);
	for (k=0;k<rc.nSoloCompilazione;k++)
		eseguiTest(dirTest,appoggio,rc.soloCompilazione[k],1,&rap);
	rimuoviAppoggio(appoggio);

	liberaElenco(rc.test,rc.nTest);
	liberaElenco(rc.orfani,rc.nOrfani);
	liberaElenco(rc.soloCompilazione,rc.nSoloCompilazione);

	fprintf(stderr,"runtest: superati %d, falliti %d\n",rap.superati,rap.falliti);
	return rap.falliti?1:0;
}
