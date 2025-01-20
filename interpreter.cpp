/*
  Nil è meglio con nullptr o con uno specifico oggetto nil? per ora provo con un oggetto specifico ...
  Devo fare che le stringhe abbiano le loro operazioni ... prima prova di classe C++! 
*/ 

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <print>

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
  pcode(int c){code=c;}  
  virtual ~pcode(){}
  virtual void exec(interp* interpreter)=0; //{throw domain_error("not an executable pcode");};
  int get(){return code;}
};
		
class ipcode: public pcode {
protected:	
  int value;
public:
  ipcode(int c, int i):pcode(c){value=i;}	
};

class spcode: public pcode {
protected:	
  string value;
public:
  spcode(int c, string s):pcode(c){value=s;}	
};

class pcodePlus: public pcode {
public:
  pcodePlus(int c):pcode(c){}
  virtual void exec(interp* interpreter);
};

class pcodeMinus: public pcode {
public:
  pcodeMinus(int c):pcode(c){}
  virtual void exec(interp* interpreter);
};

class pcodeUMinus: public pcode {
public:
  pcodeUMinus(int c):pcode(c){}
  virtual void exec(interp* interpreter);
};

class pcodeMult: public pcode {
public:
  pcodeMult(int c):pcode(c){}
  virtual void exec(interp* interpreter);
};

class pcodeDiv: public pcode {
public:
  pcodeDiv(int c):pcode(c){}
  virtual void exec(interp* interpreter);
};

class pcodeMod: public pcode {
public:
  pcodeMod(int c):pcode(c){}
  virtual void exec(interp* interpreter);
};

class pcodeIDiv: public pcode {
public:
  pcodeIDiv(int c):pcode(c){}
  virtual void exec(interp* interpreter);
};

class pcodePop: public pcode {
public:
  pcodePop(int c):pcode(c){}
  virtual void exec(interp* interpreter);
};

class pcodeNil: public pcode {
public:
  pcodeNil(int c):pcode(c){}
  virtual void exec(interp* interpreter);
};

class pcodeTrue: public pcode {
public:
  pcodeTrue(int c):pcode(c){}
  virtual void exec(interp* interpreter);
};

class pcodeFalse: public pcode {
public:
  pcodeFalse(int c):pcode(c){}
  virtual void exec(interp* interpreter);
};

class pcodeIntConst: public ipcode {
public:
  pcodeIntConst(int c, int v):ipcode(c,v){}
  virtual void exec(interp* interpreter);
};

class pcodeStrConst: public spcode {
public:
  pcodeStrConst(int c, string v):spcode(c,v){}
  virtual void exec(interp* interpreter);
};

class pcodeVar: public ipcode {
public:
  pcodeVar(int c, int v):ipcode(c,v){}
  virtual void exec(interp* interpreter);
};

class pcodeLoad: public ipcode {
public:
  pcodeLoad(int c, int v):ipcode(c,v){}
  virtual void exec(interp* interpreter);
};
	
class pcodeStore: public ipcode {
public:
  pcodeStore(int c, int v):ipcode(c,v){}
  virtual void exec(interp* interpreter);
};

class pcodeGoto: public ipcode {
public:
  pcodeGoto(int c, int v):ipcode(c,v){}
  virtual void exec(interp* interpreter){};
};

class pcodeLabel: public ipcode {
public:
  pcodeLabel(int c, int v):ipcode(c,v){}
  virtual void exec(interp* interpreter){};
};
	
class pcodePrint: public ipcode {
public:
  pcodePrint(int c, int v):ipcode(c,v){}
  virtual void exec(interp* interpreter);
};

class pcodePCodeEnd: public ipcode {
public:
  pcodePCodeEnd(int c, int v):ipcode(c,v){}
  virtual void exec(interp* interpreter);
};

class pcodeNotImpl: public spcode {
public:
  pcodeNotImpl(int c, string v):spcode(c,v){}	
  virtual void exec(interp* interpreter);
};

