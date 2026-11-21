#pragma once

#include <string>
#include <vector>

using namespace std;

#include "gc.h"

using namespace gc;

#include "cplobject.h"

template <class T> class recycler {
  vector<T*> objs;
public:
  int recycled=0;
  virtual ~recycler(){for(auto o:objs) delete o;}
  void add(T* o){objs.push_back(o);}  
  T* get(){T* r=objs.back();objs.pop_back();recycled++;return r;}
  int size(){return objs.size();}
  T* recycle(){return (size()>0)?get():nullptr;}
};
