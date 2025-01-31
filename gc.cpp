
#include <unordered_set>

class GCObj {
  int  gen;      // si potrebbe fare che -1 è lock
  bool marked;
public:
  GCObj();
  virtual ~GCObj(){}
  virtual void mark(){marked=true;}
  void lock(){gen=-1;}
  void unlock(){gen=0;};
  void reclaim(){delete this;}
  friend class GC;
};

class GC {
  unordered_set<GCObj*> objs;
  int cnt;
public:
  GC():cnt{0}{}
  void mark();
  void sweep();
  void add(GCObj* o){if(cnt++>100000) collect();objs.insert(o);}
  void collect(){mark();sweep();cnt=0;}
};

GC theGC;

GCObj::GCObj():gen{0},marked{false}{
	theGC.add(this);
}

void GC::mark(){
  for(const auto& o:objs)
    o->marked=false;
  for(auto o:objs){
	  if (o->gen==-1)
	    o->mark();
  }
}

void GC::sweep(){
  /*
  for(auto it=objs.begin();it!=objs.end();){
    if (!(*it)->marked){
      (*it)->reclaim();
      it=objs.erase(it);
    } else {
      ++it;
    }
  }
  */
  int cnt=0;
  for(const auto& o:objs)
    if (o->marked) 
      cnt++;
  cout << "marked:" << cnt << endl;    
}
