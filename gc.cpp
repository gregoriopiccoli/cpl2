/*
DA FARE:
migliorare esplorazione quando si cambia generazione, ora ricerca sempre ...
supportare più di 2 generazioni (ora scandice 0,1 o max)  
*/

#include <cassert>

using namespace std;

#define GC_OBJSLIM  5000 //10000  // Ogni quanti nuovi oggetti esegue una collect
#define GC_GEN      4    //2      // Il numero di generazioni
#define GC_GENSCALE 4    //8      // la proporzione prima di salire alla collect del livello superiore    

//#define GC_USING_USET

#ifdef GC_USING_USET
#include <unordered_set>
#endif

int gc_ending=0;

// ogni oggetto da sottoporre a GC deve derivare da questo che implementa il funzionamento di base
class GCObject {
protected:	
  bool marked;
  int locked;
  //
  virtual int childCnt(){return 0;}
  virtual GCObject* getChild(int p){return nullptr;}
  virtual bool reclaim(){delete this;return true;}
public:
  GCObject();
  virtual ~GCObject(){ /* cout << "reclaimed " << this << endl;*/};
  virtual int lock(){return ++locked;};
  virtual int unlock(){ if (locked==0 && !gc_ending) {cout << print() << endl; assert(locked>0);} return --locked;};
  virtual int lockCnt(){return locked;}
  //
  int generation;
  virtual void mark(){
	if (!marked) {
	  marked=true;
	  for(int i=0;i<childCnt();i++){
		auto c=getChild(i);
		if(c) c->mark();
	  }
	 }
  }
  virtual void expand(int gen){
	if (generation<gen) 
	  generation=gen;
	int end=childCnt();   
	for(int i=0;i<end;i++){
	  auto c=getChild(i);
	  if(c && c->generation<generation) 
	    c->expand(generation);
	}
  }
  //
  friend class GC;
  //
  virtual string print() const {return "untyped obj";}
};

class GC {
  // insieme di tutti gli oggetti da gestire
#ifdef GC_USING_USET  
  unordered_set <GCObject*> objs;
#else  
  vector<GCObject*> objs;
#endif  
  int maxgen;
  int objlimit=GC_OBJSLIM,added=0;
  unsigned long cnt=0,maxlive=0,maxsize=0,gcexecutions=0;
  //
  void mark(int gen); // marca tutti gli oggetti raggiungibili della generazione specificata,
                      // quelli di generazioni successive sono considerati raggiunti
  void sweep(int gen);// elimina gli oggetti che non sono più raggiungibili
public:
  explicit GC(int m){maxgen=m;}
  virtual ~GC(){ // distrugge il GC, elimina tutti gli oggetti ancora vivi così viene chiamato il loro distruttore
	  //cout << "distrugge il GC, elimina tutti gli oggetti ancora vivi per chiamare il distruttore\n";
	  gc_ending=1;
      int sz=0,locked=0;
      for(const auto& it:objs){
        if (it->locked>0) {
			locked++;
			//cout << "locked on closing:" << it->print() << endl;
		}
        delete it;
	  }
      objs.clear();
      cout << "--- closing GC, objs:" << sz << " locked:" << locked << " cnt:" << cnt << " maxlive:" << maxlive << " maxsize:" << maxsize << " gcexec:" << gcexecutions << endl;
    }
  void add(GCObject* o){ // aggiunge un oggetto agli oggetti che gestisce, se è il caso chiama la garbage collection
      if (added>objlimit) {
          //int g=rand()%100==0?maxgen:(rand()%10==0?1:0); // ogni 10 collect(1), ogni 100 collect maxgen
          int g=0; while (rand()%GC_GENSCALE==0 && g<maxgen) g++;
          //cout << objs.size() << " collecting gen: " << g;
          collect(g);
          added=0;
          //cout << " after: " << objs.size() << endl;
      }
#ifdef GC_USING_USET  
      objs.insert(o); // aggiunge l'oggetto agli oggetti noti
#else      
      objs.push_back(o); // aggiunge l'oggetto agli oggetti noti
#endif      
      added++; // conteggia gli oggetti aggiunti per far scattare il GC
      cnt++;
      //if (debug) cout << "inserted " << o << endl;
      }
  void addRecycled(GCObject* o){objs.push_back(o);}    
  void collect(int gen=0){if(gen>maxgen) gen=maxgen;/*long n=objs.size();*/mark(gen);sweep(gen);gcexecutions++;/*long nn=n-objs.size();cout << "recuperati:" << nn << " gen:" << gen << endl;*/}
  void collectall(){collect(maxgen);}
  static GC& getGC(){static GC theGC(GC_GEN);return theGC;}
  void status();
  int setGenerations(int gen){ // cambia il numero di generazioni, se viene diminuito bisogna abbassare tutti gli oggetti di generazioni che non esisteranno più
    if (maxgen>gen){ // se si sono abbassate le generazioni riposiziona gli oggetti delle generazioni eliminate
      for(const auto& it: objs)
        if(it->generation>gen)
          it->generation=gen;
    };
    int old=maxgen; maxgen=gen;return old; // ritorna il vecchio numero di generazioni
  }
  void printLocked(){
	for(const GCObject* const& o:objs) 
	  if (o->locked>0)
	    cout << o->print() << endl;
  }
};