class pcodeIntType: public pcode {
public:
  pcodeIntType(int c):pcode(c){}
  virtual void exec(interp* interpreter);
};

class pcodeStrType: public pcode {
public:
  pcodeStrType(int c):pcode(c){}
  virtual void exec(interp* interpreter);
};

class pcodeLine: public ipcode {
public:
  pcodeLine(int c, int v):ipcode(c,v){}
  virtual void exec(interp* interpreter);
};

#include "pcodes.h"

pcode* makePCode(int c,const char* s){
  switch(c){
	case P_PLUS:return new pcodePlus(P_PLUS);
	case P_MINUS:return new pcodeMinus(P_MINUS);
	case P_UMINUS:return new pcodeUMinus(P_UMINUS);
	case P_MULT:return new pcodeMult(P_MULT);
	case P_DIV:return new pcodeDiv(P_DIV);
	case P_MOD:return new pcodeMod(P_MOD);
	case P_IDIV:return new pcodeIDiv(P_IDIV);
	case P_POP:return new pcodePop(P_POP);
	case P_INT_CONST:return new pcodeIntConst(P_INT_CONST,atoi(s));
	case P_STR_CONST:return new pcodeStrConst(P_STR_CONST,s);
	case P_NIL:return new pcodeNil(P_NIL);
	case P_TRUE:return new pcodeTrue(P_TRUE);
	case P_FALSE:return new pcodeFalse(P_FALSE);
	case P_VAR:return new pcodeVar(P_VAR,theStringIntern.add(s));
	case P_LOAD:return new pcodeLoad(P_LOAD,theStringIntern.add(s)); 
	case P_STORE:return new pcodeStore(P_STORE,theStringIntern.add(s));
	case P_GOTO:return new pcodeGoto(P_GOTO,atoi(s));
	case P_LABEL:return new pcodeLabel(P_LABEL,atoi(s));
	case P_PRINT:return new pcodePrint(P_PRINT,atoi(s));
	case P_PCODEEND:return new pcodePCodeEnd(P_PCODEEND,atoi(s));
	case P_INT_TYPE: return new pcodeIntType(P_INT_TYPE);  
	case P_STR_TYPE: return new pcodeIntType(P_STR_TYPE);  
	case P_LINE:return new pcodeLine(P_LINE,atoi(s));
  }
  return new pcodeNotImpl(c,s);
}

// --- gli oggetti dell'esecuzione dell'interprete

int theObjCounter=0;

class obj {
public:
  obj() {theObjCounter++;};
  virtual ~obj(){theObjCounter--; if (theObjCounter==0) cout << "no more objs ...\n";};
  virtual string print() {throw domain_error("print not implemented");};
  virtual shared_ptr<obj> load(int intern) {throw domain_error("load not implemented");};
  virtual shared_ptr<obj> slice(shared_ptr<obj> idx) {throw domain_error("slice not implemented");};
  virtual shared_ptr<obj> store(int intern, shared_ptr<obj> value) {throw domain_error("store not implemented");};
  virtual shared_ptr<obj> storeslice(shared_ptr<obj> idx,shared_ptr<obj> value) {throw domain_error("storeslice not implemented");};
  virtual shared_ptr<obj> call(int n) {throw domain_error("call not implemented");};
  //
  virtual shared_ptr<obj> plus(obj*) {throw domain_error("plus not implemented");};
  virtual shared_ptr<obj> minus(obj*) {throw domain_error("minus not implemented");};
  virtual shared_ptr<obj> uminus() {throw domain_error("uminus not implemented");};
  virtual shared_ptr<obj> mult(obj*) {throw domain_error("mult not implemented");};
  virtual shared_ptr<obj> div(obj*) {throw domain_error("div not implemented");};
  virtual shared_ptr<obj> mod(obj*) {throw domain_error("mod not implemented");};
  virtual shared_ptr<obj> idiv(obj*) {throw domain_error("idiv not implemented");};
  //
  virtual shared_ptr<obj> lt(obj*) {throw domain_error("lt not implemented");};
  virtual shared_ptr<obj> le(obj*) {throw domain_error("le not implemented");};
  virtual shared_ptr<obj> eq(obj*) {throw domain_error("eq not implemented");};
  virtual shared_ptr<obj> ge(obj*) {throw domain_error("ge not implemented");};
  virtual shared_ptr<obj> gt(obj*) {throw domain_error("gt not implemented");};
  virtual shared_ptr<obj> ne(obj*) {throw domain_error("ne not implemented");};
  //virtual shared_ptr<obj> ltgt() {throw domain_error("lt-gt not implemented");};
  //virtual shared_ptr<obj> legt() {throw domain_error("le-gt not implemented");};
  //virtual shared_ptr<obj> ltge() {throw domain_error("lt-ge not implemented");};
  //virtual shared_ptr<obj> lege() {throw domain_error("le-ge not implemented");};
  //
  //virtual shared_ptr<obj> _or() {throw domain_error("or not implemented");};
  //virtual shared_ptr<obj> _and() {throw domain_error("and not implemented");};
  virtual shared_ptr<obj> _not() {throw domain_error("not not implemented");};
  virtual shared_ptr<obj> is(obj*) {throw domain_error("is not implemented");};

};

