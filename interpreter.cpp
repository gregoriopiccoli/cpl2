/*
  Nil è meglio con nullptr o con uno specifico oggetto nil? per ora provo con un oggetto specifico così non è mai un puntatore non inizializzato ...
  Devo fare che le stringhe abbiano le loro operazioni ... prima prova di classe C++!
  fare trucco della compare per tipo e mettere le operazioni di confronto sulla compare
*/

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <deque>
#include <memory>

using namespace std;

// --- gestione della lista delle stringhe che fanno da identificatori, vengono poste in una lista e viene assegnato un numero progressivo

class StringIntern {
   unordered_map<string,int> map;  // mappa di interning delle stringhe
   vector<string> imap;            // mappa inversa per riavere il valore rapidamente (si può migliorare, così salva due volte le stringhe)
   int n=0;
public:
   int add(string s){
     if (map.contains(s)) return map[s];
	 map[s]=++n;
	 imap.push_back(s);
	 return n;
   }
   string get(int k){
     //if (imap.contains(k)) return imap[k];
	 //return "";
	 return imap[k-1];
   }
};

// --- il singleton che gestisce la lista degli identificatori
StringIntern theStringIntern;

// --- il pcode

class interp;

class pcode {
protected:
  int code;
public:
  pcode():code{0}{}
  virtual ~pcode(){}
  virtual void exec(interp& interpreter)=0; //{throw domain_error("not an executable pcode");};
  int getCode(){return code;}
  virtual int getIntValue(){return 0;}
};

class ipcode: public pcode {
protected:
  int value;
public:
  explicit ipcode(int i):value{i}{}
  virtual int getIntValue(){return value;}
};

class spcode: public pcode {
protected:
  string value;
public:
  explicit spcode(const string& s):value{s}{}
};

#include "pcodes.h"

class pcodePlus: public pcode {
public:
  pcodePlus(){code=P_PLUS;}
  virtual void exec(interp& interpreter) override;
};

class pcodeMinus: public pcode {
public:
  pcodeMinus(){code=P_MINUS;}
  virtual void exec(interp& interpreter) override;
};

class pcodeUMinus: public pcode {
public:
  pcodeUMinus(){code=P_UMINUS;}
  virtual void exec(interp& interpreter) override;
};

class pcodeMult: public pcode {
public:
  pcodeMult(){code=P_MULT;}
  virtual void exec(interp& interpreter) override;
};

class pcodeDiv: public pcode {
public:
  pcodeDiv(){code=P_DIV;}
  virtual void exec(interp& interpreter) override;
};

class pcodeMod: public pcode {
public:
  pcodeMod(){code=P_MOD;}
  virtual void exec(interp& interpreter) override;
};

class pcodeIDiv: public pcode {
public:
  pcodeIDiv(){code=P_IDIV;}
  virtual void exec(interp& interpreter) override;
};

class pcodeEq: public pcode {
public:
  pcodeEq(){code=P_EQ;}
  virtual void exec(interp& interpreter) override;
};

class pcodeLt: public pcode {
public:
  pcodeLt(){code=P_LT;}
  virtual void exec(interp& interpreter) override;
};

class pcodeLe: public pcode {
public:
  pcodeLe(){code=P_LE;}
  virtual void exec(interp& interpreter) override;
};

class pcodeGe: public pcode {
public:
  pcodeGe(){code=P_GE;}
  virtual void exec(interp& interpreter) override;
};

class pcodeGt: public pcode {
public:
  pcodeGt(){code=P_GT;}
  virtual void exec(interp& interpreter) override;
};

class pcodeNe: public pcode {
public:
  pcodeNe(){code=P_NE;}
  virtual void exec(interp& interpreter) override;
};

class pcodePop: public pcode {
public:
  pcodePop(){code=P_POP;}
  virtual void exec(interp& interpreter) override;
};

class pcodeNil: public pcode {
public:
  pcodeNil(){code=P_NIL;}
  virtual void exec(interp& interpreter) override;
};

class pcodeTrue: public pcode {
public:
  pcodeTrue(){code=P_TRUE;}
  virtual void exec(interp& interpreter) override;
};

class pcodeFalse: public pcode {
public:
  pcodeFalse(){code=P_FALSE;}
  virtual void exec(interp& interpreter) override;
};

class pcodeIntConst: public ipcode {
public:
  explicit pcodeIntConst(int v):ipcode(v){code=P_INT_CONST;}
  virtual void exec(interp& interpreter) override;
};

class pcodeStrConst: public spcode {
public:
  explicit pcodeStrConst(const string& v):spcode(v){code=P_STR_CONST;}
  virtual void exec(interp& interpreter) override;
};

class pcodeArray: public ipcode {
public:
  explicit pcodeArray(int v):ipcode(v){code=P_ARRAY;}
  virtual void exec(interp& interpreter) override;
};
	
class pcodeDict: public ipcode {
public:
  explicit pcodeDict(int v):ipcode(v){code=P_DICT;}
  virtual void exec(interp& interpreter) override;
};

class pcodeVar: public ipcode {
public:
  explicit pcodeVar(int v):ipcode(v){code=P_VAR;}
  virtual void exec(interp& interpreter) override;
};

class pcodeVarStore: public ipcode {
public:
  explicit pcodeVarStore(int v):ipcode(v){code=P_VAR;}
  virtual void exec(interp& interpreter) override;
};

