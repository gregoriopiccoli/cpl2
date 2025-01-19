/*
  Nil è meglio con nullptr o con uno specifico oggetto nil? 
*/ 

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <print>

using namespace std;

// --- gestione della lista delle stringhe che fanno da identificatori, vengono poste in una lista e viene assegnato un numero progressivo

class StringIntern {
   unordered_map<string,int> map;
   vector<string> imap;
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
  int code;
public:
  pcode(int c){code=c;}  
  virtual ~pcode(){}
  virtual void exec(interp* interpreter)=0; //{throw domain_error("not an executable pcode");};
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

#include "pcodes.h"

pcode* makePCode(int c,const char* s){
  int v;
  switch(c){
	case P_PLUS:return new pcodePlus(P_PLUS);
	case P_MINUS:return new pcodeMinus(P_MINUS);
	case P_UMINUS:return new pcodeUMinus(P_UMINUS);
	case P_MULT:return new pcodeMult(P_MULT);
	case P_DIV:return new pcodeDiv(P_DIV);
	case P_MOD:return new pcodeMod(P_MOD);
	case P_IDIV:return new pcodeIDiv(P_IDIV);
	case P_INT_CONST:return new pcodeIntConst(P_INT_CONST,atoi(s));
	case P_NIL:return new pcodeNil(P_NIL);
	case P_TRUE:return new pcodeTrue(P_TRUE);
	case P_FALSE:return new pcodeFalse(P_FALSE);
	case P_GOTO:return new pcodeGoto(P_GOTO,atoi(s));
	case P_LABEL:return new pcodeLabel(P_LABEL,atoi(s));
	case P_PRINT:return new pcodePrint(P_PRINT,atoi(s));
	case P_PCODEEND:return new pcodePCodeEnd(P_PCODEEND,atoi(s));
  }
  return nullptr;
}

// --- gli oggetti dell'esecuzione dell'interprete

int theObjCounter=0;

class obj {
public:
  obj() {theObjCounter++;};
  virtual ~obj(){theObjCounter--; if (theObjCounter==0) cout << "no more objs ...\n";};
  virtual string print() {throw domain_error("print not implemented");};
  virtual shared_ptr<obj> load(int intern) {throw domain_error("load not implemented");};
  virtual shared_ptr<obj> slice(int pos) {throw domain_error("slice not implemented");};
  virtual shared_ptr<obj> store(int intern, shared_ptr<obj> value) {throw domain_error("store not implemented");};
  virtual shared_ptr<obj> storeslice(int pos,shared_ptr<obj> value) {throw domain_error("storeslice not implemented");};
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
  string s;
public:  
  strObj(string v){s=v;}
  virtual string print(){return s;}
};

class boolObj:public obj {
	bool b;
public:
    boolObj(bool v){b=v;};
    virtual string print(){return (b?"true":"false");};
};

class floatObj: public obj {
	float v;
public:
    floatObj(float f){v=f;};	
    virtual string print(){return to_string(v);};
};

// --- gli array e i dizionari

class arrayObj: public obj {
	vector<shared_ptr<obj>> a;
public:
  virtual shared_ptr<obj> slice(int pos) {return a[pos];};
  virtual shared_ptr<obj> storeslice(int pos,shared_ptr<obj> value) {a[pos]=value;return value;};
  virtual shared_ptr<obj> append(shared_ptr<obj> v){a.push_back(v);return v;};
  virtual shared_ptr<obj> remove(int pos){shared_ptr<obj> v=a[pos];a.erase(a.begin()+pos);return v;};
  virtual shared_ptr<obj> insert(int pos,shared_ptr<obj> v){a.insert(a.begin()+pos,v);return v;};
  virtual int len(){return a.size();};  
  virtual string print();
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
  virtual shared_ptr<obj> load(string key) {
     if (map.contains(key)) return map[key];
	 string err=key+" name not found";
	 throw out_of_range(err);
  };
  virtual shared_ptr<obj> store(string key, shared_ptr<obj> value) {map[key]=value;return value;};    
  virtual string print();
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
  unordered_map<int,shared_ptr<obj>> map;
public:
  virtual shared_ptr<obj> load(int intern) {
     if (map.contains(intern)) return map[intern];
	 string err=theStringIntern.get(intern)+" name not found";
	 throw out_of_range(err);
  };
  virtual shared_ptr<obj> store(int intern, shared_ptr<obj> value) {map[intern]=value;return value;};    
};

// --- il sistema ---

class pcodeProgram {
	vector<pcode*> prg;
public:
    void add(pcode* p){prg.push_back(p);}
    pcode* get(int i){return prg[i];}
    virtual ~pcodeProgram(){for(auto p:prg) delete p;}	
};
	 	
class sys {
public:
  // i moduli caricati
  unordered_map<string,pcodeProgram*> modules;
  // dei valori che esistono sempre
  shared_ptr<boolObj> True{new boolObj(true)};
  shared_ptr<boolObj> False{new boolObj(false)};
};
sys theSys;

class interp {
public:
  pcodeProgram* prg;
  vector<shared_ptr<obj>> stack;
  int sp{-1},pc{0};
  void run(){
	  while(pc!=-2){
		  cout << "pc:" << pc << " sp:" << sp << " sz:" << stack.size() << endl;
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

void pcodeNil::exec(interp* interpreter){
  interpreter->sp++;
  interpreter->stack[interpreter->sp]=nullptr;
};

void pcodeTrue::exec(interp* interpreter){
  interpreter->sp++;
  interpreter->stack.push_back(theSys.True);
};

void pcodeFalse::exec(interp* interpreter){
  interpreter->sp++;
  interpreter->stack.push_back(theSys.False);
};

void pcodeIntConst::exec(interp* interpreter){
  interpreter->sp++;
  obj* o=new intObj(value);
  interpreter->stack.push_back(shared_ptr<obj>(o)); 
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
}

void pcodePCodeEnd::exec(interp* interpreter){
  interpreter->pc=-3; // convenzione per fermarsi 
}

// ------------------------------------------------
	
int main(){
  int x=theStringIntern.add("x");
  int y=theStringIntern.add("y");
  // --- oggetti di base
  shared_ptr<obj> n(new intObj(100));
  shared_ptr<obj> s(new strObj("pippo"));  
  cout << n->print() << " " << s->print() << endl;
  // --- contenitore
  shared_ptr<obj> cc(new containerObj());
  cc->store(x,n);
  cc->store(y,s);
  cout << cc->load(y)->print() << " " << cc->load(x)->print() << endl;  
  // --- array 
  shared_ptr<arrayObj> aa(new arrayObj());
  aa->append(n);
  aa->append(s);  
  aa->append(theSys.True);
  aa->append(theSys.False);
  cout << aa->slice(0)->print() << " " << aa->slice(1)->print() << " len:" << aa->len() << endl;  
  aa->storeslice(0,s);
  shared_ptr<obj> ff(new floatObj(12.34));
  aa->insert(0,ff);
  aa->remove(1);
  cout << aa->slice(0)->print() << " " << aa->slice(1)->print() << " len:" << aa->len() << endl;  
  // --- dizionari
  shared_ptr<dictObj> dd(new dictObj());
  dd->store("n",n);
  dd->store(n->print(),s);
  dd->store("xxx",n);
  dd->store("aa",aa);
  cout << "dd['n']=" << dd->load("n")->print() << " dd['"<< n->print() <<"']=" << dd->load(n->print())->print() << " dd['xxx']=" << dd->load("xxx")->print() << endl;
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
  prg.add(makePCode(P_INT_CONST,"1"));
  prg.add(makePCode(P_INT_CONST,"2"));
  prg.add(makePCode(P_PLUS,""));
  prg.add(makePCode(P_INT_CONST,"3"));
  prg.add(makePCode(P_MULT,""));
  prg.add(makePCode(P_INT_CONST,"3"));
  prg.add(makePCode(P_MINUS,""));
  prg.add(makePCode(P_PRINT,"1"));
  prg.add(makePCode(P_PCODEEND,"0"));
  
  interp exe;
  exe.prg=&prg;
  exe.run(); 
  
  return 0;
}
