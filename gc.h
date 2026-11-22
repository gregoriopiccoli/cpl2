// Variante 1: mark-and-sweep semplice, senza generazioni. È il riferimento per tutti i confronti.
// Ogni collect: (1) azzera i segni e mette in pila gli oggetti bloccati, (2) marca con una pila esplicita,
// (3) sweep compattando i sopravvissuti nello stesso vettore.
#pragma once
#include "gccommon.h"

#include <iostream>

#define GC_DEBUG

namespace gc {

class GC;
class Tracer;

#ifdef GC_DEBUG
enum: uint32_t { MAGIC_LIVE=0x600DC0DEu, MAGIC_DEAD=0xDEAD600Du };
#endif

// ogni oggetto gestito deriva da questa classe e va creato con new (mai delete a mano)
class GCObject {
  bool marked=false;
  int locked=0;
  friend class GC;
  friend class Tracer;
#ifdef GC_DEBUG
  uint32_t magic=0;     // MAGIC_LIVE da add() in poi, MAGIC_DEAD quando liberato
#endif
protected:
  // deve chiamare t(p) per ogni puntatore a oggetti gestiti contenuto nell'oggetto (nullptr ammessi).
  // Default: nessun figlio. Dentro trace non si devono creare oggetti gestiti né modificare la struttura.
  virtual void trace(Tracer&){}
  // chiamata dallo sweep per liberare l'oggetto; ridefinibile per pool/riciclo
  virtual void reclaim(){
#ifdef GC_DEBUG
    checkLive();
    magic=MAGIC_DEAD;
#endif
    delete this;
  }
public:
  GCObject();
  // la copia di un oggetto è un oggetto nuovo: viene registrata e non eredita i lock dell'originale
  GCObject(const GCObject&):GCObject(){}
  GCObject& operator=(const GCObject&){return *this;}
  virtual ~GCObject()=default;
  //
  int lock(){
#ifdef GC_DEBUG
    checkLive();
#endif
    return ++locked;
  }
  int unlock(){
#ifdef GC_DEBUG
    checkLive();
#endif
    assert(locked>0);
    return --locked;
  }
  int lockCnt() const {return locked;}
#ifdef GC_DEBUG
  void checkLive(){assert(magic==MAGIC_LIVE);}
  void kill(){magic=MAGIC_DEAD;}
  void resuscitate(){magic=MAGIC_LIVE;}
#endif
  virtual string print() const {return "???";}
};

// passato a trace(): marca il figlio e lo mette sulla pila del GC (niente ricorsione)
class Tracer {
  std::vector<GCObject*>& work; // referenza al vettore che contiene gli oggetti da esplorare
  explicit Tracer(std::vector<GCObject*>& w):work(w){}
  friend class GC;
public:
  Tracer(const Tracer&)=delete;
  Tracer& operator=(const Tracer&)=delete;
  void operator()(GCObject* c){
#ifdef GC_DEBUG
    if (c) c->checkLive();
#endif
    if (c && !c->marked){
      c->marked=true;    // segna l'oggetto come marcato
      work.push_back(c); // mette i suoi figli tra gli oggetti da marcare (versione non ricorsiva)
    }
  }
};

class GC {
  std::vector<GCObject*> objs; // tutti gli oggetti gestiti
  std::vector<GCObject*> work; // pila della marcatura
  Pacer pacer;
  GCStats st;
  int paused=0;
  bool collecting=false;