class pcodeLoad: public ipcode {
public:
  explicit pcodeLoad(int v):ipcode(v){code=P_LOAD;}
  virtual void exec(interp& interpreter) override;
};

class pcodeSlice: public pcode {
public:
  explicit pcodeSlice(){code=P_SLICE;}
  virtual void exec(interp& interpreter) override;
};

class pcodeStore: public ipcode {
public:
  explicit pcodeStore(int v):ipcode(v){code=P_STORE;}
  virtual void exec(interp& interpreter) override;
};

class pcodeGoto: public ipcode {
public:
  explicit pcodeGoto(int v):ipcode(v){code=P_GOTO;}
  virtual void exec(interp& interpreter) override;
};

class pcodeLabel: public ipcode {
public:
  explicit pcodeLabel(int v):ipcode(v){code=P_LABEL;}
  virtual void exec(interp& interpreter) override {}; // il pcode Label in esecuzione è una NOP
};

class pcodeCall: public ipcode {
public:
  explicit pcodeCall(int v):ipcode(v){code=P_CALL;}
  virtual void exec(interp& interpreter) override;
};

class pcodeIfFalse: public ipcode {
public:
  explicit pcodeIfFalse(int v):ipcode(v){code=P_IF_FALSE;}
  virtual void exec(interp& interpreter) override;
};

class pcodeIfAnd: public ipcode {
public:
  explicit pcodeIfAnd(int v):ipcode(v){code=P_IF_AND;}
  virtual void exec(interp& interpreter) override;
};

class pcodeIfOr: public ipcode {
public:
  explicit pcodeIfOr(int v):ipcode(v){code=P_IF_OR;}
  virtual void exec(interp& interpreter) override;
};

class pcodeNot: public pcode {
public:
  pcodeNot(){code=P_NOT;}
  virtual void exec(interp& interpreter) override;
};

class pcodePrint: public ipcode {
public:
  explicit pcodePrint(int v):ipcode(v){code=P_PRINT;}
  virtual void exec(interp& interpreter) override;
};

class pcodePCodeEnd: public ipcode {
public:
  explicit pcodePCodeEnd(int v):ipcode(v){code=P_PCODEEND;}
  virtual void exec(interp& interpreter) override;
};

class pcodeNotImpl: public spcode {
public:
  explicit pcodeNotImpl(int c,const string& v):spcode(v){code=c;}
  virtual void exec(interp& interpreter) override;
};

class pcodeAnyType: public pcode {
public:
  explicit pcodeAnyType(){code=P_ANY_TYPE;}
  virtual void exec(interp& interpreter) override;
};

class pcodeIntType: public pcode {
public:
  explicit pcodeIntType(){code=P_INT_TYPE;}
  virtual void exec(interp& interpreter) override;
};

class pcodeStrType: public pcode {
public:
  explicit pcodeStrType(){code=P_STR_TYPE;}
  virtual void exec(interp& interpreter) override;
};

class pcodeLine: public ipcode {
public:
  explicit pcodeLine(int v):ipcode(v){code=P_LINE;}
  virtual void exec(interp& interpreter) override;
};

class pcodeProc: public ipcode {
public:
  explicit pcodeProc(int v):ipcode(v){code=P_PROC;}
  virtual void exec(interp& interpreter) override;
};

class pcodeEndParm: public pcode {
public:
  pcodeEndParm(){code=P_ENDPARM;}
  virtual void exec(interp& interpreter) override {};
};

class pcodeEndProc: public pcode {
public:
  pcodeEndProc(){code=P_ENDPROC;}
  virtual void exec(interp& interpreter) override;
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
	case P_VAR:return new pcodeVar(theStringIntern.add(s));
	case P_VAR_STORE:return new pcodeVarStore(theStringIntern.add(s));
	case P_LOAD:return new pcodeLoad(theStringIntern.add(s));
	case P_SLICE:return new pcodeSlice();
	case P_STORE:return new pcodeStore(theStringIntern.add(s));
	case P_GOTO:return new pcodeGoto(atoi(s));
	case P_LABEL:return new pcodeLabel(atoi(s));
	case P_CALL:return new pcodeCall(atoi(s));
	case P_ENDPARM:return new pcodeEndParm();
	case P_ENDPROC:return new pcodeEndProc();
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
	case P_PROC:return new pcodeProc(theStringIntern.add(s));
  }
  return new pcodeNotImpl(c,s);
}

// --- gli oggetti dell'esecuzione dell'interprete

static int theObjCounter=0;

