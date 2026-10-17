#include <iostream>
#include <unordered_set>
#include <vector>
#include <map>
#include <cassert>

/*
DA FARE:
test esaustivi

FATTO:
collect automatico alla insert di un nuovo oggetto ... sembra che abbia trovato un buon compromesso: collect(0) ogni 10000 oggetti, 1 su 10 collect(1), uno su 100 collect(max)
iteratore per la lista degli oggetti contenuti
*/

using namespace std;

// ogni oggetto da sottoporre a GC deve derivare da questo che implementa il funzionamento di base
class GCObject {
  int generation;
  bool marked;
  int locked;
  //
  virtual int childCnt()=0;
  virtual GCObject* getChild(int p)=0;
  virtual void mark(){if (!marked) {marked=true;for(int i=0;i<childCnt();i++){auto c=getChild(i);if(c) c->mark();}};}
  virtual void expand(int gen){if (generation<gen) generation=gen; for(int i=0;i<childCnt();i++){auto c=getChild(i);if(c && c->generation<generation) c->expand(generation);}}
  virtual void reclaim(){delete this;}
public:
  GCObject();
  virtual ~GCObject(){ /* cout << "reclaimed " << this << endl;*/};
  int lock(){return ++locked;};
  int unlock(){assert(locked>0);return --locked;};
  friend class GC;
};

class GC {
  // insieme di tutti gli oggetti da gestire
  unordered_set <GCObject*> objs;
  int maxgen;
  int objlimit=10000,added=0;
  long cnt=0;
  //
  void mark(int gen); // marca tutti gli oggetti raggiungibili della generazione specificata,
                      // quelli di generazioni successive sono considerati raggiunti
  void sweep(int gen);// elimina gli oggetti che non sono più raggiungibili
public:
  GC(int m){maxgen=m;}
  virtual ~GC(){ // distrugge il GC, elimina tutti gli oggetti ancora vivi così viene chiamato il loro distruttore
      int sz=0,locked=0;
      for(const auto& it:objs){
        if (it->locked>0) locked++;
        delete it;
      }
      objs.clear();
      cout << "--- closing GC, objs:" << sz << " locked:" << locked << " cnt:" << cnt << endl;
    }
  void add(GCObject* o){ // aggiunge un oggetto agli oggetti che gestisce, se è il caso chiama la garbage collection
      if (added>objlimit) {
          int g=rand()%100==0?maxgen:(rand()%10==0?1:0); // ogni 10 collect(1), ogni 100 collect maxgen
          //cout << objs.size();
          collect(g);
          added=0;
          //cout << " autogc(" << g << ") " << objs.size() << endl;
      }
      objs.insert(o); // aggiunge l'oggetto agli oggetti noti
      added++; // conteggia gli oggetti aggiunti per far scattare il GC
      cnt++;
      //if (debug) cout << "inserted " << o << endl;
      }
  void collect(int gen=0){if(gen>maxgen) gen=maxgen;mark(gen);sweep(gen);}
  void collectall(){collect(maxgen);}
  static GC& getGC(){static GC theGC(2);return theGC;}
  void status();
  int setGenerations(int gen){ // cambia il numero di generazioni, se viene diminuito bisogna abbassare tutti gli oggetti di generazioni che non esisteranno più
    if (maxgen>gen){ // se si sono abbassate le generazioni riposiziona gli oggetti delle generazioni eliminate
      for(const auto& it: objs)
        if(it->generation>gen)
          it->generation=gen;
    };
    int old=maxgen; maxgen=gen;return old; // ritorna il vecchio numero di generazioni
  }
};

inline GCObject::GCObject(){
  generation=0;
  marked=false;
  locked=0;
  GC::getGC().add(this);
  //cout << "created " << this << endl;
}