inline GC& stdGC(){return GC::getGC();}

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
  //if (gc_ending) cout << "inizio expand\n";
  for(const auto& it:objs){
	//if (gc_ending && it->generation>gen) {cout << it->print() << " gen:" << it->generation << endl;}
    if (it->generation>gen)       // se l'oggetto è maggiore della generazione da reclamare
      it->expand(it->generation); // porta alla sua generazione tutti i suoi discendenti; equivale alla marcatura, ma molto più persistente
  }
  //if (gc_ending) cout << "fine expand\n";
  // smarca gli oggetti che potrebbero essere rilasciati
  for(const auto& it : objs){
    it->marked=(it->generation>gen); // marca come non raggiunti tutti gli oggetti di generazione minore di "gen"
  }
  //if (gc_ending) cout << "fine mark per gen\n";
  // percorre tutti gli oggetti che appaiono raggiungibili
  for (const auto& it : objs){
      if (it->locked /*&& !it->marked*/)  // se l'oggetto è parte degli oggetti raggiungibili da programma ed è di una generazione che può essere reclamata
        it->mark();                   // lo marca e marca tutti gli oggetti raggiungibili da questo oggetto
  }
  //if (gc_ending) cout << "fine mark per lock\n";
  // ora tutti gli oggetti di generazione minore o uguale a "gen" (quella da reclamare) sono stati marcati se sono ancora raggiungibili
  // tutti gli oggetti di generazione maggiore a "gen" sono stati marcati
  // tutti gli oggetti collegati a oggetti di generazioni più vecchie sono stati allineati alla generazione del padre
}

inline void GC::sweep(int gen){
  // tutti gli oggetti di generazione superiore a "gen" o raggiungibili sono stati marcati
  // quelli non marcati sono da reclamare
  bool shiftGen=gen<maxgen;
  if (objs.size()>maxsize) maxsize=objs.size();
  unsigned long m=0;
#ifdef GC_USING_USET  
  for (auto it=objs.begin();it!=objs.end();){
    if ((*it)->marked){
      // oggetto marcato, si deve far salire di generazione
      //if (shiftGen && (*it)->generation<=gen){ // gli oggetti sopravvissuti che erano sotto "gen" salgono di generazione
      //  (*it)->generation++;
      if ((*it)->generation<gen){ // gli oggetti sopravvissuti che erano sotto "gen" salgono di generazione
        (*it)->generation=gen;
        //cout << (*it) << " generation " << (*it)->generation << endl;
      }
      ++it;
      m++;
    } else {
      // oggetto non marcato, si deve rilasciare
      GCObject* ptr=(*it);
      it=objs.erase(it);
      ptr->reclaim();
      //cout << "deleted " << ptr << endl;
    }
  }
#else
  vector<GCObject*> nnn;
  for(auto& o:objs) {
	if (o->marked){
      if (shiftGen && o->generation<=gen){ // gli oggetti sopravvissuti che erano sotto "gen" salgono di generazione
        o->generation++;
        //cout << (*it) << " generation " << (*it)->generation << endl;
      }
      nnn.push_back(o);
      m++;
	} else {
      o->reclaim();
	} 
  } 
  objs=nnn;
#endif 
  if (m>maxlive) maxlive=m;
  //status();showIntCache();
}

inline void GC::status(){
  cout << "objs " << objs.size() << " object x generation:";
  int gen_cnt[GC_GEN+1],i,locked=0;
  for(i=0;i<=maxgen;i++) gen_cnt[i]=0;
  for (const GCObject* const& it : objs){
    gen_cnt[it->generation]++;
    if (it->locked>0) locked++;
  }
  for(i=0;i<=maxgen;i++)
    cout << gen_cnt[i] << " ";
  cout << " locked:" << locked << endl;
}

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
  GCObject* ptr=nullptr; // questo deve essere un oggetto che deriva da GCObject
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
  T* ptr; // questo deve essere un oggetto che deriva da GCObject
public:
  lockgc_ptr(){ptr=nullptr;};
  explicit lockgc_ptr(T* p){ptr=p;if(p) p->lock();};
  virtual ~lockgc_ptr(){if(ptr) ptr->unlock();ptr=nullptr;}
  T* operator->(){return (T*)ptr;}
  T* operator=(T* p){if(ptr) ptr->unlock();ptr=p;if(p) p->lock();return p;}
  lockgc_ptr& operator=(const lockgc_ptr& p){if(ptr) ptr->unlock();ptr=p.ptr;if (ptr) ptr->lock(); return *this;}
  operator T* const &() const {return ptr;}
  //operator T*(){return ptr;}
};

template <class T> class gc_array_ : public GCObject {
  vector<T*>& data;
  bool stop;
public:	
  explicit gc_array_(vector<T*>& d):data{d}{lock();stop=false;}
  virtual int childCnt() override {return (stop?0:data.size());}
  virtual int unlock() override {
    locked--;
    stop=(locked==0);
    //cout << "gc array locked:"<< locked << " stop:" << stop << endl; 
    return locked;
  }
  virtual T* getChild(int p) override {return data[p];}
  virtual string print() const override {return "array locker";}
};

/*
template <class T1,class T2> class gc_dict_ : public GCObject {
protected:	
  unordered_map<T1,T2*>& data;
  virtual void mark() override {
	if (!marked) {
	  marked=true;
	  for(auto&[k,o]:data) 
	    o->mark();
	 };
  }
  virtual void expand(int gen) override {
	if (generation<gen) generation=gen; 
	for(auto& [k,o]:data){
	  if(o->generation<generation) 
	    o->expand(generation);
	 }
  }
public:
  explicit gc_dict_(unordered_map<T1,T2*>& d):data{d}{}

};
*/