class nilObj: public obj {
public:	
  virtual string print() {return "nil";};
  virtual shared_ptr<obj> eq(obj*);
};

class intObj:public obj {
public:
  int value;
  intObj(int v){value=v;}
  virtual string print(){return to_string(value);}
  virtual shared_ptr<obj> plus(obj* o);
  virtual shared_ptr<obj> minus(obj* o);
  virtual shared_ptr<obj> mult(obj* o);
};

shared_ptr<obj> intObj::plus(obj* o) {
  intObj* oo=dynamic_cast<intObj*>(o);
  if (oo!=nullptr) {
	return shared_ptr<obj>(new intObj(value+oo->value));  
  } else 
	throw domain_error("integer plus with a non integer");
}

shared_ptr<obj> intObj::minus(obj* o) {
  intObj* oo=dynamic_cast<intObj*>(o);
  if (oo!=nullptr) {
	return shared_ptr<obj>(new intObj(value-oo->value));  
  } else 
	throw domain_error("integer minus with a non integer");
}

shared_ptr<obj> intObj::mult(obj* o) {
  intObj* oo=dynamic_cast<intObj*>(o);
  if (oo!=nullptr) {
	return shared_ptr<obj>(new intObj(value*oo->value));  
  } else 
	throw domain_error("integer mult with a non integer");
}

class strObj:public obj {
public:  
  string value;
  strObj(string v){value=v;}
  virtual string print(){return value;}
};

class boolObj:public obj {
	bool b;
public:
    boolObj(bool v){b=v;};
    virtual string print(){return (b?"true":"false");};
};

// i singleton degli oggetti che non richiedono tante copie ...
shared_ptr<obj> theTrue(new boolObj(true));
shared_ptr<obj> theFalse(new boolObj(false));
shared_ptr<obj> theNil(new nilObj());

shared_ptr<obj> nilObj::eq(obj* o){
	return shared_ptr<obj> (o==theNil.get()?theTrue:theFalse); 
}

class floatObj: public obj {
	float v;
public:
    floatObj(float f){v=f;};	
    virtual string print(){return to_string(v);};
};

// --- i tipi di base 

class intType: public obj {
public:
    virtual string print(){return "int";}    
};

class strType: public obj {
public:
    virtual string print(){return "str";}    
};

shared_ptr<obj> theIntType(new intType);
shared_ptr<obj> theStrType(new strType);

// --- gli array e i dizionari

class arrayObj: public obj {
	vector<shared_ptr<obj>> a;
public:
  