inline void GC::mark(int gen){
  // bisogna allineare alla generazione più alta tutti i figli e marcare tutti gli oggetti da conservare
  // gli oggetti da conservare sono di generazione più alta di "gen" e quelli che sono raggiungibili dal programma

  // esegue prima la marcatura usando l'informazione delle generazioni: tutti gli oggetti di generazioni più alte di quella da reclamare marcano i loro figli con la loro generazione
  // in questo modo gli oggetti che sono vivi da molto tempo vengono esplorati raramente dal "mark" ricorsivo
  for(const auto& it:objs){
    if (it->generation>gen)       // se l'oggetto è maggiore della generazione da reclamare
      it->expand(it->generation); // porta alla sua generazione tutti i suoi discendenti; equivale alla marcatura, ma molto più persistente
  }
  // smarca gli oggetti che potrebbero essere rilasciati
  for(const auto& it : objs){
    it->marked=(it->generation>gen); // marca come non raggiunti tutti gli oggetti di generazione minore di "gen"
  }
  // percorre tutti gli oggetti che appaiono raggiungibili
  for (const auto& it : objs){
      if (it->locked /*&& !it->marked*/)  // se l'oggetto è parte degli oggetti raggiungibili da programma ed è di una generazione che può essere reclamata
        it->mark();                   // lo marca e marca tutti gli oggetti raggiungibili da questo oggetto
  }
  // ora tutti gli oggetti di generazione minore o uguale a "gen" (quella da reclamare) sono stati marcati se sono ancora raggiungibili
  // tutti gli oggetti di generazione maggiore a "gen" sono stati marcati
  // tutti gli oggetti collegati a oggetti di generazioni più vecchie sono stati allineati alla generazione del padre
}

inline void GC::sweep(int gen){
  // tutti gli oggetti di generazione superiore a "gen" o raggiungibili sono stati marcati
  // quelli non marcati sono da reclamare
  bool shiftGen=gen<maxgen;
  for (auto it=objs.begin();it!=objs.end();){
    if ((*it)->marked){
      // oggetto marcato, si deve far salire di generazione
      if (shiftGen && (*it)->generation<=gen){ // gli oggetti sopravvissuti che erano sotto "gen" salgono di generazione
        (*it)->generation++;
        //if (debug) cout << (*it) << " generation " << (*it)->generation << endl;
      }
      ++it;
    } else {
      // oggetto non marcato, si deve rilasciare
      GCObject* ptr=(*it);
      it=objs.erase(it);
      //delete ptr;
      ptr->reclaim();
      //cout << "deleted " << ptr << endl;
    }
  }
}

inline void GC::status(){
  cout << "objs " << objs.size() << " object x generation:";
  int gen_cnt[maxgen+1],i,locked=0;
  for(i=0;i<=maxgen;i++) gen_cnt[i]=0;
  for (const auto&it : objs){
    gen_cnt[it->generation]++;
    if (it->locked>0) locked++;
  }
  for(i=0;i<=maxgen;i++)
    cout << gen_cnt[i] << " ";
  cout << " locked:" << locked << endl;
}

inline GC& stdGC(){return GC::getGC();}

//// Sistema per automatizzare la ricerca dei figli (DA FARE: commentare)

template <class T> class gc_ptr;

class GCAuto: public GCObject {
    vector<gc_ptr<GCObject>*> gcObjs={};
    virtual int childCnt(){return gcObjs.size();}
    virtual GCObject* getChild(int p);
 public:
    void put(gc_ptr<GCObject>* p){gcObjs.push_back(p);}
};

// puntatore SMART che implementa da solo la ricerca dei figli
template <class T> class gc_ptr {
  GCObject* ptr=0; // questo deve essere un oggetto che deriva da GCObject
public:
  gc_ptr(GCAuto* parent){parent->put((gc_ptr<GCObject>*)this);}
  T* operator->(){return (T*)ptr;}
  T* operator=(T* p){ptr=p;return p;}
  operator T*(){return (T*)ptr;}
};
#define GC_ptr(cl,name) gc_ptr<cl> name{gc_ptr<cl>(this)}

inline GCObject* GCAuto::getChild(int p){
    gc_ptr<GCObject>* o=gcObjs[p];
    return *o;
}

//// Puntatori simili agli smart pointers per tenere bloccati gli oggetti che fanno da root