class obj {
public:
  obj() {theObjCounter++;}
  virtual ~obj(){theObjCounter--; if (theObjCounter==0) cout << "no more objs ...\n";}
  virtual string print() const {throw domain_error("print not implemented");}
  virtual shared_ptr<obj> load(int intern) {throw domain_error("load not implemented");}
  virtual shared_ptr<obj> slice(const obj* idx) {throw domain_error("slice not implemented");}
  virtual shared_ptr<obj> store(int intern, shared_ptr<obj> value) {throw domain_error("store not implemented");}
  virtual shared_ptr<obj> storeslice(shared_ptr<obj> idx,shared_ptr<obj> value) {throw domain_error("storeslice not implemented");}
  //virtual shared_ptr<obj> call(int parmCount) {throw domain_error("call not implemented");}
  virtual void call(int parmCount,interp& interpreter){throw domain_error("call (with interpreter) not implemented");}
  //
  virtual shared_ptr<obj> plus(const obj*) const {throw domain_error("plus not implemented");}
  virtual shared_ptr<obj> minus(const obj*) const {throw domain_error("minus not implemented");}
  virtual shared_ptr<obj> uminus() const {throw domain_error("uminus not implemented");}
  virtual shared_ptr<obj> mult(const obj*) const {throw domain_error("mult not implemented");}
  virtual shared_ptr<obj> div(const obj*) const {throw domain_error("div not implemented");}
  virtual shared_ptr<obj> mod(const obj*) const {throw domain_error("mod not implemented");}
  virtual shared_ptr<obj> idiv(const obj*) const {throw domain_error("idiv not implemented");}
  //
  virtual shared_ptr<obj> lt(const obj*) const {throw domain_error("lt not implemented");}
  virtual shared_ptr<obj> le(const obj*) const {throw domain_error("le not implemented");}
  virtual shared_ptr<obj> eq(const obj*) const {throw domain_error("eq not implemented");}
  virtual shared_ptr<obj> ge(const obj*) const {throw domain_error("ge not implemented");}
  virtual shared_ptr<obj> gt(const obj*) const {throw domain_error("gt not implemented");}
  virtual shared_ptr<obj> ne(const obj*) const {throw domain_error("ne not implemented");}
  //
  //virtual shared_ptr<obj> _or() {throw domain_error("or not implemented");}
  //virtual shared_ptr<obj> _and() {throw domain_error("and not implemented");}
  virtual shared_ptr<obj> _not() {throw domain_error("not not implemented");}
  virtual shared_ptr<obj> is(const obj*) {throw domain_error("is not implemented");}

};

class nilObj: public obj {
public:
  virtual string print() const override {return "nil";}
  virtual shared_ptr<obj> eq(const obj*) const override;
};

class intObj:public obj {
public:
  int value;
  explicit intObj(int v){value=v;}
  virtual string print() const override {return to_string(value);}
  virtual shared_ptr<obj> plus(const obj* o) const override;
  virtual shared_ptr<obj> minus(const obj* o) const override;
  virtual shared_ptr<obj> mult(const obj* o) const override;
  virtual shared_ptr<obj> div(const obj* o) const override;
  virtual shared_ptr<obj> idiv(const obj* o) const override;
  virtual shared_ptr<obj> mod(const obj* o) const override;
  virtual shared_ptr<obj> eq(const obj* o) const override;
  virtual shared_ptr<obj> lt(const obj* o) const override;
  virtual shared_ptr<obj> le(const obj* o) const override;
  virtual shared_ptr<obj> ge(const obj* o) const override;
  virtual shared_ptr<obj> gt(const obj* o) const override;
  virtual shared_ptr<obj> ne(const obj* o) const override;
  //
  const intObj* check_int(const obj* o,string msg) const {const intObj* oo=dynamic_cast<const intObj*>(o);if (oo==nullptr) throw domain_error(msg);return oo;}
};

shared_ptr<obj> intObj::plus(const obj* o) const {
  const intObj* oo=check_int(o,"integer + with a non integer");
  //return shared_ptr<obj>(new intObj(value+oo->value));
  return make_shared<intObj>(value+oo->value);
}

shared_ptr<obj> intObj::minus(const obj* o) const {
  const intObj* oo=check_int(o,"integer - with a non integer");
  return shared_ptr<obj>(new intObj(value-oo->value));
}

shared_ptr<obj> intObj::mult(const obj* o) const {
  const intObj* oo=check_int(o,"integer * with a non integer");
  return shared_ptr<obj>(new intObj(value*oo->value));
}

shared_ptr<obj> intObj::div(const obj* o) const {
  const intObj* oo=check_int(o,"integer / with a non integer");
  return shared_ptr<obj>(new intObj(value/oo->value));
}

shared_ptr<obj> intObj::idiv(const obj* o) const {
  const intObj* oo=check_int(o,"integer idiv with a non integer");
  return shared_ptr<obj>(new intObj(value/oo->value));
}

shared_ptr<obj> intObj::mod(const obj* o) const {
  const intObj* oo=check_int(o,"integer % with a non integer");
  return shared_ptr<obj>(new intObj(value % oo->value));
}

class strObj:public obj {
public:
  string value;
  explicit strObj(const string& v):value{v}{}
  virtual string print() const override {return value;}
  virtual shared_ptr<obj> plus(const obj* o) const override;
  virtual shared_ptr<obj> eq(const obj* o) const override;
  virtual shared_ptr<obj> lt(const obj* o) const override;
  virtual shared_ptr<obj> le(const obj* o) const override;
  virtual shared_ptr<obj> ge(const obj* o) const override;
  virtual shared_ptr<obj> gt(const obj* o) const override;
  virtual shared_ptr<obj> ne(const obj* o) const override;
  //
   const strObj* check_str(const obj* o,string msg) const {const strObj* oo=dynamic_cast<const strObj*>(o);if (oo==nullptr) throw domain_error(msg);return oo;}
};

shared_ptr<obj> strObj::plus(const obj* o) const {
  const strObj* oo=check_str(o,"str + with a non str");
  return shared_ptr<obj>(new strObj(value + oo->value));
}