  virtual shared_ptr<obj> slice(shared_ptr<obj> idx) {
	    intObj* pos=dynamic_cast<intObj*>(idx.get());
	    if (pos!=nullptr) return a[pos->value];
	    throw domain_error("slicing an array with a non integer index");
	  };
  virtual shared_ptr<obj> storeslice(shared_ptr<obj> idx,shared_ptr<obj> value) {
	    intObj* pos=dynamic_cast<intObj*>(idx.get());
	    if (pos!=nullptr) {a[pos->value]=value;return value;}
	    throw domain_error("storing in an array with a non integer index");
	  };
  virtual shared_ptr<obj> append(shared_ptr<obj> v){a.push_back(v);return v;};
  virtual shared_ptr<obj> remove(int pos){shared_ptr<obj> v=a[pos];a.erase(a.begin()+pos);return v;};
  virtual shared_ptr<obj> insert(int pos,shared_ptr<obj> v){a.insert(a.begin()+pos,v);return v;};
  virtual int len(){return a.size();};  
  virtual string print(); 
  //
  shared_ptr<obj> sliceidx(int idx){return a[idx];}
  void storesliceidx(int idx, shared_ptr<obj> v){a[idx]=v;}
};

string arrayObj::print(){
  string res="";
  for(auto o:a){
	res+=","+o->print();
  }
  res+="]";
  res[0]='[';
  return res;
}

class dictObj : public obj {
  unordered_map<string,shared_ptr<obj>> map;
public:
  virtual shared_ptr<obj> slice(shared_ptr<obj> idx) {
	 strObj* key=dynamic_cast<strObj*>(idx.get());
	 if (key!=nullptr){
       if (map.contains(key->value)) return map[key->value];
	   string err=key->value+" name not found";
	   throw out_of_range(err);
	 }
	 throw domain_error("slicing a dictionary with a non string index");
  };
  virtual shared_ptr<obj> storeslice(shared_ptr<obj> idx, shared_ptr<obj> value) {
	 strObj* key=dynamic_cast<strObj*>(idx.get());
	 if (key!=nullptr){
	   map[key->value]=value;
	   return value;
     }
     throw domain_error("storing in a dictionary with a non string key");
  };    
  virtual string print();
  //
  shared_ptr<obj> loadkey(string key){return map[key];}
  shared_ptr<obj> storekey(string key, shared_ptr<obj> value){map[key]=value;return value;}
};

string dictObj::print(){
  string res="";
  for(auto [key,o]:map){
	res+=","+key+":"+o->print();
  }
  res+="}";
  res[0]='{';
  return res;
}

// --- i contenitori con nomi (classi, variabili locali, moduli ...)

class containerObj : public obj {
  unordered_map<int,shared_ptr<obj>> objs;
  unordered_map<int,shared_ptr<obj>> types;  
public:
  void add(int intern, shared_ptr<obj> type) {objs[intern]=theNil;types[intern]=type;} 
  virtual shared_ptr<obj> load(int intern) {
     if (objs.contains(intern)) return objs[intern];
	 string err=theStringIntern.get(intern)+" name not found";
	 throw out_of_range(err);
  };
  virtual shared_ptr<obj> store(int intern, shared_ptr<obj> value) {
	 if (objs.contains(intern)){
	   objs[intern]=value;
	   return value;
	 }
	 string err=theStringIntern.get(intern)+" name not found";
	 throw out_of_range(err);
  };    
};

// --- il sistema ---

#include "pcodes.c"

class pcodeProgram {
	vector<pcode*> prg;
public:
    void add(pcode* p){prg.push_back(p);}
    pcode* get(int i){return prg[i];}
    virtual ~pcodeProgram(){for(auto p:prg) delete p;}	
    int loadPcd(string fn);
};

