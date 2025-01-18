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

class pcode {
  int code;
  int value;
public:
  pcode(int c,int v){
    code=c;
	value=v;
  }  
};
		
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
  virtual shared_ptr<obj> plus() {throw domain_error("plus not implemented");};
  virtual shared_ptr<obj> minus() {throw domain_error("minus not implemented");};
  virtual shared_ptr<obj> uminus() {throw domain_error("uminus not implemented");};
  virtual shared_ptr<obj> mult() {throw domain_error("mult not implemented");};
  virtual shared_ptr<obj> div() {throw domain_error("div not implemented");};
  virtual shared_ptr<obj> idiv() {throw domain_error("idiv not implemented");};
  //
  virtual shared_ptr<obj> lt() {throw domain_error("lt not implemented");};
  virtual shared_ptr<obj> le() {throw domain_error("le not implemented");};
  virtual shared_ptr<obj> eq() {throw domain_error("eq not implemented");};
  virtual shared_ptr<obj> ge() {throw domain_error("ge not implemented");};
  virtual shared_ptr<obj> gt() {throw domain_error("gt not implemented");};
  virtual shared_ptr<obj> ne() {throw domain_error("ne not implemented");};
  //virtual shared_ptr<obj> ltgt() {throw domain_error("lt-gt not implemented");};
  //virtual shared_ptr<obj> legt() {throw domain_error("le-gt not implemented");};
  //virtual shared_ptr<obj> ltge() {throw domain_error("lt-ge not implemented");};
  //virtual shared_ptr<obj> lege() {throw domain_error("le-ge not implemented");};
  //
  virtual shared_ptr<obj> _or() {throw domain_error("or not implemented");};
  virtual shared_ptr<obj> _and() {throw domain_error("and not implemented");};
  virtual shared_ptr<obj> _not() {throw domain_error("not not implemented");};
  virtual shared_ptr<obj> is() {throw domain_error("is not implemented");};

};

class intObj:public obj {
  int n;
public:
  intObj(int v){n=v;}
  virtual string print(){return to_string(n);}
};

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
	vector<pcode> prg;
};
	 	
class sys {
public:
  // i moduli caricati
  unordered_map<string,pcodeProgram> modules;
  // dei valori che esistono sempre
  shared_ptr<boolObj> True{new boolObj(true)};
  shared_ptr<boolObj> False{new boolObj(false)};
};
sys theSys;

class interp {
public:
  vector<shared_ptr<obj>> stack;
  int sp{0},pc{0};
};

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
  return 0;
}