  void mark(){
    // un solo passaggio: i bloccati sono le radici, tutti gli altri partono non marcati
    for(GCObject* o:objs){
      o->marked=o->locked>0;
      if(o->marked) work.push_back(o); // aggiunge agli oggetta da esplorare tutte le radici bloccate
    }
    // ora cicla nella lista degli oggetta da esplorare
    Tracer t(work);
    while(!work.empty()){
      GCObject* p=work.back();
      work.pop_back();
      st.traces++;
      p->trace(t);
    }
  }
  void sweep(){
    // compatta i sopravvissuti all'inizio del vettore, senza allocare
    size_t j=0,n=objs.size();
    for(size_t i=0;i<n;i++){
      GCObject* o=objs[i];
      if(o->marked)
        objs[j++]=o;
      else {
        o->reclaim();
        st.freed++;
      }
    }
    objs.resize(j);
  }
public:
  static GC& get(){static GC g;return g;}
  static const char* name(){return "simple";}
  GC()=default;
  GC(const GC&)=delete;
  GC& operator=(const GC&)=delete;
  ~GC(){ // alla chiusura reclama tutti gli oggetti ancora registrati, così vengono chiamati i distruttori
    collecting=true;
    for(GCObject* o:objs) o->reclaim();
    objs.clear();
  }
  void add(GCObject* o){
    assert(!collecting && "nessun oggetto gestito va creato dentro trace() o in un distruttore");
    if(paused==0 && pacer.due()) collect();
    objs.push_back(o);
    pacer.added++;
    st.allocated++;
    if(objs.size()>st.peakObjects) st.peakObjects=objs.size();
  }
  void collect(int i=0){
    PauseTimer pt(st);
    st.fullCollections++;
    collecting=true;
    mark();
    sweep();
    collecting=false;
    pacer.afterCollect(objs.size());
  }
  void collectFull(){collect();}
  void pause(){paused++;}
  void resume(){assert(paused>0);paused--;}
  int setGCPercent(int p){int old=pacer.percent;pacer.percent=p;pacer.updateLimit(objs.size());return old;}
  size_t setMinObjs(size_t m){size_t old=pacer.minObjs;pacer.minObjs=m<1?1:m;pacer.updateLimit(objs.size());return old;}
  size_t size() const {return objs.size();}
  const GCStats& stats() const {
#ifdef GC_DEBUG
    std::cout << "-- stats-- live:" << objs.size() << " peak:" << st.peakObjects << " allocated:" << st.allocated << " freed:" << st.freed  << " limit:" << pacer.limit << std::endl;
#endif
    return st;
  }
  void resetStats(){st=GCStats{};st.peakObjects=objs.size();}
  void addRecycled(GCObject* o){add(o); /* alternativa ... objs.push_back(o); */}    
  void printLocked(){
	for(const GCObject* const& o:objs) 
	  if (o->locked>0)
	    cout << o->print() << endl;
  }


};

inline GCObject::GCObject(){
#ifdef GC_DEBUG
      magic=MAGIC_LIVE;
#endif
  GC::get().add(this);
}

inline GC& stdGC(){return GC::get();}

//// radici: puntatore da usare nel programma (non dentro gli oggetti gestiti) che tiene bloccato l'oggetto

template <class T> // requires std::derived_from<T, GCObject>
class lockgc_ptr {
  T* ptr=nullptr;
  void set(T* p){ // blocca il nuovo prima di sbloccare il vecchio: funziona anche con p==ptr
    if(p) p->lock();
    T* old=ptr;
    ptr=p;
    if(old) old->unlock();
  }
public:
  lockgc_ptr(){}
  lockgc_ptr(T* p){set(p);}
  lockgc_ptr(const lockgc_ptr& o){set(o.ptr);}                   // la copia aggiunge un lock
  lockgc_ptr(lockgc_ptr&& o) noexcept {ptr=o.ptr;o.ptr=nullptr;} // lo spostamento trasferisce il lock
  ~lockgc_ptr(){set(nullptr);}
  lockgc_ptr& operator=(T* p){set(p);return *this;}
  lockgc_ptr& operator=(const lockgc_ptr& o){set(o.ptr);return *this;}
  lockgc_ptr& operator=(lockgc_ptr&& o) noexcept {
    if(this!=&o){T* old=ptr;ptr=o.ptr;o.ptr=nullptr;if(old) old->unlock();}
    return *this;
  }
  T* get() const {return ptr;}
  T* operator->() const {
#ifdef GC_DEBUG
    ptr->checkLive();
#endif
    return ptr;
  }
  operator T*() const {return ptr;}
};

//// campi che si registrano da soli: GCAuto + gc_ptr (

class GCAuto;

class gc_ptr_base {
protected:
  GCObject* ptr=nullptr;
  GCObject* owner; // oggetto che contiene il campo: serve alla write barrier
  explicit gc_ptr_base(GCObject* o):owner(o){}
  friend class GCAuto;
};

// oggetto i cui figli sono i suoi membri gc_ptr: non serve scrivere trace()
class GCAuto: public GCObject {
  std::vector<gc_ptr_base*> fields;
protected:
  void trace(Tracer& t) override {for(gc_ptr_base* f:fields) t(f->ptr);};
public:
  GCAuto()=default;
  GCAuto(const GCAuto&)=delete; // la copia avrebbe i campi registrati nell'originale
  GCAuto& operator=(const GCAuto&)=delete;
  void registerField(gc_ptr_base* f){fields.push_back(f);}
};

// da usare solo come membro di una classe derivata da GCAuto: gc_ptr<T> campo{this};
template <class T> class gc_ptr : public gc_ptr_base {
public:
  explicit gc_ptr(GCAuto* o):gc_ptr_base(o){o->registerField(this);}
  gc_ptr(const gc_ptr&)=delete; // un gc_ptr esiste solo registrato nel suo oggetto
  gc_ptr& operator=(const gc_ptr& o){return *this=o.get();}
  gc_ptr& operator=(T* p){ptr=p;return *this;}
  T* get() const {return static_cast<T*>(ptr);}
  T* operator->() const {return get();}
  operator T*() const {return get();}
};

#define GC_ptr(cl,name) gc_ptr<cl> name{gc_ptr<cl>(this)}

// un sistema per bloccare un array di obj*
template <class T> class gc_array_ : public GCObject {
  vector<T*>& data;
public:	
  explicit gc_array_(vector<T*>& d):data{d}{lock();}
  void trace(Tracer& t) override {for(auto& a:data)t(a);}
};

} // namespace gc

