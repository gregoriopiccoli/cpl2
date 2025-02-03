/*
Dove possibile usare sempre referenze!

DA FARE: 
  GC parallelo?
  fare referenza a tipo di base anche per le stringhe e i float
  Blocco dei parametri da fare una volta sola, FATTO ora però devo fare i blocchi di codice di inizializzazione FATTO e poi devo usarlo!
  La print di una procedura/funzione dovrebbe descriverla completamente
  Float e operazioni miste tra int e float
   
FATTO: 
  Fatto il Garbage Collector! 
  Provato a riciclare gli interi ... ci si mette più tempo! Dovrò provare con la garbage collection. OTTIMO!
  Nil è meglio con nullptr o con uno specifico oggetto nil? per ora provo con un oggetto specifico così non è mai un puntatore non inizializzato ...
  Devo fare che le stringhe abbiano le loro operazioni ... prima prova di classe C++!
  fare trucco della compare per tipo e mettere le operazioni di confronto sulla compare
*/

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
//#include <deque>
//#include <memory>

using namespace std;

void showIntCache();

#include "gc.cpp"

// --- gestione della lista delle stringhe che fanno da identificatori, vengono poste in una lista e viene assegnato un numero progressivo

class StringIntern {
   unordered_map<string,int> map;  // mappa di interning delle stringhe
   vector<string> imap;            // mappa inversa per riavere il valore rapidamente (si può migliorare, così salva due volte le stringhe)
   int n=0;
public:
   int add(string s){
	 auto ff=map.find(s);  
     if (ff!=map.end()) return ff->second;
	 map[s]=++n;
	 imap.push_back(s);
	 return n;
   }
   string get(int k){
	 return imap[k-1];
   }
};

// --- il singleton che gestisce la lista degli identificatori
StringIntern theStringIntern;

// --- il pcode

#include "pcodes.h"
#include "pcodes.c"

class interp;

class pcode {
protected:
  int code;
public:
  pcode():code{0}{}
  virtual ~pcode(){}
  virtual void exec(interp& interpreter) const {throw domain_error("not an executable pcode");};
  int getCode() const {return code;}
  virtual int getIntValue(){return 0;}
  virtual string print() const {return pcodetxt[code];}
};

class ipcode: public pcode {
protected:
  int value;
public:
  explicit ipcode(int i):value{i}{}
  virtual int getIntValue() override {return value;}
  virtual string print() const override {return string(pcodetxt[code])+" "+to_string(value);}
};

class interningpcode: public pcode {
protected:
  int value;
public:
  explicit interningpcode(const string& s):value{theStringIntern.add(s)}{}
  virtual int getIntValue() override {return value;}
  virtual string print() const override {return string(pcodetxt[code])+" "+theStringIntern.get(value);}
};

class spcode: public pcode {
protected:
  string value;
public:
  explicit spcode(const string& s):value{s}{}
  virtual string print() const override {return string(pcodetxt[code])+" "+value;}
};