// puntatore da usare nel programma (non nei dati) che tiene lockato l'oggetto che referenzia.
// quando viene distrutto l'oggetto può essere reclamato.
template <class T> class lockgc_ptr {
  GCObject* ptr=0; // questo deve essere un oggetto che deriva da GCObject
public:
  lockgc_ptr(){ptr=0;};
  lockgc_ptr(T* p){ptr=p;if(p) p->lock();};
  virtual ~lockgc_ptr(){if(ptr) ptr->unlock();}
  T* operator->(){return (T*)ptr;}
  T* operator=(T* p){if(ptr) ptr->unlock();ptr=p;if(p) p->lock();return p;}
  lockgc_ptr& operator=(lockgc_ptr& p){if(ptr) ptr->unlock();ptr=p.ptr;if (ptr) ptr->lock(); return *this;}
  operator T*(){return (T*)ptr;}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class myObj: public GCObject { // classe di base che implementa le due funzioni per l'esplorazione durante la garbage collection
  virtual int childCnt(){return 2;}
  virtual GCObject* getChild(int p){if(p==0) return prev; return next;}
public:
  int i;
  myObj *prev=0,*next=0;
};

class myObjA: public GCAuto { // classe che usa i puntatori SMART per la ricerca automatica dei figli
public:
  myObjA():ppp(this){}
  gc_ptr<myObjA> next{gc_ptr<myObjA>(this)}; // espanso!!
  GC_ptr(myObjA,prev); // macro
  gc_ptr<myObjA> ppp;  // da inizializzare nel costruttore
};

void test1(){
    // primo test, un po' a caso
    cout << "----- Test 1 -----" << endl;
    myObj* ptr=new myObj();
    ptr->i=10;
    ptr->next=new myObj();
    ptr->next->i=100;
    cout << "ptr->i=" << ptr->i << " ptr->next->i=" << ptr->next->i << endl;
    ptr->lock();
    stdGC().status();
    stdGC().collect();
    ptr->unlock();
    lockgc_ptr<myObj> ooo=new myObj();
    ooo->i=200;
    stdGC().status();
    stdGC().collect();
    ptr=ptr->next;ptr->lock();
    stdGC().collect(1);
    stdGC().status();
    stdGC().collect(1);
    stdGC().status();
    ptr->unlock();
}

void test2(){
   // creazione di tanti oggetti e GC random
    cout << "----- Test 2 -----" << endl;
   for(int i=0;i<100000;i++){
        new myObj();
        if (rand()%20000==0) {
          stdGC().status();
          stdGC().collect();
          stdGC().status();
        }
   }
}

void test3(){
   // creazione di tanti oggetti con uno bloccato e GC random
    cout << "----- Test 3 -----" << endl;
   lockgc_ptr<myObj> ptr;
   for(int i=0;i<1000;i++){
        ptr=new myObj();
        if (rand()%100==0) {
          stdGC().status();
          stdGC().collect();
          stdGC().status();
        }
   }
   ptr=0;
   cout << "fine test 3 ora gira un collectall senza lock" << endl;
   stdGC().status();
   stdGC().collectall();
   stdGC().status();
}

void test4(){
   // creazione di tanti oggetti con uno bloccato e GC random, random assegnazione a prev o next
   cout << "----- Test 4 -----" << endl;
   lockgc_ptr<myObj> ptr,ptr1;
   myObj* ptr2=0;
   int k=0;
   for(int i=0;i<10000;i++){
        int op=rand()%6;
        switch(op){
            case 0:ptr=new myObj();ptr2=ptr;break;
            case 1:
            case 2:if (ptr2) {ptr2->next=new myObj();ptr2=ptr2->next;} break;
            case 3:
            case 4:if (ptr2) {ptr2->prev=new myObj();ptr2=ptr2->prev;} break;
            case 5:if (k==0) ptr1=ptr; k=1;break; // crea un oggetto che vivrà tantissimo
        }
        op=rand()%10000;
        if (op<=5) {
          cout << "collect " << op << endl;
          stdGC().status();
          stdGC().collect(op);
          stdGC().status();
        }
   }
   ptr=ptr1=0; // se non azzero questi due puntatori restano bloccati due oggetti, che si sbloccheranno fuori dalla funzione
   cout << "--- fine test 4 ora gira un collectall senza lock ---" << endl;
   stdGC().status();
   stdGC().collectall();
   stdGC().status();
   cout << "--- fine GC test 4 ---" << endl;
}

void test5(){
   // creazione di tanti oggetti con uno bloccato e GC random, random assegnazione a prev o next
   cout << "----- Test 5 con gc_ptr -----" << endl;
   lockgc_ptr<myObjA> ptr,ptr1;
   myObjA* ptr2=0;
   for(int i=0;i<100000;i++){
        int op=rand()%6;
        switch(op){
            case 0:ptr=new myObjA();ptr2=ptr;break;
            case 1:
            case 2:if (ptr2) {ptr2->next=new myObjA();ptr2=ptr2->next;} break;
            case 3:
            case 4:if (ptr2) {ptr2->prev=new myObjA();ptr2=ptr2->prev;} break;
            case 5:if (ptr1==0) ptr1=ptr; break; // crea un oggetto che vivrà tantissimo
        }
        /*
        op=rand()%10000;
        if (op<=5) {
          cout << "collect " << op << endl;
          stdGC().status();
          stdGC().collect(op);
          stdGC().status();
        }
        */
   }
   ptr=ptr1=0; // se non azzero questi due puntatori restano bloccati due oggetti, che si sbloccheranno fuori dalla funzione
   cout << "--- fine test 5 ora gira un collectall senza lock ---" << endl;
   stdGC().status();
   stdGC().collectall();
   stdGC().status();
   cout << "--- fine GC test 5 ---" << endl;
}

class lll: public GCObject {
    // caso complicato: due oggetti che tengono puntatori, un vettore e una mappa.
    // la "getChild" deve enumerare questi puntatori, facile per il vettore, più complicato per la mappa
    virtual int childCnt(){return vvv.size()+mmm.size();}
    virtual GCObject* getChild(int p){
        // ritorna il puntatore degli elementi del vettore vvv
        int cnt=vvv.size();if (p<cnt) return vvv[p];
        p-=cnt; // si sposta oltre il vettore ...
        // ritorna il puntatore degli elementi della mappa mmm
        auto it=mmm.begin();while (p--) it++;return it->second;
    }
public:
    vector<myObj*> vvv;
    map<int,myObj*> mmm;
};

void test6(){
    cout << "-------- Test 6" << endl;
    lockgc_ptr<lll> o=new lll();
    for(int i=0;i<10000;i++)
      o->vvv.push_back(new myObj);
    for(int i=0;i<1000;i++)
      o->mmm[i]=new myObj();
    stdGC().status();
    stdGC().collect();
    stdGC().status();
    o=0;
    stdGC().collect(2);
    stdGC().status();
}


////////////////// Test FIBONACCI

class fibInt: public GCObject {
    virtual int childCnt(){return 0;}
    virtual GCObject* getChild(int p){return 0;}
public:
  fibInt(int n){value=n;}
  int value;
};

class fibStack : public GCObject {
    virtual int childCnt(){return stk.size();}
    virtual GCObject* getChild(int p){return stk[p];}
public:
    vector<fibInt*> stk;
    void push(fibInt* value){stk.push_back(value);}
    fibInt* pop(){fibInt* res=stk.back();stk.pop_back();return res;}
};

void fib(fibStack* stk){ // tutt le operazioni avvengono sullo stack
  int n=stk->pop()->value;
  if (n==0){
    stk->push(new fibInt(0));
  } else if (n==1){
    stk->push(new fibInt(1));
  } else {
    stk->push(new fibInt(n-1));
    fib(stk); // sullo stack fib(n-1);
    stk->push(new fibInt(n-2));
    fib(stk); // sullo stack fib(n-2);
    stk->push(new fibInt(stk->pop()->value+stk->pop()->value));
  }
}

void testFib(){
  cout << "--------- testFib:calcolo del numero di Fibonacci" << endl;
  int n=32;
  lockgc_ptr<fibStack> stk=new fibStack();
  stk->push(new fibInt(n));
  fib(stk);
  cout << "fib(" << n << ")=" << stk->pop()->value << endl;
}

//////////////////

int main(){

    //stdGC().setGenerations(3);

    test1();
    cout << "fullGC test1"<<endl;stdGC().status();stdGC().collectall();stdGC().status();

    test2();
    cout << "fullGC test2"<<endl;stdGC().status();stdGC().collectall();stdGC().status();

    test3();
    cout << "fullGC test3"<<endl;stdGC().status();stdGC().collectall();stdGC().status();

    test4();
    cout << "fullGC test4"<<endl;stdGC().status();stdGC().collectall();stdGC().status();

    test5();
    cout << "fullGC test5"<<endl;stdGC().status();stdGC().collectall();stdGC().status();

    test6();
    cout << "fullGC test6"<<endl;stdGC().status();stdGC().collectall();stdGC().status();

    testFib();
    cout << "fullGC testFib"<<endl;stdGC().status();stdGC().collectall();stdGC().status();

    cout << "-- terminati tutti i test --" << endl;

    return 0;
}