int pcodeProgram::loadPcd(string fn){
	ifstream pcd(fn);
    if (pcd.good()) {     
		char line[2000];
		prg.clear();
		unsigned char code=0;
		pcd.getline(line,2000); // l'intestazione DA FARE!!!
		pcd.getline(line,2000);
		while (!pcd.eof() && code!=P_PCODEEND) {
          code=line[0];code-=31;
          //cout << line << endl;
          //cout << (int)code << "(" << pcodetxt[code] << ")" << " >>" << (char*)(line+1) << "<<" << endl;
          pcode* p=makePCode(code,line+1);
          prg.push_back(p);
		  pcd.getline(line,2000);
        }
        return 1;       
    }
    return 0;
}

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

class interp {
public:
  pcodeProgram* prg;
  vector<shared_ptr<obj>> stack;
  int sp,pc;
  shared_ptr<containerObj> context;   
  //
  interp(containerObj* c){sp=-1;pc=0;stack.reserve(10);shared_ptr<containerObj> cc(c);context=cc;}
  void run(){
	  while(pc!=-2){
		  //int instr=prg->get(pc)->get();
		  //cout << "pc:" << pc << " sp:" << sp << " sz:" << stack.size() << " cap:" << stack.capacity() << " instr:" << instr << " " << pcodetxt[instr] << endl;
		  prg->get(pc)->exec(this);
		  pc++;
	  }
  }
};

// --- riprendo i pcode

void pcodePlus::exec(interp* interpreter){
  shared_ptr<obj> obj2=interpreter->stack[interpreter->sp--];
  shared_ptr<obj> obj1=interpreter->stack[interpreter->sp];
  interpreter->stack.pop_back();
  interpreter->stack[interpreter->sp]=obj1->plus(obj2.get());
};

void pcodeMinus::exec(interp* interpreter){
  shared_ptr<obj> obj2=interpreter->stack[interpreter->sp--];
  shared_ptr<obj> obj1=interpreter->stack[interpreter->sp];
  interpreter->stack.pop_back();
  interpreter->stack[interpreter->sp]=obj1->minus(obj2.get());
};

void pcodeUMinus::exec(interp* interpreter){
  shared_ptr<obj> obj1=interpreter->stack[interpreter->sp];
  interpreter->stack[interpreter->sp]=obj1->uminus();
};

void pcodeMult::exec(interp* interpreter){
  shared_ptr<obj> obj2=interpreter->stack[interpreter->sp--];
  shared_ptr<obj> obj1=interpreter->stack[interpreter->sp];
  interpreter->stack.pop_back();
  interpreter->stack[interpreter->sp]=obj1->mult(obj2.get());
};

void pcodeDiv::exec(interp* interpreter){
  shared_ptr<obj> obj2=interpreter->stack[interpreter->sp--];
  shared_ptr<obj> obj1=interpreter->stack[interpreter->sp];
  interpreter->stack.pop_back();
  interpreter->stack[interpreter->sp]=obj1->div(obj2.get());
};

void pcodeMod::exec(interp* interpreter){
  shared_ptr<obj> obj2=interpreter->stack[interpreter->sp--];
  shared_ptr<obj> obj1=interpreter->stack[interpreter->sp];
  interpreter->stack.pop_back();
  interpreter->stack[interpreter->sp]=obj1->mod(obj2.get());
};

void pcodeIDiv::exec(interp* interpreter){
  shared_ptr<obj> obj2=interpreter->stack[interpreter->sp--];
  shared_ptr<obj> obj1=interpreter->stack[interpreter->sp];
  interpreter->stack.pop_back();
  interpreter->stack[interpreter->sp]=obj1->idiv(obj2.get());
};

void pcodePop::exec(interp* interpreter){
  interpreter->sp--;
  interpreter->stack.pop_back();
};

void pcodeNil::exec(interp* interpreter){
  interpreter->sp++;
  interpreter->stack.push_back(theNil);
};

void pcodeTrue::exec(interp* interpreter){
  interpreter->sp++;
  interpreter->stack.push_back(theTrue);
};