class boolObj:public obj {
public:
  bool value;
  explicit boolObj(bool v){value=v;};
  virtual string print() const override {return (value?"true":"false");};
  virtual shared_ptr<obj> eq(const obj* o) const override;
  virtual shared_ptr<obj> ne(const obj* o) const override;
  //
  const boolObj* check_bool(const obj* o,string msg) const {const boolObj* oo=dynamic_cast<const boolObj*>(o);if (oo==nullptr) throw domain_error(msg);return oo;}
};

// i singleton degli oggetti che non richiedono tante copie ...
shared_ptr<obj> theTrue(new boolObj(true));
shared_ptr<obj> theFalse(new boolObj(false));
shared_ptr<obj> theNil(new nilObj());

shared_ptr<obj> nilObj::eq( const obj* o) const {
	return shared_ptr<obj> (o==theNil.get()?theTrue:theFalse);
}

shared_ptr<obj> intObj::eq(const obj* o) const {
  const intObj* oo=check_int(o,"int op = with a non int");
  return (value == oo->value?theTrue:theFalse);
}

shared_ptr<obj> intObj::lt(const obj* o) const {
  const intObj* oo=check_int(o,"int op < with a non int");
  return (value < oo->value?theTrue:theFalse);
}

shared_ptr<obj> intObj::le(const obj* o) const {
  const intObj* oo=check_int(o,"int op < with a non int");
  return (value <= oo->value?theTrue:theFalse);
}

shared_ptr<obj> intObj::ge(const obj* o) const {
  const intObj* oo=check_int(o,"int op < with a non int");
  return (value >= oo->value?theTrue:theFalse);
}

shared_ptr<obj> intObj::gt(const obj* o) const {
  const intObj* oo=check_int(o,"int op < with a non int");
  return (value > oo->value?theTrue:theFalse);
}

shared_ptr<obj> intObj::ne(const obj* o) const {
  const intObj* oo=check_int(o,"int op < with a non int");
  return (value != oo->value?theTrue:theFalse);
}

shared_ptr<obj> strObj::eq(const obj* o) const {
  const strObj* oo=check_str(o,"str = with a non str");
  return (value.compare(oo->value)==0 ? theTrue:theFalse);
}

shared_ptr<obj> strObj::lt(const obj* o) const {
  const strObj* oo=check_str(o,"str < with a non str");
  return (value.compare(oo->value)==-1 ? theTrue:theFalse);
}

shared_ptr<obj> strObj::le(const obj* o) const {
  const strObj* oo=check_str(o,"str <= with a non str");
  return (value.compare(oo->value)<=0 ? theTrue:theFalse);
}

shared_ptr<obj> strObj::ge(const obj* o) const {
  const strObj* oo=check_str(o,"str >= with a non str");
  return (value.compare(oo->value)>=0 ? theTrue:theFalse);
}

shared_ptr<obj> strObj::gt(const obj* o) const {
  const strObj* oo=check_str(o,"str > with a non str");
  return (value.compare(oo->value)==1 ? theTrue:theFalse);
}

shared_ptr<obj> strObj::ne(const obj* o)  const {
  const strObj* oo=check_str(o,"str <> with a non str");
  return (value.compare(oo->value)!=0 ? theTrue:theFalse);
}

shared_ptr<obj> boolObj::eq(const obj* o) const {
  const boolObj* oo=check_bool(o,"bool = with a non bool");
  return (value == oo->value ? theTrue : theFalse);
}