class pcodePlus: public pcode {
public:
  pcodePlus(){code=P_PLUS;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeMinus: public pcode {
public:
  pcodeMinus(){code=P_MINUS;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeUMinus: public pcode {
public:
  pcodeUMinus(){code=P_UMINUS;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeMult: public pcode {
public:
  pcodeMult(){code=P_MULT;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeDiv: public pcode {
public:
  pcodeDiv(){code=P_DIV;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeMod: public pcode {
public:
  pcodeMod(){code=P_MOD;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeIDiv: public pcode {
public:
  pcodeIDiv(){code=P_IDIV;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeEq: public pcode {
public:
  pcodeEq(){code=P_EQ;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeLt: public pcode {
public:
  pcodeLt(){code=P_LT;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeLe: public pcode {
public:
  pcodeLe(){code=P_LE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeGe: public pcode {
public:
  pcodeGe(){code=P_GE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeGt: public pcode {
public:
  pcodeGt(){code=P_GT;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeNe: public pcode {
public:
  pcodeNe(){code=P_NE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodePop: public pcode {
public:
  pcodePop(){code=P_POP;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeNil: public pcode {
public:
  pcodeNil(){code=P_NIL;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeTrue: public pcode {
public:
  pcodeTrue(){code=P_TRUE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeFalse: public pcode {
public:
  pcodeFalse(){code=P_FALSE;}
  virtual void exec(interp& interpreter) const override;
};

class obj;

class pcodeIntConst: public ipcode {
  lockgc_ptr<obj> theValue;	
public:
  explicit pcodeIntConst(int v);
  ~pcodeIntConst() override {theValue=nullptr;}
  virtual void exec(interp& interpreter) const override;
  //obj* getIntConst() const {return theValue;}
};

class pcodeStrConst: public spcode {
  lockgc_ptr<obj> theValue;	
public:
  explicit pcodeStrConst(const string& v);
  ~pcodeStrConst() override {theValue=nullptr;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeArray: public ipcode {
public:
  explicit pcodeArray(int v):ipcode(v){code=P_ARRAY;}
  virtual void exec(interp& interpreter) const override;
};
	
class pcodeDict: public ipcode {
public:
  explicit pcodeDict(int v):ipcode(v){code=P_DICT;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeVar: public interningpcode {
public:
  explicit pcodeVar(const string& v):interningpcode(v){code=P_VAR;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeVarStore: public interningpcode {
public:
  explicit pcodeVarStore(const string& v):interningpcode(v){code=P_VAR;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeLoad: public interningpcode {
public:
  explicit pcodeLoad(const string& v):interningpcode(v){code=P_LOAD;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeSlice: public pcode {
public:
  explicit pcodeSlice(){code=P_SLICE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeStore: public interningpcode {
public:
  explicit pcodeStore(const string& v):interningpcode(v){code=P_STORE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeGoto: public ipcode {
public:
  explicit pcodeGoto(int v):ipcode(v){code=P_GOTO;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeLabel: public ipcode {
public:
  explicit pcodeLabel(int v):ipcode(v){code=P_LABEL;}
  virtual void exec(interp& interpreter) const override {}; // il pcode Label in esecuzione è una NOP
};

class pcodeCall: public ipcode {
public:
  explicit pcodeCall(int v):ipcode(v){code=P_CALL;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeParm: public interningpcode {
public:
  explicit pcodeParm(const string& v):interningpcode(v){code=P_PARM;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeIfFalse: public ipcode {
public:
  explicit pcodeIfFalse(int v):ipcode(v){code=P_IF_FALSE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeIfAnd: public ipcode {
public:
  explicit pcodeIfAnd(int v):ipcode(v){code=P_IF_AND;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeIfOr: public ipcode {
public:
  explicit pcodeIfOr(int v):ipcode(v){code=P_IF_OR;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeNot: public pcode {
public:
  pcodeNot(){code=P_NOT;}
  virtual void exec(interp& interpreter) const override;
};

class pcodePrint: public ipcode {
public:
  explicit pcodePrint(int v):ipcode(v){code=P_PRINT;}
  virtual void exec(interp& interpreter) const override;
};

class pcodePCodeEnd: public ipcode {
public:
  explicit pcodePCodeEnd(int v):ipcode(v){code=P_PCODEEND;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeNotImpl: public spcode {
public:
  explicit pcodeNotImpl(int c,const string& v):spcode(v){code=c;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeAnyType: public pcode {
public:
  explicit pcodeAnyType(){code=P_ANY_TYPE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeIntType: public pcode {
public:
  explicit pcodeIntType(){code=P_INT_TYPE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeStrType: public pcode {
public:
  explicit pcodeStrType(){code=P_STR_TYPE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeLine: public ipcode {
public:
  explicit pcodeLine(int v):ipcode(v){code=P_LINE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeProc: public interningpcode {
public:
  explicit pcodeProc(const string& v):interningpcode(v){code=P_PROC;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeFunc: public interningpcode {
public:
  explicit pcodeFunc(const string& v):interningpcode(v){code=P_FUNC;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeEndParm: public pcode {
public:
  pcodeEndParm(){code=P_ENDPARM;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeEndProc: public pcode {
public:
  pcodeEndProc(){code=P_ENDPROC;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeEndFunc: public pcode {
public:
  pcodeEndFunc(){code=P_ENDFUNC;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeStoreResult: public pcode {
public:
  pcodeStoreResult(){code=P_STORE_RESULT;}
  virtual void exec(interp& interpreter) const override;
};
	
pcode* makePCode(int c,const char* s){
  switch(c){
	case P_PLUS:return new pcodePlus();
	case P_MINUS:return new pcodeMinus();
	case P_UMINUS:return new pcodeUMinus();
	case P_MULT:return new pcodeMult();
	case P_DIV:return new pcodeDiv();
	case P_MOD:return new pcodeMod();
	case P_IDIV:return new pcodeIDiv();
	case P_EQ:return new pcodeEq();
	case P_LT:return new pcodeLt();
	case P_LE:return new pcodeLe();
	case P_GE:return new pcodeGe();
	case P_GT:return new pcodeGt();
	case P_NE:return new pcodeNe();
	case P_POP:return new pcodePop();
	case P_INT_CONST:return new pcodeIntConst(atoi(s));
	case P_STR_CONST:return new pcodeStrConst(s);
	case P_NIL:return new pcodeNil();
	case P_TRUE:return new pcodeTrue();
	case P_FALSE:return new pcodeFalse();
	case P_ARRAY:return new pcodeArray(atoi(s));
	case P_DICT:return new pcodeDict(atoi(s));
	case P_VAR:return new pcodeVar(s);
	case P_VAR_STORE:return new pcodeVarStore(s);
	case P_LOAD:return new pcodeLoad(s);
	case P_SLICE:return new pcodeSlice();
	case P_STORE:return new pcodeStore(s);
	case P_GOTO:return new pcodeGoto(atoi(s));
	case P_LABEL:return new pcodeLabel(atoi(s));
	case P_CALL:return new pcodeCall(atoi(s));
	case P_PARM:return new pcodeParm(s);
	case P_ENDPARM:return new pcodeEndParm();
	case P_PROC:return new pcodeProc(s);
	case P_ENDPROC:return new pcodeEndProc();
	case P_FUNC:return new pcodeFunc(s);
	case P_ENDFUNC:return new pcodeEndFunc();
	case P_IF_FALSE:return new pcodeIfFalse(atoi(s));
	case P_IF_AND:return new pcodeIfAnd(atoi(s));
	case P_IF_OR:return new pcodeIfOr(atoi(s));
	case P_NOT:return new pcodeNot();
	case P_PRINT:return new pcodePrint(atoi(s));
	case P_PCODEEND:return new pcodePCodeEnd(atoi(s));
	case P_ANY_TYPE:return new pcodeAnyType();
	case P_INT_TYPE:return new pcodeIntType();
	case P_STR_TYPE:return new pcodeStrType();
	case P_LINE:return new pcodeLine(atoi(s));
	case P_STORE_RESULT:return new pcodeStoreResult();
  }
  return new pcodeNotImpl(c,s);
}

// --- gli oggetti dell'esecuzione dell'interprete

//static int theObjCounter=0;

class obj : public GCObject {
public:
  //obj() {theObjCounter++;}
  //virtual ~obj(){}; //{theObjCounter--; if (theObjCounter==0) cout << "no more objs ...\n";}
  virtual void mark() override {marked=true;}
  virtual void expand(int gen) override {if (generation<gen) generation=gen;}
  //  
  virtual string print() const override {throw domain_error("print not implemented");}
  virtual obj* load(int intern) {throw domain_error("load not implemented");}
  virtual obj* slice(const obj* idx) {throw domain_error("slice not implemented");}
  virtual void store(int intern, obj* value) {throw domain_error("store not implemented");}
  virtual void storeslice(obj* idx,obj* value) {throw domain_error("storeslice not implemented");}
  virtual void call(int parmCnt) {throw domain_error("call (without interpreter) not implemented");}
  virtual void call(int parmCnt,interp& interpreter){throw domain_error("call (with interpreter) not implemented");}
  //
  virtual void store_result(obj* value) {throw domain_error("store result not implemented");}
  //
  virtual obj* plus(const obj*) const {throw domain_error("plus not implemented");}
  virtual obj* minus(const obj*) const {throw domain_error("minus not implemented");}
  virtual obj* uminus() const {throw domain_error("uminus not implemented");}
  virtual obj* mult(const obj*) const {throw domain_error("mult not implemented");}
  virtual obj* div(const obj*) const {throw domain_error("div not implemented");}
  virtual obj* mod(const obj*) const {throw domain_error("mod not implemented");}
  virtual obj* idiv(const obj*) const {throw domain_error("idiv not implemented");}
  //
  virtual obj* lt(const obj*) const {throw domain_error("lt not implemented");}
  virtual obj* le(const obj*) const {throw domain_error("le not implemented");}
  virtual obj* eq(const obj*) const {throw domain_error("eq not implemented");}
  virtual obj* ge(const obj*) const {throw domain_error("ge not implemented");}
  virtual obj* gt(const obj*) const {throw domain_error("gt not implemented");}
  virtual obj* ne(const obj*) const {throw domain_error("ne not implemented");}
  //
  //virtual shared_ptr<obj> _or() {throw domain_error("or not implemented");}
  //virtual shared_ptr<obj> _and() {throw domain_error("and not implemented");}
  virtual obj* _not() {throw domain_error("not not implemented");}
  virtual obj* is(const obj*) {throw domain_error("is not implemented");}
  //
  virtual const obj* getBaseType() const;
};

class nilObj: public obj {
public:
  virtual string print() const override {return "nil";}
  virtual obj* eq(const obj*) const override;
};

class intObj:public obj {
public:
  int value;
  explicit intObj(int v){value=v;}
  virtual string print() const override {return to_string(value);}
  virtual obj* plus(const obj* o) const override;
  virtual obj* minus(const obj* o) const override;
  virtual obj* mult(const obj* o) const override;
  virtual obj* div(const obj* o) const override;
  virtual obj* idiv(const obj* o) const override;
  virtual obj* mod(const obj* o) const override;
  virtual obj* eq(const obj* o) const override;
  virtual obj* lt(const obj* o) const override;
  virtual obj* le(const obj* o) const override;
  virtual obj* ge(const obj* o) const override;
  virtual obj* gt(const obj* o) const override;
  virtual obj* ne(const obj* o) const override;
  //
  virtual const obj* getBaseType() const override;
  //
  virtual bool reclaim() override;
  //
  //const intObj* check_int(const obj* o,const char* msg) const {const intObj* oo=dynamic_cast<const intObj*>(o);if (oo==nullptr) throw domain_error(msg);return oo;}
  const intObj* check_int(const obj* o,const char* msg) const;
};

class intCache {
  vector<intObj*> ints;
public:
  ~intCache(){for(auto o:ints) delete o;}  
  void add(intObj* i){ints.push_back(i);}
  intObj* get(){intObj* r=ints.back();ints.pop_back();return r;}
  int size(){return ints.size();}
};
	
intCache theIntCache;

void showIntCache(){cout << "intCache size:" << theIntCache.size() << endl;}

bool intObj::reclaim(){
  generation=0;
  theIntCache.add(this);
  //cout << intcache.size() << endl;
  return false;
}

pcodeIntConst::pcodeIntConst(int v):ipcode(v),theValue{new intObj(v)}{code=P_INT_CONST;}

obj* intObj::plus(const obj* o) const {
  const intObj* oo=check_int(o,"integer + with a non integer");
  if (theIntCache.size()>0) {intObj* v=theIntCache.get();v->value=value+oo->value;stdGC().addRecycled(v);return v;}
  return new intObj(value+oo->value);
}

obj* intObj::minus(const obj* o) const {
  const intObj* oo=check_int(o,"integer - with a non integer");
  if (theIntCache.size()>0) {intObj* v=theIntCache.get();v->value=value-oo->value;stdGC().addRecycled(v);return v;}
  return new intObj(value-oo->value);
}

obj* intObj::mult(const obj* o) const {
  const intObj* oo=check_int(o,"integer * with a non integer");
  return new intObj(value*oo->value);
}

obj* intObj::div(const obj* o) const {
  const intObj* oo=check_int(o,"integer / with a non integer");
  return new intObj(value/oo->value);
}

obj* intObj::idiv(const obj* o) const {
  const intObj* oo=check_int(o,"integer idiv with a non integer");
  return new intObj(value/oo->value);
}

obj* intObj::mod(const obj* o) const {
  const intObj* oo=check_int(o,"integer % with a non integer");
  return new intObj(value % oo->value);
}

class strObj:public obj {
public:
  string value;
  explicit strObj(const string& v):value{v}{}
  virtual string print() const override {return value;}
  virtual obj* plus(const obj* o) const override;
  virtual obj* eq(const obj* o) const override;
  virtual obj* lt(const obj* o) const override;
  virtual obj* le(const obj* o) const override;
  virtual obj* ge(const obj* o) const override;
  virtual obj* gt(const obj* o) const override;
  virtual obj* ne(const obj* o) const override;
  //
   const strObj* check_str(const obj* o,const char* msg) const {const strObj* oo=dynamic_cast<const strObj*>(o);if (oo==nullptr) throw domain_error(msg);return oo;}
};

pcodeStrConst::pcodeStrConst(const string& v):spcode(v),theValue{new strObj(v)}{code=P_STR_CONST;}

obj* strObj::plus(const obj* o) const {
  const strObj* oo=check_str(o,"str + with a non str");
  return new strObj(value + oo->value);
}

class boolObj:public obj {
public:
  bool value;
  explicit boolObj(bool v){value=v;};
  virtual string print() const override {return (value?"true":"false");};
  virtual obj* eq(const obj* o) const override;
  virtual obj* ne(const obj* o) const override;
  //
  //const boolObj* check_bool(const obj* o,const char*  msg) const {const boolObj* oo=dynamic_cast<const boolObj*>(o);if (oo==nullptr) throw domain_error(msg);return oo;}
  const boolObj* check_bool(const obj* o,const char*  msg) const;
};

// i singleton degli oggetti che non richiedono tante copie ...
lockgc_ptr<obj> theTrue{new boolObj(true)};
lockgc_ptr<obj> theFalse(new boolObj(false));
lockgc_ptr<obj> theNil(new nilObj());

const boolObj* boolObj::check_bool(const obj* o,const char*  msg) const {
  if (o!=theTrue && o!=theFalse) throw domain_error(msg);
  return static_cast<const boolObj*>(o);
}

const obj* obj::getBaseType() const {return theNil;}

obj* nilObj::eq(const obj* o) const {
  return (o==theNil?theTrue:theFalse);
}

obj* intObj::eq(const obj* o) const {
  const intObj* oo=check_int(o,"int op = with a non int");
  return (value == oo->value?theTrue:theFalse);
}

obj* intObj::lt(const obj* o) const {
  const intObj* oo=check_int(o,"int op < with a non int");
  return (value < oo->value?theTrue:theFalse);
}

obj* intObj::le(const obj* o) const {
  const intObj* oo=check_int(o,"int op < with a non int");
  return (value <= oo->value?theTrue:theFalse);
}

obj* intObj::ge(const obj* o) const {
  const intObj* oo=check_int(o,"int op < with a non int");
  return (value >= oo->value?theTrue:theFalse);
}

obj* intObj::gt(const obj* o) const {
  const intObj* oo=check_int(o,"int op < with a non int");
  return (value > oo->value?theTrue:theFalse);
}

obj* intObj::ne(const obj* o) const {
  const intObj* oo=check_int(o,"int op < with a non int");
  return (value != oo->value?theTrue:theFalse);
}

obj* strObj::eq(const obj* o) const {
  const strObj* oo=check_str(o,"str = with a non str");
  return (value.compare(oo->value)==0 ? theTrue:theFalse);
}

obj* strObj::lt(const obj* o) const {
  const strObj* oo=check_str(o,"str < with a non str");
  return (value.compare(oo->value)==-1 ? theTrue:theFalse);
}

obj* strObj::le(const obj* o) const {
  const strObj* oo=check_str(o,"str <= with a non str");
  return (value.compare(oo->value)<=0 ? theTrue:theFalse);
}

obj* strObj::ge(const obj* o) const {
  const strObj* oo=check_str(o,"str >= with a non str");
  return (value.compare(oo->value)>=0 ? theTrue:theFalse);
}

obj* strObj::gt(const obj* o) const {
  const strObj* oo=check_str(o,"str > with a non str");
  return (value.compare(oo->value)==1 ? theTrue:theFalse);
}

obj* strObj::ne(const obj* o)  const {
  const strObj* oo=check_str(o,"str <> with a non str");
  return (value.compare(oo->value)!=0 ? theTrue:theFalse);
}

obj* boolObj::eq(const obj* o) const {
  const boolObj* oo=check_bool(o,"bool = with a non bool");
  return (value == oo->value ? theTrue : theFalse);
}

obj* boolObj::ne(const obj* o) const {
  const boolObj* oo=check_bool(o,"bool <> with a non bool");
  return (value != oo->value ? theTrue : theFalse);
}

class floatObj: public obj {
public:
    float value;
    explicit floatObj(float f){value=f;};
    virtual string print() const override {return to_string(value);}
};

// --- i tipi di base

class intType: public obj {
public:
    virtual string print() const override {return "int";}
};

class strType: public obj {
public:
    virtual string print() const override {return "str";}
};

class floatType: public obj {
public:
    virtual string print() const override {return "float";}
};

lockgc_ptr<obj> theIntType{new intType()};
lockgc_ptr<obj> theStrType{new strType()};
lockgc_ptr<obj> theFloatType{new floatType()};

const obj* intObj::getBaseType() const {return theIntType;}

const intObj* intObj::check_int(const obj* o,const char* msg) const {
  if (o->getBaseType()!=theIntType) throw domain_error(msg);
  return static_cast<const intObj*>(o);
}

// --- gli array e i dizionari

class arrayObj: public obj {
	vector<obj*> a;
public:
  arrayObj(){}
  explicit arrayObj(const int& sz){resize(sz);}
  //
  virtual void mark() override {if (!marked) {marked=true;for(const auto& o:a) o->mark();}}
  virtual void expand(int gen) override {if (generation<gen) generation=gen;for(const auto& o:a) {if (o->generation<gen) o->expand(gen);}}
  //
  virtual obj* slice(const obj* idx) override {
	    const intObj* pos=dynamic_cast<const intObj*>(idx);
	    if (pos!=nullptr) 
	      return a[pos->value];
	    throw domain_error("slicing an array with a non integer index");
	  };
  virtual void storeslice(obj* idx,obj* value) override {
	    const intObj* pos=dynamic_cast<intObj*>(idx);
	    if (pos!=nullptr) {
		  a[pos->value]=value;
		} else {
	      throw domain_error("storing in an array with a non integer index");
	    }
	  };
  virtual obj* append(obj* v){a.push_back(v);return v;};
  virtual obj* remove(int pos){obj* v=a[pos];a.erase(a.begin()+pos);return v;};
  virtual obj* insert(int pos,obj* v){a.insert(a.begin()+pos,v);return v;};
  virtual int len(){return a.size();};
  virtual string print() const override;
  //
  obj* sliceidx(int idx){return a[idx];}
  void storesliceidx(int idx, obj*& v){a[idx]=v;}
  //void append(const obj*& v){a.push_back(v);}
  void resize(int sz){int oldsz=a.size();a.resize(sz);for(int i=oldsz;i<sz;i++) a[i]=theNil;}
};

string arrayObj::print() const {
  string res="";
  if (a.size()>0) {
    for(const obj* o:a){
	  res+=","+o->print();
    }
    res+="]";
    res[0]='[';
  } else 
    res="[]";  
  return res;
}

class dictObj : public obj {
  unordered_map<string,obj*> map;
public:
  //
  virtual void mark() override {if (!marked) {marked=true;for(const auto& [k,o]:map) o->mark();}}
  virtual void expand(int gen) override {if (generation<gen) generation=gen;for(const auto& [k,o]:map) {if (o->generation<gen) o->expand(gen);}}  
  //
  virtual obj* slice(const obj* idx) override {
	 string key=idx->print();
     auto ff=map.find(key);
     if (ff!=map.end())
       return ff->second;
	 string err=key+": key not found in dict";
	 throw out_of_range(err);
  };
  virtual void storeslice(obj* idx, obj* value) override {
	 string key=idx->print(); 
	 map[key]=value;
  };
  virtual string print() const override;
  //
  obj* loadkey(const string& key){return map[key];}
  obj* storekey(const string& key, obj* value){map[key]=value;return value;}
};

string dictObj::print() const {
  string res="";
  if (map.size()>0){
    for(auto [key,o]:map){
	  res+=","+key+":"+o->print();
    }
    res+="}";
    res[0]='{';
  } else
    res="{}";  
  return res;
}

// --- i contenitori con nomi (classi, variabili locali, moduli ...)

class contextObj : public obj {
protected:
  contextObj* superlevel;                               // il contesto dove verranno cercate tutte le etichette non trovate in questo contesto
  unordered_map<int,obj*> objs;              
  unordered_map<int,obj*> types;
public:
  contextObj();                                         // se non viene specificato un superlevel il superlevel sarà built-in
  explicit contextObj(contextObj* sl):superlevel{sl}{}   // il costruttore che specifica quale è il contesto che fa da superlevel
  //
  virtual void mark() override {if (!marked) {marked=true;for(auto const& [k,o]:objs) o->mark();for(auto const& [k,o]:types) o->mark();}}
  virtual void expand(int gen) override {
	if (generation<gen) generation=gen;
	for(auto const& [k,o]:objs) {if (o->generation<gen) o->expand(gen);} 
	for(auto const& [k,o]:types) {if (o->generation<gen) o->expand(gen);}
  }
  //
  virtual void add(int intern, obj* type) {
	auto ff=objs.find(intern);
	if (ff!=objs.end()) throw out_of_range("name already in context");
	objs[intern]=theNil;
	types[intern]=type;
  }
  virtual void add(int intern, obj* type, obj* value) {
	auto ff=objs.find(intern);
	if (ff!=objs.end()) throw out_of_range("name already in context");
	objs[intern]=value;
	types[intern]=type;
  }
  virtual obj* load(int intern) override {
    auto ff=objs.find(intern);
    if (ff!=objs.end()) return ff->second;
    return superlevel->load(intern);
  }
  virtual void store(int intern, obj* value) override {
	auto ff=objs.find(intern);
	if (ff!=objs.end()){
	  ff->second=value;	
	} else {
	  superlevel->store(intern,value);
    }
  }
  virtual string print() const override {
	string s="container ";
	for(auto& [k,t]:types){
		s+=","+t->print()+" "+theStringIntern.get(k)+" ("+objs.at(k)->print()+")";
	}
	return s;
  }
};

// la classe che implementa i builtin: ferma la ricerca perché è sempre l'ultimo livello
class builtInContainer : public contextObj {
public:	
  virtual obj* load(int intern) override {
	auto ff=objs.find(intern);  
    if (ff!=objs.end()) 
      return ff->second;
	string err=theStringIntern.get(intern)+" name not found";
	cout << err << endl;
	throw out_of_range(err);
  }
  virtual void store(int intern, obj* value) override {
	auto ff=objs.find(intern);  
	if (ff!=objs.end()){
	  ff->second=value;
	} else {
	  string err=theStringIntern.get(intern)+" name not found";
	  cout << err << endl;
	  throw out_of_range(err);
	}
  }
  int adx(int intern, obj* type, obj* value){add(intern,type,value);return 1;} // funzione per caricare le procedure c++ nei builtin
};

lockgc_ptr<builtInContainer> theBuiltIn{new builtInContainer};

// il costruttore di un contesto che non specifica qual è il suo contesto di base riceve builtin come punto finale della ricerca
contextObj::contextObj():superlevel{theBuiltIn}{};

// --- contenitore che cerca in locale e nel modulo
class procContextObj : public contextObj {
};

// --- il sistema ---

class interp;

class pcodeProgram {
protected:	
  vector<pcode*> prg;    // i pcode che costituiscono il programma
  vector<int> labelPos;  // la posizione delle label nel pcode, per effettuare i salti
public:
  void add(pcode* p){
	prg.push_back(p);
	if (p->getCode()==P_LABEL) putLabelPos(p->getIntValue());
  }
  const pcode& get(int i){return *prg[i];}
  int getLabelPos(int l){return labelPos[l];}
  void clear(){for(auto p:prg) delete p;prg.clear();labelPos.clear();}
  virtual ~pcodeProgram(){clear();}
  int loadPcd(string fn);
  void putLabelPos(int id){
	if ((int)labelPos.size()<=id) labelPos.resize(id+1);
 	labelPos[id]=prg.size()-1;
  }
  const pcode& operator[](int i) const {return *prg[i];}
};

int pcodeProgram::loadPcd(string fn){
  ifstream pcd(fn);
  if (pcd.good()) {
	char line[2000];
	string header="#cpl20.0.1";
	prg.clear();
	unsigned char code=0;
	pcd.getline(line,2000);
	if (header.compare(line)!=0){
	  cout << "not a compiled cpl2 file ..." <<endl;
	  prg.push_back(new pcodePCodeEnd(0));
	  return 0;
	}
	pcd.getline(line,2000);
	while (!pcd.eof() && code!=P_PCODEEND) {
      code=((unsigned char)line[0])-31;
      //cout << line << endl;
      //cout << prg.size() << " " << pcodetxt[code] << "(" << (int)code << ") " << (char*)(line+1) << endl;
      pcode* p=makePCode(code,line+1);
      prg.push_back(p);
      // gestione delle label
      if (code==P_LABEL){
	    putLabelPos(atoi(line+1));
		//cout << "-- label:" << id << " pos:" << prg.size()-1 << endl;
	  }
	  pcd.getline(line,2000);
    }
    pcode* last=prg[prg.size()-1];
    if (last->getCode()!=P_PCODEEND || last->getIntValue()!=0){
	  prg.clear();
	  prg.push_back(new pcodePCodeEnd(0));
	  cout << fn << " has errors!" << endl;
	  return 0;
	}
    return 1;
  }
  prg.push_back(new pcodePCodeEnd(0));
  return 0;
};

class sys {
public:
  // i moduli caricati
  unordered_map<string,pcodeProgram*> modules;
  // dei valori che esistono sempre
};
sys theSys;

//#define PRINT_PCODE_EXECUTION

class interp {
public:
  vector<obj*> stack;
  int sp,pc,currentSourceLine;
  contextObj* context;
  pcodeProgram& prg;
  bool stop;
  gc_array_<obj>* stack_gc;
  //
  explicit interp(contextObj* c, pcodeProgram& p):context{c},prg{p},stack_gc{new gc_array_<obj>(stack)}{
	sp=-1;pc=0;stop=false;
	currentSourceLine=0;
	stack.reserve(20);
  }
  ~interp(){stack_gc->unlock();};
  //
  void run() {
	stop=false;  
	while(!stop){
#ifdef PRINT_PCODE_EXECUTION
	  //int instr=prg->get(pc)->getCode();
	  cout << "pc:" << pc << " sp:" << sp << " sz:" << stack.size() << " cap:" << stack.capacity() << " " << prg[pc].print() << endl;
	  //cout << "pc:" << pc << " sp:" << sp << " " << prg->get(pc)->getCode() << " " << prg->get(pc)->print() << endl;
#endif
//#define TESTSWITCH
#ifdef TESTSWITCH
      const pcode& ppp=prg[pc];
      switch (ppp.getCode()){
	    case P_INT_CONST:
          sp++;
          stack.push_back(reinterpret_cast<const pcodeIntConst&>(ppp).getIntConst());
		  break;
	    case P_PLUS:  
          stack[sp-1]=stack[sp-1]->plus(stack[sp]);
          sp--;
          stack.pop_back();
          break;
	    default:
		  ppp.exec(*this);
	  }
#else      
	  prg[pc].exec(*this);
#endif	  
	  pc++;
	}
  }
};

class cppFunc : public obj {
protected:
  string name;
public:  	
  virtual string print() const override {return "<"+name+": built-in function>";};
  void getParms(int n, interp& i){
	// da fare ...
	// ... lettura e pop dei parametri ...
	// alla fine toglie dallo stack l'oggetto della call  
	i.stack.pop_back();  
  };
};

#define BUILTIN(_fn_) class builtin_##_fn_ : public cppFunc { public: \
  builtin_##_fn_(){name=#_fn_;/*lock();*/} \
  virtual void call(int parmCnt,interp& interpreter) override { getParms(parmCnt,interpreter);		  
#define BUILTINEND(_fn_) }}; \
  int add_builtin_##_fn_=theBuiltIn->adx(theStringIntern.add(#_fn_),theNil,new builtin_##_fn_());

/*	Esempio di traduzione per le funzioni c++ aggiunte ai builtin 
class hello2 : public cppFunc {
public:	
  hello2(){name="hello2";lock();}
  virtual void call(int parmCnt,interp& interpreter) override {
	getParms(parmCnt,interpreter);  
	interpreter.stack.push_back(new strObj("hello bult-in!"));
  }
};
int add_hello_builtin=theBuiltIn->adx(theStringIntern.add("hello2"),theNil,new hello2());
*/

BUILTIN(hello)
  interpreter.stack.push_back(new strObj("hello bult-in!"));
BUILTINEND(hello)

// --- l'oggetto che implementa la procedura

class procVars : public contextObj {
public:	
  explicit procVars(contextObj* ctx):contextObj{ctx}{}
  virtual obj* getResult(){return theNil;}
};

class procParm : public contextObj  {
public:
  vector<int> prmOrder;
  void add(int intern, obj* type) override {
	contextObj::add(intern,type);
	prmOrder.push_back(intern);
  }
  virtual string print() const override {
	string s="";
	for(int i=0;i<(int)prmOrder.size();i++){
		s+=(i!=0?", ":"")+types.at(prmOrder[i])->print()+" "+theStringIntern.get(prmOrder[i]);
	}
	return s;
  }
  void initParms(procVars*& vars, interp& i, int parmCnt);
};

class procObj : public obj {
protected:
  int name,pc;
  contextObj* ctx;
  pcodeProgram& prg;
  procParm* prm;
public:
  procObj(int n, interp& i);
  ~procObj() override {/*prm->unlock();*/}
  //
  procObj*& operator=(procObj*&) = delete;
  procObj(procObj&) = delete;
  //
  virtual void mark() override {if (!marked) {prm->mark();ctx->mark();} obj::mark();}
  virtual void expand(int gen) override {
	if (prm->generation<gen) prm->expand(gen); 
	if (ctx->generation<gen) ctx->expand(gen);
	obj::expand(gen);
  }  
  //
  virtual string print() const override {return "<proc "+theStringIntern.get(name)+"("+prm->print()+") -- pcode>";}
  virtual void call(int parmCnt,interp& interpreter) override;
  virtual procVars* mkVars(){return new procVars(ctx);}
};

class funcVars: public procVars {
protected:	
  obj* result_type;
  obj* result_value;
public:	
  funcVars(contextObj* ctx,obj* t):procVars{ctx},result_type{t},result_value{theNil}{}
  //
  virtual void mark() override {if (!marked) {result_value->mark();result_type->mark();} procVars::mark();}
  virtual void expand(int gen) override {
	if (result_value->generation<gen) result_value->expand(gen);
	if (result_type->generation<gen) result_type->expand(gen);
	procVars::expand(gen);
  }  
  //
  virtual void store_result(obj* value) override {result_value=value;}
  virtual obj* getResult() override {return result_value;}
};

class funcObj: public procObj {
protected:
  obj* result_type;
public:  
  funcObj(int n, interp& i, obj*& t):procObj(n,i),result_type{t}{/*result_type->lock();*/}
  virtual string print() const override {return "<func "+result_type->print()+" "+theStringIntern.get(name)+"("+prm->print()+") -- pcode>";}
  virtual procVars* mkVars()override {return new funcVars(ctx,result_type);}
};

procObj::procObj(int n, interp& i):ctx{i.context},prg{i.prg}{
  prm=new procParm();
  name=n;
  //prg=&i.prg; // tiene un puntatore al codice
  // ora può creare il blocco dei parametri, così viene creata una volta la struttura che poi servirà a processare i parametri dallo stack  
  contextObj* c=i.context;
  i.context=prm;
  i.pc++;                   // si sposta dall'istruzione PROC, trova la lista dei parametri che termina con ENDPARM
  i.run();                  // eseguendo il codice della lista dei parametri riempie il contesto con nomi e tipi
  // 
  i.context=c;            // ripristina il contesto di esecuzione dell'interprete
  i.stop=false;
  //cout << "proc/func " << theStringIntern.get(name) << "(" << prm->print() << ")" << endl;
  pc=i.pc; // setta il punto di partenza della procedura: dopo il blocco dei parametri
};

void procObj:: call(int parmCnt, interp& interpreter) {
  // cout << this->print() << " parametri:" << prm->prmOrder.size() << " parametri passati:" << parmCnt << endl; 	
  // salva lo stato dell'interprete
  int retPc=interpreter.pc;                                 // il program counter attuale
  const pcodeProgram& retPrg=interpreter.prg;               // il programma che sta eseguendo l'interprete
  contextObj* retCtx=interpreter.context;  // il contesto attuale dell'interprete
  // setta l'interprete allo stato della procedura
  procVars* vars=mkVars();                        // crea il nuovo ambiente delle variabili che ha come contesto di base il modulo dove è stata definita la procedura 
  vars->lock();
  // ^--- func/proc shared_ptr<contextObj> vars=make_shared<contextObj>(*ctx); // crea il nuovo ambiente delle variabili che ha come contesto di base il modulo dove è stata definita la procedura
  prm->initParms(vars,interpreter,parmCnt);                  // crea le variabili dalla lista dei parametri
  // ora ha tolto dallo stack i parametri e sono state create delle variabili locali inizalizzate con i valori passati DA FARE: valori di default
  interpreter.context=vars;                                  // setta le variabili come nuovo contesto dell'interprete
  interpreter.prg=prg;                                       // setta come programma il pcode della procedura
  interpreter.pc=pc+1;                                       // mette il program counte alla posizione del codice della procedura
  // ora può interpretare i pcode della procedura
  interpreter.run();
  // mette un nil che è sempre il risultato di una procedura
  interpreter.stack.pop_back(); // toglie l'oggeto procedura che è nella cima dello stack
  interpreter.stack.push_back(vars->getResult());
  // ^--- func/proc interpreter.stack.push_back(theNil);
  // rimette a posto lo stato dell'interprete a prima della chiamata
  interpreter.pc=retPc;
  interpreter.prg=retPrg;
  interpreter.context=retCtx;
  interpreter.stop=false;
  vars->unlock();
}

void procParm::initParms(procVars*& vars, interp& i, int parmCnt){
  int sp=i.sp-parmCnt+1;
  int pnp=(int)prmOrder.size();
  if (pnp<parmCnt) throw domain_error("too many parameters calling ... ");
  for(int j=0;j<pnp;j++){
	int p=prmOrder[j];  
	//cout << "parm:" << theStringIntern.get(p) << " type:" << types[p]->print() << " value:" << (j<parmCnt?i.stack[sp+j]->print():"nil") << endl;
	vars->add(p,types[p],(j<parmCnt?i.stack[sp+j]:static_cast<obj*>(theNil)));
  }
  // sistema lo stack togliendo i valori dei parametri passati alla funzione 
  for(int j=0;j<parmCnt;j++)
    i.stack.pop_back();
  i.sp-=parmCnt;  
}
// --- riprendo i pcode

void pcodePlus::exec(interp& interpreter) const {
  const obj* obj2=interpreter.stack[interpreter.sp--];
  const obj* obj1=interpreter.stack[interpreter.sp];
  interpreter.stack[interpreter.sp]=obj1->plus(obj2);
  interpreter.stack.pop_back();
  //cout << interpreter.sp << " " << interpreter.stack.size() << ">";
}

void pcodeMinus::exec(interp& interpreter) const {
  const obj* obj2=interpreter.stack[interpreter.sp--];
  const obj* obj1=interpreter.stack[interpreter.sp];
  interpreter.stack[interpreter.sp]=obj1->minus(obj2);
  interpreter.stack.pop_back();
}

void pcodeUMinus::exec(interp& interpreter) const {
  interpreter.stack[interpreter.sp]=interpreter.stack[interpreter.sp]->uminus();
}

void pcodeMult::exec(interp& interpreter) const {
  const obj* obj2=interpreter.stack[interpreter.sp--];
  const obj* obj1=interpreter.stack[interpreter.sp];
  interpreter.stack[interpreter.sp]=obj1->mult(obj2);
  interpreter.stack.pop_back();
}

void pcodeDiv::exec(interp& interpreter) const {
  const obj* obj2=interpreter.stack[interpreter.sp--];
  const obj* obj1=interpreter.stack[interpreter.sp];
  interpreter.stack[interpreter.sp]=obj1->div(obj2);
  interpreter.stack.pop_back();
}

void pcodeMod::exec(interp& interpreter) const {
  const obj* obj2=interpreter.stack[interpreter.sp--];
  const obj* obj1=interpreter.stack[interpreter.sp];
  interpreter.stack[interpreter.sp]=obj1->mod(obj2);
  interpreter.stack.pop_back();
}

void pcodeIDiv::exec(interp& interpreter) const {
  const obj* obj2=interpreter.stack[interpreter.sp--];
  const obj* obj1=interpreter.stack[interpreter.sp];
  interpreter.stack[interpreter.sp]=obj1->idiv(obj2);
  interpreter.stack.pop_back();
}

void pcodeEq::exec(interp& interpreter) const {
  const obj* obj2=interpreter.stack[interpreter.sp--];
  const obj* obj1=interpreter.stack[interpreter.sp];
  interpreter.stack[interpreter.sp]=obj1->eq(obj2);
  interpreter.stack.pop_back();
}

void pcodeLt::exec(interp& interpreter) const {
  const obj* obj2=interpreter.stack[interpreter.sp--];
  const obj* obj1=interpreter.stack[interpreter.sp];
  interpreter.stack[interpreter.sp]=obj1->lt(obj2);
  interpreter.stack.pop_back();
}

void pcodeLe::exec(interp& interpreter) const {
  const obj* obj2=interpreter.stack[interpreter.sp--];
  const obj* obj1=interpreter.stack[interpreter.sp];
  interpreter.stack[interpreter.sp]=obj1->le(obj2);
  interpreter.stack.pop_back();
}


void pcodeGe::exec(interp& interpreter) const {
  const obj* obj2=interpreter.stack[interpreter.sp--];
  const obj* obj1=interpreter.stack[interpreter.sp];
  interpreter.stack[interpreter.sp]=obj1->ge(obj2);
  interpreter.stack.pop_back();
}

void pcodeGt::exec(interp& interpreter) const {
  const obj* obj2=interpreter.stack[interpreter.sp--];
  const obj* obj1=interpreter.stack[interpreter.sp];
  interpreter.stack[interpreter.sp]=obj1->gt(obj2);
  interpreter.stack.pop_back();
}

void pcodeNe::exec(interp& interpreter) const {
  const obj* obj2=interpreter.stack[interpreter.sp--];
  const obj* obj1=interpreter.stack[interpreter.sp];
  interpreter.stack[interpreter.sp]=obj1->ne(obj2);
  interpreter.stack.pop_back();
}

void pcodePop::exec(interp& interpreter) const {
  interpreter.sp--;
  interpreter.stack.pop_back();
}

void pcodeNil::exec(interp& interpreter) const {
  interpreter.sp++;
  interpreter.stack.push_back(theNil);
}

void pcodeTrue::exec(interp& interpreter) const {
  interpreter.sp++;
  interpreter.stack.push_back(theTrue);
};

void pcodeFalse::exec(interp& interpreter) const {
  interpreter.sp++;
  interpreter.stack.push_back(theFalse);
}

void pcodeIntConst::exec(interp& interpreter) const {
  interpreter.sp++;
  //interpreter.stack.push_back(new intObj(value));
  interpreter.stack.push_back(theValue);
}

void pcodeStrConst::exec(interp& interpreter) const {
  interpreter.sp++;
  //interpreter.stack.push_back(new strObj(value));
  interpreter.stack.push_back(theValue);
}

void pcodeArray::exec(interp& interpreter) const {
  arrayObj* a=new arrayObj(value);
  int sp=interpreter.sp-value+1;
  for(int i=0;i<value;i++){
	a->storesliceidx(i,interpreter.stack[sp+i]);
  }
  if (value>0){
    for(int i=sp+1;i<=interpreter.sp;i++)
      interpreter.stack.pop_back();
    interpreter.stack[sp]=a;  
  } else {
	interpreter.stack.push_back(a); 
  }
  interpreter.sp=sp;
}

void pcodeDict::exec(interp& interpreter) const {
  dictObj* d=new dictObj();
  int sp=interpreter.sp-value*2+1;
  for(int i=0;i<value;i++){
	d->storekey(interpreter.stack[sp+i*2]->print(),interpreter.stack[sp+i*2+1]);
  }
  if (value>0){
    for(int i=sp+1;i<=interpreter.sp;i++)
      interpreter.stack.pop_back();
    interpreter.stack[sp]=d;
  } else {
    interpreter.stack.push_back(d);
  }
  interpreter.sp=sp;
}

void pcodeVar::exec(interp& interpreter) const {
  interpreter.context->add(value,interpreter.stack[interpreter.sp]);
}

void pcodeVarStore::exec(interp& interpreter) const {
  interpreter.context->add(value,interpreter.stack[interpreter.sp-1],interpreter.stack[interpreter.sp]);
  interpreter.sp--;
  interpreter.stack.pop_back();
}

void pcodeLoad::exec(interp& interpreter) const {
  interpreter.sp++;
  interpreter.stack.push_back(interpreter.context->load(value));
}

void pcodeSlice::exec(interp& interpreter) const {
  const obj* idx=interpreter.stack[interpreter.sp--];
  obj* cnt=interpreter.stack[interpreter.sp];
  //cout << "cnt:" << cnt->print() << " idx:" << idx->print() << endl;
  interpreter.stack[interpreter.sp]=cnt->slice(idx);
  interpreter.stack.pop_back();
}

void pcodeStore::exec(interp& interpreter) const {
  interpreter.context->store(value,interpreter.stack[interpreter.sp]);
}

void pcodeGoto::exec(interp& interpreter) const {
  //cout << "GoTo " << value << " at " << interpreter->prg->getLabelPos(value) << endl;
  int p=interpreter.prg.getLabelPos(value);
  interpreter.pc=p;
}

void pcodeCall::exec(interp& interpreter) const {
  //cout << "call stack:" << interpreter.sp << " " << interpreter.stack.size() << endl;
  interpreter.stack[interpreter.sp-value]->call(value,interpreter);
}

void pcodeParm::exec(interp& interpreter) const {
  // aggiunge un parametro alla lista dei parametri che si sta formando
  //cout << "param value:" << value << " type:" << interpreter.stack[interpreter.sp]->print() << endl;
  interpreter.context->add(value,interpreter.stack[interpreter.sp]);
  interpreter.sp--;
  interpreter.stack.pop_back();
  // DA FARE: parametri con valore di default
}

void pcodeEndParm::exec(interp& interpreter) const {
  interpreter.stop=true;
}

void pcodeEndProc::exec(interp& interpreter) const {
  interpreter.stop=true;
}

void pcodeEndFunc::exec(interp& interpreter) const {
  interpreter.stop=true;
}

void pcodeIfFalse::exec(interp& interpreter) const {
  const obj* v=interpreter.stack[interpreter.sp];
  if (v!=theTrue && v!= theFalse) throw out_of_range("if with a non boolean expression");
  interpreter.sp--;
  interpreter.stack.pop_back();
  if (v==theFalse) interpreter.pc=interpreter.prg.getLabelPos(value);
}

void pcodeIfAnd::exec(interp& interpreter) const {
  const obj* v=interpreter.stack[interpreter.sp];
  if (v!=theTrue && v!= theFalse) throw out_of_range("and with a non boolean expression");
  if (v==theFalse)
    interpreter.pc=interpreter.prg.getLabelPos(value);
  else {
    interpreter.sp--;
	interpreter.stack.pop_back();
  }
}

void pcodeIfOr::exec(interp& interpreter) const {
  const obj* v=interpreter.stack[interpreter.sp];
  if (v!=theTrue && v!= theFalse) throw out_of_range("or with a non boolean expression");
  if (v==theTrue)
    interpreter.pc=interpreter.prg.getLabelPos(value);
  else {
  	interpreter.sp--;
	interpreter.stack.pop_back();
  }
}

void pcodeNot::exec(interp& interpreter) const {
  const obj* v=interpreter.stack[interpreter.sp];
  if (v!=theTrue && v!= theFalse) throw out_of_range("not with a non boolean expression");
  interpreter.stack[interpreter.sp]=(v==theTrue?theFalse:theTrue);
}

void pcodePrint::exec(interp& interpreter) const {
  int i;
  for(i=1;i<=value;i++){
    const obj* o=interpreter.stack[interpreter.sp-value+i];
	cout << o->print();
  }
  cout << endl;
  interpreter.sp-=value;
  for(i=1;i<=value;i++)
    interpreter.stack.pop_back();
}

void pcodePCodeEnd::exec(interp& interpreter) const {
  //cout << "stop" << endl;
  interpreter.stop=true;
}

void pcodeAnyType::exec(interp& interpreter) const {
  interpreter.sp++;
  interpreter.stack.push_back(theNil);
}

void pcodeIntType::exec(interp& interpreter) const {
  interpreter.sp++;
  interpreter.stack.push_back(theIntType);
}

void pcodeStrType::exec(interp& interpreter) const {
  interpreter.sp++;
  interpreter.stack.push_back(theStrType);
}

void pcodeLine::exec(interp& interpreter) const {
  //cout << "line:" << value << endl;
  interpreter.currentSourceLine=value;
}

void pcodeNotImpl::exec(interp& interpreter) const {
  cout << "pcode:" << code << " " << pcodetxt[code] << " value:" << value << " not impl!" << endl;
  throw domain_error("pcode not implemented");
}

void makeProcOrFunc(interp& interpreter, int value, obj*& p, obj* type){
  int pc;
  //cout << "proc:" << theStringIntern.get(value) << " pc:" << interpreter.pc <<endl;
  // --- aggiunge la procedura/funzione al contesto attuale
  // -- era: shared_ptr<obj> p(new procObj(value,interpreter));
  interpreter.context->add(value,type,p);
  // --- salta il codice della procedura
  pc=interpreter.pc;
  //cout << "cerca fine proc " << pc << endl;
  int c=interpreter.prg.get(pc++).getCode();
  while (c!=P_ENDPROC && c!=P_ENDFUNC)
	c=interpreter.prg.get(pc++).getCode();
  interpreter.pc=pc-1;
  //cout << "finita dichiarazione: " << p->print() << endl;
}

void pcodeProc::exec(interp& interpreter) const {
  obj* p=new procObj(value,interpreter);
  makeProcOrFunc(interpreter,value,p,theNil);
}

void pcodeFunc::exec(interp& interpreter) const {
  obj* t=interpreter.stack[interpreter.sp--];
  interpreter.stack.pop_back();t->lock();
  //cout << "declaring func " << t->print() << " " << theStringIntern.get(value) << "()" << endl;
  obj* f=new funcObj(value,interpreter,t);//f->lock();
  makeProcOrFunc(interpreter,value,f,t);
  t->unlock();
}

void pcodeStoreResult::exec(interp& interpreter) const {
  interpreter.context->store_result(interpreter.stack[interpreter.sp]);
}

// ------------------------------------------------
#define ANKERL_NANOBENCH_IMPLEMENT
#include "nanobench.h"

void test_cc(){
  pcodeProgram prg;	
  contextObj* ctx=new contextObj();
  interp intp(ctx,prg);
  theStringIntern.add("i");
  //
  prg.add(new pcodeIntType());     // 0: INT_TYPE
  prg.add(new pcodeVar("i"));      // 1: VAR i
  prg.add(new pcodePop());         // 2: POP
  prg.add(new pcodeIntConst(0));   // 3: INT_CONST 0
  prg.add(new pcodeStore("i"));    // 4: STORE i
  prg.add(new pcodePop());         // 5: POP 
  prg.add(new pcodeLabel(3));      // 6: LABEL 3
  prg.add(new pcodeLoad("i"));     // 7: LOAD i
  prg.add(new pcodeIntConst(1000000)); // 8: INT_CONST 1000000
  prg.add(new pcodeLt());          // 9: LT 
  prg.add(new pcodeIfFalse(4));    //10: IF_FALSE 4
  prg.add(new pcodeLoad("i"));     //11: LOAD i
  prg.add(new pcodeIntConst(1));   //12: INT_CONST 1
  prg.add(new pcodePlus());        //13: PLUS 
  prg.add(new pcodeStore("i"));    //14: STORE i
  prg.add(new pcodePop());         //15: POP 
  prg.add(new pcodeGoto(3));       //16: GOTO 3
  prg.add(new pcodeLabel(4));      //17: LABEL 4
  prg.add(new pcodeLoad("i"));
  prg.add(new pcodePrint(1));
  prg.add(new pcodePCodeEnd(9));
  
  //intObj* c_1(new intObj(1));
  //intObj* c_1000000(new intObj(1000000));
  lockgc_ptr<intObj> c_1(new intObj(1));
  lockgc_ptr<intObj> c_1000000(new intObj(1000000));
    
  // esecuzione
  prg[0].exec(intp);                                      // int type
  
  //obj* i;                              //prg.get(1)->exec(intp); // var i
  lockgc_ptr<obj> i;                   // ATTENZIONE: solo così sono sicuro di tenere i valori dal GC!
   
  intp.stack.pop_back();intp.sp--;     //prg.get(2)->exec(intp); // pop
  prg[3].exec(intp);                                      // int const 0
  i=intp.stack[intp.sp];               //prg.get(4)->exec(intp); // store i
  intp.stack.pop_back();intp.sp--;     //prg.get(5)->exec(intp); // pop
  label3:                                                     // label 3
  intp.stack.push_back(i);intp.sp++;   //prg.get(7)->exec(intp); // load i
  intp.stack.push_back(c_1000000); intp.sp++; //prg.get(8)->exec(intp);  // int const 1000000
  
  //prg[9].exec(intp);                                      // lt
  intp.stack[intp.sp-1]=intp.stack[intp.sp-1]->lt(intp.stack[intp.sp]);
  intp.stack.pop_back();
  intp.sp--;
  
  //if false 4
  bool t=intp.stack[intp.sp--]==theFalse;
  intp.stack.pop_back();
  if (t) goto label4;  
  
  intp.stack.push_back(i);intp.sp++;  //prg.get(11)->exec(intp); // load i
  intp.stack.push_back(c_1);intp.sp++;//prg.get(12)->exec(intp); // int const 1
  
  //prg[13].exec(intp);                                     // plus
  intp.stack[intp.sp-1]=intp.stack[intp.sp-1]->plus(intp.stack[intp.sp]);
  intp.stack.pop_back();
  intp.sp--;
  
  i=intp.stack[intp.sp];              //prg.get(14)->exec(intp); // store i;
  
  intp.stack.pop_back();intp.sp--;    //prg.get(15)->exec(intp); // pop
  goto label3;
  label4:   
  intp.stack.push_back(i);intp.sp++;  //prg[18].exec(intp);              
  prg[19].exec(intp);
}
  
void test(const string& fn){
  // prova reale ...
  pcodeProgram prg;
  int r=prg.loadPcd(fn);  
  lockgc_ptr<contextObj> ctx{new contextObj()};
  interp intp(ctx,prg);
  if (r) intp.run();
  assert(intp.sp==-1);
}

int bench(string fn){
  ankerl::nanobench::Bench().run("conta fino a 1000000", [&] {
	  test(fn);
  });
  return 0;
}

int bench_cc(){
  ankerl::nanobench::Bench().run("conta fino a 1000000 in cc", [&] {
	test_cc();
  });
  return 0;
}

void releaseSysObjs(){
  //cout << "--- removing system objects ---\n";   
  theNil=nullptr;
  theTrue=nullptr;
  theFalse=nullptr;
  theIntType=nullptr;
  theStrType=nullptr;
  theFloatType=nullptr;
  theBuiltIn=nullptr;
}

int main(){
  //bench("primo.pcd");
  //test("primo.pcd");
  //test("terzo.pcd");
  test("fib.pcd");
  //bench("fib.pcd");
  //bench_cc();
  //test_cc();
  
  
  cout << "--- status on exit ---\n";  
  stdGC().status();
  //cout << "--- collect 0 ---\n";  
  //stdGC().collect(0);
  //stdGC().status();
  //cout << "--- collect all ---\n";  
  releaseSysObjs();
  stdGC().collectall();
  stdGC().status();
  //cout << "--- status after destruction of system objects ---\n";   
  //stdGC().collectall();
  //stdGC().status();
  cout << "--- final status,locked objs ---\n";
  stdGC().printLocked();
  cout << "--- end ---\n";
}