void pcodeFalse::exec(interp* interpreter){
  interpreter->sp++;
  interpreter->stack.push_back(theFalse);
};

void pcodeIntConst::exec(interp* interpreter){
  interpreter->sp++;
  obj* o=new intObj(value);
  interpreter->stack.push_back(shared_ptr<obj>(o)); 
};

void pcodeStrConst::exec(interp* interpreter){
  interpreter->sp++;
  obj* o=new strObj(value);
  interpreter->stack.push_back(shared_ptr<obj>(o)); 
};

void pcodeVar::exec(interp* interpreter){
  cout << "adding var:" << theStringIntern.get(value) << " type:" << interpreter->stack[interpreter->sp]->print() << endl;
  interpreter->context->add(value,interpreter->stack[interpreter->sp]);
};

void pcodeLoad::exec(interp* interpreter){
  interpreter->sp++;
  obj* o=interpreter->context->load(value).get();
  interpreter->stack.push_back(shared_ptr<obj>(o)); 
};

void pcodeStore::exec(interp* interpreter){
  shared_ptr<obj> o=interpreter->stack[interpreter->sp];
  interpreter->context->store(value,o);
};

void pcodePrint::exec(interp* interpreter){
  int i;	
  for(i=1;i<=value;i++){
    shared_ptr<obj> o=interpreter->stack[interpreter->sp-value+i];
    if (o.get()==nullptr) 
	  cout << "nil";
    else  
	  cout << o->print();
  }
  cout << endl;
  interpreter->sp-=value;
  for(i=1;i<=value;i++)
    interpreter->stack.pop_back();
};

void pcodePCodeEnd::exec(interp* interpreter){
  interpreter->pc=-3; // convenzione per fermarsi 
};

void pcodeIntType::exec(interp* interpreter){
  interpreter->sp++;
  interpreter->stack.push_back(theIntType);
};

void pcodeStrType::exec(interp* interpreter){
  interpreter->sp++;
  interpreter->stack.push_back(theStrType);
};

void pcodeLine::exec(interp* interpreter){
	cout << "line:" << value << endl;
};

void pcodeNotImpl::exec(interp* interpreter){
	cout << "pcode:" << code << " " << pcodetxt[code] << " value:" << value << " not impl!" << endl; 
	throw domain_error("pcode not implemented");
}

// ------------------------------------------------
	
int main(){
  //
  initpcodetxt();
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
  
  // prova reale ...
  pcodeProgram prg;
  
  prg.loadPcd("primo.pcd");
  /*
  prg.add(makePCode(P_INT_CONST,"1"));
  prg.add(makePCode(P_INT_CONST,"2"));
  prg.add(makePCode(P_PLUS,""));
  prg.add(makePCode(P_INT_CONST,"3"));
  prg.add(makePCode(P_MULT,""));
  prg.add(makePCode(P_PRINT,"1"));
  prg.add(makePCode(P_INT_CONST,"3"));
  prg.add(makePCode(P_INT_CONST,"1"));
  prg.add(makePCode(P_MINUS,""));
  prg.add(makePCode(P_NIL,""));
  prg.add(makePCode(P_STR_CONST," pippo!"));
  prg.add(makePCode(P_PRINT,"3"));
  //
  prg.add(makePCode(P_INT_TYPE,""));
  prg.add(makePCode(P_VAR,"i"));
  prg.add(makePCode(P_POP,""));
  prg.add(makePCode(P_INT_CONST,"1000"));
  prg.add(makePCode(P_STORE,"i"));
  prg.add(makePCode(P_POP,""));
  prg.add(makePCode(P_LOAD,"i"));
  prg.add(makePCode(P_PRINT,"1"));
  //
  prg.add(makePCode(P_PCODEEND,"0"));
  */
  containerObj* ctx=new containerObj();
  interp exe(ctx);
  exe.prg=&prg;
  exe.run(); 
  
  return 0;
}