shared_ptr<obj> boolObj::ne(const obj* o) const {
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

shared_ptr<obj> theIntType=make_shared<intType>();
shared_ptr<obj> theStrType=make_shared<strType>();

// --- gli array e i dizionari

class arrayObj: public obj {
	vector<shared_ptr<obj>> a;
public:
  arrayObj(){}
  explicit arrayObj(const int& sz){resize(sz);}
  virtual shared_ptr<obj> slice(const obj* idx) override {
	    const intObj* pos=dynamic_cast<const intObj*>(idx);
	    if (pos!=nullptr) 
	      return a[pos->value];
	    throw domain_error("slicing an array with a non integer index");
	  };
  virtual shared_ptr<obj> storeslice(shared_ptr<obj> idx,shared_ptr<obj> value) override {
	    const intObj* pos=dynamic_cast<intObj*>(idx.get());
	    if (pos!=nullptr) {a[pos->value]=value;return value;}
	    throw domain_error("storing in an array with a non integer index");
	  };
  virtual shared_ptr<obj> append(shared_ptr<obj> v){a.push_back(v);return v;};
  virtual shared_ptr<obj> remove(int pos){shared_ptr<obj> v=a[pos];a.erase(a.begin()+pos);return v;};
  virtual shared_ptr<obj> insert(int pos,shared_ptr<obj> v){a.insert(a.begin()+pos,v);return v;};
  virtual int len(){return a.size();};
  virtual string print() const override;
  //
  shared_ptr<obj> sliceidx(int idx){return a[idx];}
  void storesliceidx(int idx, const shared_ptr<obj>& v){a[idx]=v;}
  void append(const shared_ptr<obj>& v){a.push_back(v);}
  void resize(int sz){int oldsz=a.size();a.resize(sz);for(int i=oldsz;i<sz;i++) a[i]=theNil;}
};

string arrayObj::print() const {
  string res="";
  if (a.size()>0) {
    for(shared_ptr<obj> o:a){
	  res+=","+o->print();
    }
    res+="]";
    res[0]='[';
  } else 
    res="[]";  
  return res;
}

class dictObj : public obj {
  unordered_map<string,shared_ptr<obj>> map;
public:
  virtual shared_ptr<obj> slice(const obj* idx) override {
	 string key=idx->print();
     if (map.contains(key)) 
       return map[key];
	 string err=key+": key not found in dict";
	 throw out_of_range(err);
  };
  virtual shared_ptr<obj> storeslice(shared_ptr<obj> idx, shared_ptr<obj> value) override {
	 const strObj* key=dynamic_cast<strObj*>(idx.get());
	 if (key!=nullptr){
	   map[key->value]=value;
	   return value;
     }
     throw domain_error("storing in a dictionary with a non string key");
  };
  virtual string print() const override;
  //
  shared_ptr<obj> loadkey(const string& key){return map[key];}
  shared_ptr<obj> storekey(const string& key, shared_ptr<obj> value){map[key]=value;return value;}
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

class containerObj : public obj {
protected:
  containerObj& superlevel;
  unordered_map<int,shared_ptr<obj>> objs;
  unordered_map<int,shared_ptr<obj>> types;
public:
  containerObj();
  explicit containerObj(containerObj& sl):superlevel{sl}{}
  void add(int intern, shared_ptr<obj> type) {
	if (objs.contains(intern)) throw out_of_range("name already in context");
	objs[intern]=theNil;
	types[intern]=type;
  }
  void add(int intern, shared_ptr<obj> type, shared_ptr<obj> value) {
	if (objs.contains(intern)) throw out_of_range("name already in context");
	objs[intern]=value;
	types[intern]=type;
  }
  int adx(int intern, shared_ptr<obj> type, shared_ptr<obj> value){add(intern,type,value);return 1;}
  virtual shared_ptr<obj> load(int intern) override {
    if (objs.contains(intern)) return objs[intern];
    return superlevel.load(intern);
  }
  virtual shared_ptr<obj> store(int intern, shared_ptr<obj> value) override {
	if (objs.contains(intern)){
	  objs[intern]=value;
	  return value;
	}
	return superlevel.store(intern,value);
  }
};

class builtInContainer : public containerObj {
public:	
  //builtInContainer();
};
builtInContainer theBuiltIn;

containerObj::containerObj():superlevel{theBuiltIn}{
}

// --- contenitore che cerca in locale e nel modulo
class procContextObj : public containerObj {
};

// --- il sistema ---

#include "pcodes.c"

class pcodeProgram {
protected:	
  vector<pcode*> prg;    // i pcode che costituiscono il programma
  vector<int> labelPos;  // la posizione delle label nel pcode, per effettuare i salti
public:
  void add(pcode* p){
	prg.push_back(p);
	if (p->getCode()==P_LABEL) putLabelPos(p->getIntValue());
  }
  pcode* get(int i){return prg[i];}
  int getLabelPos(int l){return labelPos[l];}
  virtual ~pcodeProgram(){for(auto p:prg) delete p;}
  int loadPcd(string fn);
  void putLabelPos(int id){
	if ((int)labelPos.size()<=id) labelPos.resize(id+1);
 	labelPos[id]=prg.size()-1;
  }
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
  //shared_ptr<boolObj> True{new boolObj(true)};
  //shared_ptr<boolObj> False{new boolObj(false)};
  //shared_ptr<obj> Nil{new nilObj()};
};
sys theSys;

//#define PRINT_PCODE_EXECUTION

class interp {
public:
  pcodeProgram* prg;
  vector<shared_ptr<obj>> stack;
  int sp,pc,currentSourceLine;
  shared_ptr<containerObj>& context;
  //
  explicit interp(shared_ptr<containerObj>& c):context{c}{
	sp=-1;pc=0;currentSourceLine=0;
	prg=nullptr;
	stack.reserve(20);
  }
  void run(){
	while(pc!=-2){
#ifdef PRINT_PCODE_EXECUTION
	  int instr=prg->get(pc)->getCode();
	  cout << "pc:" << pc << " sp:" << sp << " sz:" << stack.size() << " cap:" << stack.capacity() << " instr:" << instr << " " << pcodetxt[instr] << endl;
#endif
//#define TESTSWITCH
#ifdef TESTSWITCH
      pcode* ppp=prg->get(pc);
      switch (ppp->getCode()){
	    case P_INT_CONST:
          sp++;
          stack.push_back(make_shared<intObj>(ppp->getIntValue()));
		  break;
	    case P_PLUS:  
          stack[sp-1]=stack[sp-1].get()->plus(stack[sp].get());
          sp--;
          stack.pop_back();
          break;
	    default:
		  ppp->exec(*this);
	  }
#else      
	  prg->get(pc)->exec(*this);
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
  builtin_##_fn_(){name=#_fn_;} \
  virtual void call(int parmCount,interp& interpreter) override { getParms(parmCount,interpreter);		  
#define BUILTINEND(_fn_) }}; \
  int add_builtin_##_fn_=theBuiltIn.adx(theStringIntern.add(#_fn_),theNil,make_shared<builtin_##_fn_>());

/*	
class hello : public cppFunc {
public:	
  hello(){name="hello";}
  virtual void call(int paraCount,interp& interpreter) override {
	getParms(i);  
	i.stack.push_back(make_shared<strObj>("hello bult-in!"));
  }
};
*/
//int add_hello_builtin=theBuiltIn.adx(theStringIntern.add("hello"),theNil,make_shared<builtin_hello>());
//builtInContainer::builtInContainer(){
  //add(theStringIntern.add("hello"),theNil,make_shared<builtin_hello>());
//}

BUILTIN(hello)
  interpreter.stack.push_back(make_shared<strObj>("hello bult-in!"));
BUILTINEND(hello)

// --- l'oggetto che implementa la procedura

class procObj : public obj {
protected:
  int name,pc;
  pcodeProgram* prg;
  //weak_ptr<containerObj> ctx; // metodo 1
  shared_ptr<containerObj>& ctx; // metodo 2
public:
  procObj(int n, interp& i):ctx{i.context}{
	  name=n;pc=i.pc;prg=i.prg;
	  //ctx=i->context;
  };
  virtual string print() const override {return "<"+theStringIntern.get(name)+":pcode procedure>";};
  virtual void call(int parmCount,interp& interpreter) override;
};

void procObj:: call(int parmCount, interp& interpreter) {
  // salva lo stato dell'interprete
  int retpc=interpreter.pc;
  pcodeProgram* retprg=interpreter.prg;
  shared_ptr<containerObj> retctx=interpreter.context;
  // raccoglie i parametri
  // DA FARE
  // esegue la procedura
  interpreter.pc=pc+1;
  interpreter.prg=prg;

  //i->context=ctx.lock(); // metodo 1
  interpreter.context=ctx;  // metodo 2

  interpreter.run();
  // toglie dallo stack il puntatore alla procedura
  // mette un nil che è sempre il risultato di una procedura
  interpreter.stack.pop_back();
  interpreter.stack.push_back(theNil);
  // rimette a posto lo stato dell'interprete a prima della chiamata
  interpreter.pc=retpc;
  interpreter.prg=retprg;
  interpreter.context=retctx;
}

// --- riprendo i pcode

void pcodePlus::exec(interp& interpreter){
  const obj* obj2=interpreter.stack[interpreter.sp--].get();
  const obj* obj1=interpreter.stack[interpreter.sp].get();
  interpreter.stack[interpreter.sp]=obj1->plus(obj2);
  interpreter.stack.pop_back();
}

void pcodeMinus::exec(interp& interpreter){
  const obj* obj2=interpreter.stack[interpreter.sp--].get();
  const obj* obj1=interpreter.stack[interpreter.sp].get();
  interpreter.stack[interpreter.sp]=obj1->minus(obj2);
  interpreter.stack.pop_back();
}

void pcodeUMinus::exec(interp& interpreter){
  interpreter.stack[interpreter.sp]=interpreter.stack[interpreter.sp]->uminus();
}

void pcodeMult::exec(interp& interpreter){
  const obj* obj2=interpreter.stack[interpreter.sp--].get();
  const obj* obj1=interpreter.stack[interpreter.sp].get();
  interpreter.stack[interpreter.sp]=obj1->mult(obj2);
  interpreter.stack.pop_back();
}

void pcodeDiv::exec(interp& interpreter){
  const obj* obj2=interpreter.stack[interpreter.sp--].get();
  const obj* obj1=interpreter.stack[interpreter.sp].get();
  interpreter.stack[interpreter.sp]=obj1->div(obj2);
  interpreter.stack.pop_back();
}

void pcodeMod::exec(interp& interpreter){
  const obj* obj2=interpreter.stack[interpreter.sp--].get();
  const obj* obj1=interpreter.stack[interpreter.sp].get();
  interpreter.stack[interpreter.sp]=obj1->mod(obj2);
  interpreter.stack.pop_back();
}

void pcodeIDiv::exec(interp& interpreter){
  const obj* obj2=interpreter.stack[interpreter.sp--].get();
  const obj* obj1=interpreter.stack[interpreter.sp].get();
  interpreter.stack[interpreter.sp]=obj1->idiv(obj2);
  interpreter.stack.pop_back();
}

void pcodeEq::exec(interp& interpreter){
  const obj* obj2=interpreter.stack[interpreter.sp--].get();
  const obj* obj1=interpreter.stack[interpreter.sp].get();
  interpreter.stack[interpreter.sp]=obj1->eq(obj2);
  interpreter.stack.pop_back();
}

void pcodeLt::exec(interp& interpreter){
  const obj* obj2=interpreter.stack[interpreter.sp--].get();
  const obj* obj1=interpreter.stack[interpreter.sp].get();
  interpreter.stack[interpreter.sp]=obj1->lt(obj2);
  interpreter.stack.pop_back();
}

void pcodeLe::exec(interp& interpreter){
  const obj* obj2=interpreter.stack[interpreter.sp--].get();
  const obj* obj1=interpreter.stack[interpreter.sp].get();
  interpreter.stack[interpreter.sp]=obj1->le(obj2);
  interpreter.stack.pop_back();
}

void pcodeGe::exec(interp& interpreter){
  const obj* obj2=interpreter.stack[interpreter.sp--].get();
  const obj* obj1=interpreter.stack[interpreter.sp].get();
  interpreter.stack[interpreter.sp]=obj1->ge(obj2);
  interpreter.stack.pop_back();
}

void pcodeGt::exec(interp& interpreter){
  const obj* obj2=interpreter.stack[interpreter.sp--].get();
  const obj* obj1=interpreter.stack[interpreter.sp].get();
  interpreter.stack[interpreter.sp]=obj1->gt(obj2);
  interpreter.stack.pop_back();
}

void pcodeNe::exec(interp& interpreter){
  const obj* obj2=interpreter.stack[interpreter.sp--].get();
  const obj* obj1=interpreter.stack[interpreter.sp].get();
  interpreter.stack[interpreter.sp]=obj1->ne(obj2);
  interpreter.stack.pop_back();
}

void pcodePop::exec(interp& interpreter){
  interpreter.sp--;
  interpreter.stack.pop_back();
}

void pcodeNil::exec(interp& interpreter){
  interpreter.sp++;
  interpreter.stack.push_back(theNil);
}

void pcodeTrue::exec(interp& interpreter){
  interpreter.sp++;
  interpreter.stack.push_back(theTrue);
};

void pcodeFalse::exec(interp& interpreter){
  interpreter.sp++;
  interpreter.stack.push_back(theFalse);
}

void pcodeIntConst::exec(interp& interpreter){
  interpreter.sp++;
  interpreter.stack.push_back(make_shared<intObj>(value));
}

void pcodeStrConst::exec(interp& interpreter){
  interpreter.sp++;
  interpreter.stack.push_back(make_shared<strObj>(value));
}

void pcodeArray::exec(interp& interpreter){
  shared_ptr<arrayObj> a=make_shared<arrayObj>(value);
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

void pcodeDict::exec(interp& interpreter){
  shared_ptr<dictObj> d=make_shared<dictObj>();
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

void pcodeVar::exec(interp& interpreter){
  interpreter.context->add(value,interpreter.stack[interpreter.sp]);
}

void pcodeVarStore::exec(interp& interpreter){
  interpreter.context->add(value,interpreter.stack[interpreter.sp-1],interpreter.stack[interpreter.sp]);
  interpreter.sp--;
  interpreter.stack.pop_back();
}

void pcodeLoad::exec(interp& interpreter){
  interpreter.sp++;
  interpreter.stack.push_back(interpreter.context->load(value));
}

void pcodeSlice::exec(interp& interpreter){
  const obj* idx=interpreter.stack[interpreter.sp--].get();
  obj* cnt=interpreter.stack[interpreter.sp].get();
  //cout << "cnt:" << cnt->print() << " idx:" << idx->print() << endl;
  interpreter.stack[interpreter.sp]=cnt->slice(idx);
  interpreter.stack.pop_back();
}

void pcodeStore::exec(interp& interpreter){
  interpreter.context->store(value,interpreter.stack[interpreter.sp]);
}

void pcodeGoto::exec(interp& interpreter){
  //cout << "GoTo " << value << " at " << interpreter->prg->getLabelPos(value) << endl;
  int p=interpreter.prg->getLabelPos(value);
  interpreter.pc=p;
}

void pcodeCall::exec(interp& interpreter){
  interpreter.stack[interpreter.sp].get()->call(value,interpreter);
}

void pcodeEndProc::exec(interp& interpreter){
  interpreter.pc=-3;
}

void pcodeIfFalse::exec(interp& interpreter){
  const obj* v=interpreter.stack[interpreter.sp].get();
  if (v!=theTrue.get() && v!= theFalse.get()) throw out_of_range("if with a non boolean expression");
  interpreter.sp--;
  interpreter.stack.pop_back();
  if (v==theFalse.get()) interpreter.pc=interpreter.prg->getLabelPos(value);
}

void pcodeIfAnd::exec(interp& interpreter){
  const obj* v=interpreter.stack[interpreter.sp].get();
  if (v!=theTrue.get() && v!= theFalse.get()) throw out_of_range("and with a non boolean expression");
  if (v==theFalse.get())
    interpreter.pc=interpreter.prg->getLabelPos(value);
  else {
    interpreter.sp--;
	interpreter.stack.pop_back();
  }
}

void pcodeIfOr::exec(interp& interpreter){
  const obj* v=interpreter.stack[interpreter.sp].get();
  if (v!=theTrue.get() && v!= theFalse.get()) throw out_of_range("or with a non boolean expression");
  if (v==theTrue.get())
    interpreter.pc=interpreter.prg->getLabelPos(value);
  else {
  	interpreter.sp--;
	interpreter.stack.pop_back();
  }
}

void pcodeNot::exec(interp& interpreter){
  const obj* v=interpreter.stack[interpreter.sp].get();
  if (v!=theTrue.get() && v!= theFalse.get()) throw out_of_range("not with a non boolean expression");
  interpreter.stack[interpreter.sp]=(v==theTrue.get()?theFalse:theTrue);
}

void pcodePrint::exec(interp& interpreter){
  int i;
  for(i=1;i<=value;i++){
    shared_ptr<obj> o=interpreter.stack[interpreter.sp-value+i];
	cout << o->print();
  }
  cout << endl;
  interpreter.sp-=value;
  for(i=1;i<=value;i++)
    interpreter.stack.pop_back();
}

void pcodePCodeEnd::exec(interp& interpreter){
  //cout << "stop" << endl;
  interpreter.pc=-3; // convenzione per fermarsi
}

void pcodeAnyType::exec(interp& interpreter){
  interpreter.sp++;
  interpreter.stack.push_back(theNil);
}

void pcodeIntType::exec(interp& interpreter){
  interpreter.sp++;
  interpreter.stack.push_back(theIntType);
}

void pcodeStrType::exec(interp& interpreter){
  interpreter.sp++;
  interpreter.stack.push_back(theStrType);
}

void pcodeLine::exec(interp& interpreter){
  //cout << "line:" << value << endl;
  interpreter.currentSourceLine=value;
}

void pcodeNotImpl::exec(interp& interpreter){
  cout << "pcode:" << code << " " << pcodetxt[code] << " value:" << value << " not impl!" << endl;
  throw domain_error("pcode not implemented");
}

void pcodeProc::exec(interp& interpreter){
  int pc=interpreter.pc;
  //cout << "proc:" << theStringIntern.get(value) << " pc:" << interpreter->pc <<endl;
  // --- aggiunge la procedura al contesto attuale
  shared_ptr<obj> p(new procObj(value,interpreter));
  interpreter.context->add(value,theNil,p);
  // --- salta il codice della procedura
  int c=interpreter.prg->get(pc++)->getCode();
  while (c!=P_ENDPROC)
	c=interpreter.prg->get(pc++)->getCode();
  interpreter.pc=pc-1;
}

// ------------------------------------------------
#define ANKERL_NANOBENCH_IMPLEMENT
#include "nanobench.h"

void test(){
  // prova reale ...
  pcodeProgram prg;
  int r=prg.loadPcd("primo.pcd");  
  shared_ptr<containerObj> ctx=make_shared<containerObj>();
  interp exe(ctx);
  exe.prg=&prg;
  if (r) exe.run();
}

int main(){
  //ankerl::nanobench::Bench().run("conta fino a 1000000", [&] {
	  test();
  //});
  return 0;
}

  /*
  int r=1;
  prg.labelPos.resize(10);
  prg.code(new pcodeIntType());     // 0: INT_TYPE
  prg.code(new pcodeVar(0));        // 1: VAR i
  prg.code(new pcodePop());         // 2: POP
  prg.code(new pcodeIntConst(0));   // 3: INT_CONST 0
  prg.code(new pcodeStore(0));      // 4: STORE i
  prg.code(new pcodePop());         // 5: POP 
  prg.code(new pcodeLabel(3));      // 6: LABEL 3
  prg.code(new pcodeLoad(0));       // 7: LOAD i
  prg.code(new pcodeIntConst(1000000)); // 8: INT_CONST 1000000
  prg.code(new pcodeLt());          // 9: LT 
  prg.code(new pcodeIfFalse(4));    //10: IF_FALSE 4
  prg.code(new pcodeLoad(0));       //11: LOAD i
  prg.code(new pcodeIntConst(1));   //12: INT_CONST 1
  prg.code(new pcodePlus());        //13: PLUS 
  prg.code(new pcodeStore(0));      //14: STORE i
  prg.code(new pcodePop());         //15: POP 
  prg.code(new pcodeGoto(3));       //16: GOTO 3
  prg.code(new pcodeLabel(4));      //17: LABEL 4
  prg.code(new pcodePCodeEnd(9));
  */

/*
  //
  int x=theStringIntern.add("x");
  int y=theStringIntern.add("y");
  // --- oggetti di base
  shared_ptr<obj> n(new intObj(100));
  shared_ptr<obj> s(new strObj("pippo"));
  cout << n->print() << " " << s->print() << endl;
  // --- contenitore
  shared_ptr<containerObj> cc(new containerObj());
  cc->add(x,theNil);
  cc->add(y,theNil);
  cc->store(x,n);
  cc->store(y,s);
  cout << cc->load(y)->print() << " " << cc->load(x)->print() << endl;
  // --- array
  shared_ptr<arrayObj> aa(new arrayObj());
  aa->append(n);
  aa->append(s);
  aa->append(theTrue);
  aa->append(theFalse);
  cout << aa->sliceidx(0)->print() << " " << aa->sliceidx(1)->print() << " len:" << aa->len() << endl;
  aa->storesliceidx(0,s);
  cout << 1 << endl;
  shared_ptr<obj> ff(new floatObj(12.34));
  aa->insert(0,ff);
  aa->remove(1);
  cout << aa->sliceidx(0)->print() << " " << aa->sliceidx(1)->print() << " len:" << aa->len() << endl;
  // --- dizionari
  shared_ptr<dictObj> dd(new dictObj());
  dd->storekey("n",n);
  dd->storekey(n->print(),s);
  dd->storekey("xxx",n);
  dd->storekey("aa",aa);
  cout << "dd['n']=" << dd->loadkey("n")->print() << " dd['"<< n->print() <<"']=" << dd->loadkey(n->print())->print() << " dd['xxx']=" << dd->loadkey("xxx")->print() << endl;
  // --- stampa di array
  cout << aa->print() << endl;
  // --- stampa del dizionario
  cout << dd->print() << endl;
  // --- prova di cast ...
  //obj* ir=n;
  intObj* ii=dynamic_cast<intObj*>(n.get());
  if (ii!=0) printf("riuscito come dovrebbe essere!\n");
  shared_ptr<strObj> iii(dynamic_cast<strObj*>(n.get()));
  if (iii.get()==0) printf("non riuscito come dovrebbe essere!\n");
  u8string utf8=u8"Hello € world! ↗";
  printf("%s\n",(char*)utf8.c_str());
  */
