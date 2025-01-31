#include <cassert>
#include <unordered_set>

using namespace std;

// ogni oggetto da sottoporre a GC deve derivare da questo che implementa il funzionamento di base
class GCObject {
protected:	
  bool marked;
  int locked;
  //
  virtual int childCnt(){return 0;}
  virtual GCObject* getChild(int p){return nullptr;}
  virtual void reclaim(){delete this;}
public:
  GCObject();
  virtual ~GCObject(){ /* cout << "reclaimed " << this << endl;*/};
  int lock(){return ++locked;};
  int unlock(){assert(locked>0);return --locked;};
  //
  int generation;
  virtual void mark(){if (!marked) {marked=true;for(int i=0;i<childCnt();i++){auto c=getChild(i);if(c) c->mark();}};}
  virtual void expand(int gen){if (generation<gen) generation=gen; for(int i=0;i<childCnt();i++){auto c=getChild(i);if(c && c->generation<generation) c->expand(generation);}}
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
  explicit GC(int m){maxgen=m;}
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
        //cout << (*it) << " generation " << (*it)->generation << endl;
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
  //status();
}

inline void GC::status(){
  cout << "objs " << objs.size() << " object x generation:";
  int gen_cnt[maxgen+1],i,locked=0;
  for(i=0;i<=maxgen;i++) gen_cnt[i]=0;
  for (const GCObject* const& it : objs){
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
    virtual int childCnt() override {return gcObjs.size();}
    virtual GCObject* getChild(int p) override ;
 public:
    void put(gc_ptr<GCObject>* p){gcObjs.push_back(p);}
};

// puntatore SMART che implementa da solo la ricerca dei figli
template <class T> class gc_ptr {
  GCObject* ptr=0; // questo deve essere un oggetto che deriva da GCObject
public:
  explicit gc_ptr(GCAuto* parent){parent->put(static_cast<gc_ptr<GCObject>*>(this));}
  T* operator->(){return (T*)ptr;}
  T* operator=(T* p){ptr=p;return p;}
  operator T*(){return (T*)ptr;}
};
#define GC_ptr(cl,name) gc_ptr<cl> name{gc_ptr<cl>(this)}

inline GCObject* GCAuto::getChild(int p){
    //gc_ptr<GCObject>* o=gcObjs[p];
    //return *o;
    return *gcObjs[p];
}

//// Puntatori simili agli smart pointers per tenere bloccati gli oggetti che fanno da root

// puntatore da usare nel programma (non nei dati) che tiene lockato l'oggetto che referenzia.
// quando viene distrutto l'oggetto può essere reclamato.
template <class T> class lockgc_ptr {
  T* ptr=0; // questo deve essere un oggetto che deriva da GCObject
public:
  lockgc_ptr(){ptr=0;};
  explicit lockgc_ptr(T* p){ptr=p;if(p) p->lock();};
  virtual ~lockgc_ptr(){if(ptr) ptr->unlock();}
  T* operator->(){return (T*)ptr;}
  T* operator=(T* p){if(ptr) ptr->unlock();ptr=p;if(p) p->lock();return p;}
  lockgc_ptr& operator=(const lockgc_ptr& p){if(ptr) ptr->unlock();ptr=p.ptr;if (ptr) ptr->lock(); return *this;}
  operator T* const &() const {return ptr;}
  //operator T*(){return ptr;}
};

template <class T> class gc_array_ : public GCObject {
  vector<T*>& data;
public:	
  gc_array_(vector<T*>& d):data{d}{}
  virtual int childCnt() override {return data.size();}
  virtual T* getChild(int p) override {return data[p];}
};

template <class T1,class T2> class gc_dict_ : public GCObject {
  unordered_map<T1,T2*>& data;
public:	
  gc_dict_(unordered_map<T1,T2*>& d):data{d}{}
  virtual void mark(){
	if (!marked) {
	  marked=true;
	  for(auto&[k,o]:data) 
	    o->mark();
	 };
  }
  virtual void expand(int gen){
	if (generation<gen) generation=gen; 
	for(auto& [k,o]:data){
	  if(o->generation<generation) 
	    o->expand(generation);
	 }
  }
};
