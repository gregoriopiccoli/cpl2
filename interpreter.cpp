/*
Dove possibile usare sempre referenze!

DA FARE:
  tipi definiti dall'utente
  operazioni negli array
  check di tipo profondo nella is per proc/func
  check di tipo in assegnazione ad array e dizionari  
  come fare ! nel tipo or?
  trovare il modo di fare test su errori di compilazione
  la print() di func/proc/class devono mettere il nome del modulo
  fare l'import di modulo del tipo a.b.c
  provare computed goto -- FATTO almeno in parte ed è più veloce! Ho dovuto anche cambiare la getIntValue dei pcode per evitare una chiamata virtuale
  GC parallelo?
  provare a fare un GC super-semplice mark e sweep, magari è più veloce
  fare referenza a tipo di base anche per le stringhe e i float
  Blocco dei parametri da fare una volta sola, FATTO ora però devo fare i blocchi di codice di inizializzazione FATTO e poi devo usarlo!
  La print di una procedura/funzione dovrebbe descriverla completamente, ora lo fa per le proc/func cpl ma devo riuscire anche per quelle c++
  Float e operazioni miste tra int e float
   
FATTO: 
  assegnazione di int a float
  iniziato il tipo float
  array & dict comprehension
  fare il check di tipo su "result"
  gestione del programma iniziale come modulo e gestione della path impostata dal programma di partenza
  verificare il rilascio della memoria dei programmi di pcode
  provare fib in traduzione c++ -- FATTO e cavolo se corre!
  Fatto il Garbage Collector! 
  Provato a riciclare gli interi ... ci si mette più tempo! Dovrò provare con la garbage collection. OTTIMO!
  Nil è meglio con nullptr o con uno specifico oggetto nil? per ora provo con un oggetto specifico così non è mai un puntatore non inizializzato ...
  Devo fare che le stringhe abbiano le loro operazioni ... prima prova di classe C++!
  fare trucco della compare per tipo e mettere le operazioni di confronto sulla compare
*/

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <cassert>
#include <charconv>
//#include <deque>
//#include <memory>

using namespace std;

void showIntCache();
static int gcOpzione=0;
static int tipocheckOpzione=0;  // accende il check di tipo nelle assegnazioni: l'interruttore e' l'opzione --typecheck/-c

#include "gc.cpp"
#include "cli.c"

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
class obj;

class pcode {
protected:
  int code,value;
public:
  pcode():code{0},value{0}{}
  virtual ~pcode(){}
  virtual void exec(interp& interpreter) const {throw domain_error("not an executable pcode");};
  int getCode() const {return code;}
  int getIntValue() const {return value;}
  virtual obj* getObjValue() const {return nullptr;}
  virtual string print() const {return pcodetxt[code];}
};

class ipcode: public pcode {
protected:
  //int value;
public:
  explicit ipcode(int i){value=i;}
  virtual ~ipcode(){}
  //virtual int getIntValue() const override {return value;}
  virtual string print() const override {return string(pcodetxt[code])+" "+to_string(value);}
};

class interningpcode: public pcode {
protected:
  //int value;
public:
  explicit interningpcode(const string& s){value=theStringIntern.add(s);}
  virtual ~interningpcode(){}
  //virtual int getIntValue() const override {return value;}
  virtual string print() const override {return string(pcodetxt[code])+" "+theStringIntern.get(value);}
};

class spcode: public pcode {
protected:
  string value;
public:
  explicit spcode(const string& s):value{s}{}
  virtual ~spcode(){}
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

class pcodeIntConst: public ipcode {
  lockgc_ptr<obj> theValue;	
public:
  explicit pcodeIntConst(int v);
  virtual ~pcodeIntConst() override {theValue=nullptr;}
  virtual void exec(interp& interpreter) const override;
  virtual obj* getObjValue() const override {return theValue;}
};

class pcodeStrConst: public spcode {
  lockgc_ptr<obj> theValue;	
public:
  explicit pcodeStrConst(const string& v);
  virtual ~pcodeStrConst() override {theValue=nullptr;}
  virtual void exec(interp& interpreter) const override;
  virtual obj* getObjValue() const override {return theValue;}
};

class pcodeFloatConst: public spcode {
  lockgc_ptr<obj> theValue;	
public:
  explicit pcodeFloatConst(const string& v);
  virtual ~pcodeFloatConst() override {theValue=nullptr;}
  virtual void exec(interp& interpreter) const override;
  virtual obj* getObjValue() const override {return theValue;}
};

class pcodeImportId: public spcode {
public:
  explicit pcodeImportId(const string& v):spcode(v){code=P_IMPORTID;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeImportIdx: public spcode {
public:
  explicit pcodeImportIdx(const string& v):spcode(v){code=P_IMPORTIDX;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeImport: public ipcode {
public:
  explicit pcodeImport(int v):ipcode(v){code=P_IMPORT;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeArray: public ipcode {
public:
  explicit pcodeArray(int v):ipcode(v){code=P_ARRAY;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeTuple: public ipcode {
public:
  explicit pcodeTuple(int v):ipcode(v){code=P_TUPLE;}
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

class pcodeLoadX: public interningpcode {
public:
  explicit pcodeLoadX(const string& v):interningpcode(v){code=P_LOADX;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeSlice: public pcode {
public:
  explicit pcodeSlice(){code=P_SLICE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeStoreSlice: public pcode {
public:
  explicit pcodeStoreSlice(){code=P_STORESLICE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeStore: public interningpcode {
public:
  explicit pcodeStore(const string& v):interningpcode(v){code=P_STORE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeStoreX: public interningpcode {
public:
  explicit pcodeStoreX(const string& v):interningpcode(v){code=P_STOREX;}
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

class pcodeParmDefault: public interningpcode {
public:
  explicit pcodeParmDefault(const string& v):interningpcode(v){code=P_PARM_DEFAULT;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeIfFalse: public ipcode {
public:
  explicit pcodeIfFalse(int v):ipcode(v){code=P_IF_FALSE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeCase: public ipcode {
public:
  explicit pcodeCase(int v):ipcode(v){code=P_CASE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeForTo: public interningpcode {
public:
  explicit pcodeForTo(const string& v):interningpcode(v){code=P_FOR_TO;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeForDownto: public interningpcode {
public:
  explicit pcodeForDownto(const string& v):interningpcode(v){code=P_FOR_DOWNTO;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeForTest: public ipcode {
public:
  explicit pcodeForTest(int v):ipcode(v){code=P_FOR_TEST;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeForNext: public interningpcode {
public:
  explicit pcodeForNext(const string& v):interningpcode(v){code=P_FOR_NEXT;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeForeachInit: public ipcode {
public:
  explicit pcodeForeachInit(int v):ipcode(v){code=P_FOREACH_INIT;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeForeachVar: public interningpcode {
public:
  explicit pcodeForeachVar(const string& v):interningpcode(v){code=P_FOREACH_VAR;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeForeachKey: public interningpcode {
public:
  explicit pcodeForeachKey(const string& v):interningpcode(v){code=P_FOREACH_KEY;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeForeachNext: public ipcode {
public:
  explicit pcodeForeachNext(int v):ipcode(v){code=P_FOREACH_NEXT;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeComprAppend: public ipcode {
public:
  explicit pcodeComprAppend(int v):ipcode(v){code=P_COMPR_APPEND;}
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

class pcodeNotType: public pcode {
public:
  explicit pcodeNotType(){code=P_NOT_TYPE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeIs: public pcode {
public:
  explicit pcodeIs(){code=P_IS;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeIntType: public pcode {
public:
  explicit pcodeIntType(){code=P_INT_TYPE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeBoolType: public pcode {
public:
  explicit pcodeBoolType(){code=P_BOOL_TYPE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeFloatType: public pcode {
public:
  explicit pcodeFloatType(){code=P_FLOAT_TYPE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeStrType: public pcode {
public:
  explicit pcodeStrType(){code=P_STR_TYPE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeArrayType: public pcode {
public:
  explicit pcodeArrayType(){code=P_ARRAY_TYPE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeDictType: public pcode {
public:
  explicit pcodeDictType(){code=P_DICT_TYPE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeTupleType: public ipcode {
public:
  explicit pcodeTupleType(int v):ipcode(v){code=P_TUPLE_TYPE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeOrType: public ipcode {
public:
  explicit pcodeOrType(int v):ipcode(v){code=P_OR_TYPE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeProcType: public ipcode {
public:
  explicit pcodeProcType(int v):ipcode(v){code=P_PROC_TYPE;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeFuncType: public ipcode {
public:
  explicit pcodeFuncType(int v):ipcode(v){code=P_FUNC_TYPE;}
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


class pcodeClass: public interningpcode {
public:
  explicit pcodeClass(const string& v):interningpcode(v){code=P_CLASS;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeInherit: public ipcode {
public:
  explicit pcodeInherit(int v):ipcode(v){code=P_INHERIT;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeEndParm: public pcode {
public:
  pcodeEndParm(){code=P_ENDPARM;}
  virtual void exec(interp& interpreter) const override;
};

class pcodeEndParmDefault: public pcode {
public:
  pcodeEndParmDefault(){code=P_ENDPARM_DEFAULT;}
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

class pcodeEndClass: public pcode {
public:
  pcodeEndClass(){code=P_ENDCLASS;}
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
	case P_FLOAT_CONST:return new pcodeFloatConst(s);
	case P_IMPORTID:return new pcodeImportId(s);
	case P_IMPORTIDX:return new pcodeImportIdx(s);
	case P_IMPORT:return new pcodeImport(atoi(s));
	case P_NIL:return new pcodeNil();
	case P_TRUE:return new pcodeTrue();
	case P_FALSE:return new pcodeFalse();
	case P_ARRAY:return new pcodeArray(atoi(s));
	case P_DICT:return new pcodeDict(atoi(s));
	case P_TUPLE:return new pcodeTuple(atoi(s));
	case P_VAR:return new pcodeVar(s);
	case P_VAR_STORE:return new pcodeVarStore(s);
	case P_LOAD:return new pcodeLoad(s);
	case P_LOADX:return new pcodeLoadX(s);
	case P_SLICE:return new pcodeSlice();
	case P_STORESLICE:return new pcodeStoreSlice();
	case P_STORE:return new pcodeStore(s);
	case P_STOREX:return new pcodeStoreX(s);
	case P_GOTO:return new pcodeGoto(atoi(s));
	case P_LABEL:return new pcodeLabel(atoi(s));
	case P_CALL:return new pcodeCall(atoi(s));
	case P_PARM:return new pcodeParm(s);
	case P_PARM_DEFAULT:return new pcodeParmDefault(s);
	case P_ENDPARM:return new pcodeEndParm();
	case P_ENDPARM_DEFAULT:return new pcodeEndParmDefault();
	case P_PROC:return new pcodeProc(s);
	case P_ENDPROC:return new pcodeEndProc();
	case P_FUNC:return new pcodeFunc(s);
	case P_ENDFUNC:return new pcodeEndFunc();
	case P_CLASS:return new pcodeClass(s);
	case P_INHERIT: return new pcodeInherit(atoi(s));
	case P_ENDCLASS: return new pcodeEndClass();
	case P_IF_FALSE:return new pcodeIfFalse(atoi(s));
	case P_CASE:return new pcodeCase(atoi(s));
	case P_FOR_TO:return new pcodeForTo(s);
	case P_FOR_DOWNTO:return new pcodeForDownto(s);
	case P_FOR_TEST:return new pcodeForTest(atoi(s));
	case P_FOR_NEXT:return new pcodeForNext(s);
	case P_FOREACH_INIT:return new pcodeForeachInit(atoi(s));
	case P_FOREACH_VAR:return new pcodeForeachVar(s);
	case P_FOREACH_KEY:return new pcodeForeachKey(s);
	case P_FOREACH_NEXT:return new pcodeForeachNext(atoi(s));
	case P_COMPR_APPEND:return new pcodeComprAppend(atoi(s));
	case P_IF_AND:return new pcodeIfAnd(atoi(s));
	case P_IF_OR:return new pcodeIfOr(atoi(s));
	case P_NOT:return new pcodeNot();
	case P_PRINT:return new pcodePrint(atoi(s));
	case P_PCODEEND:return new pcodePCodeEnd(atoi(s));
	case P_ANY_TYPE:return new pcodeAnyType();
	case P_NOT_TYPE:return new pcodeNotType();
	case P_IS:return new pcodeIs();
	case P_INT_TYPE:return new pcodeIntType();
	case P_BOOL_TYPE:return new pcodeBoolType();
	case P_FLOAT_TYPE:return new pcodeFloatType();
	case P_STR_TYPE:return new pcodeStrType();
	case P_ARRAY_TYPE:return new pcodeArrayType();
	case P_DICT_TYPE:return new pcodeDictType();
	case P_TUPLE_TYPE:return new pcodeTupleType(atoi(s));
	case P_OR_TYPE:return new pcodeOrType(atoi(s));
	case P_PROC_TYPE:return new pcodeProcType(atoi(s));
	case P_FUNC_TYPE:return new pcodeFuncType(atoi(s));
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
  virtual obj* uminus() const override;
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
  bool is_int(const obj* o) const;
  double to_float(const obj* o,const char* msg) const;  // l'operando float di int op float, altrimenti msg
};

// definite dopo floatObj: int op float promuove l'int e calcola come float op float
static obj* floatPlus(double a,double b);
static obj* floatMinus(double a,double b);
static obj* floatMult(double a,double b);
static obj* floatDiv(double a,double b);
static obj* floatEq(double a,double b);
static obj* floatLt(double a,double b);
static obj* floatLe(double a,double b);
static obj* floatGe(double a,double b);
static obj* floatGt(double a,double b);
static obj* floatNe(double a,double b);

class intCache {
  vector<intObj*> ints;
public:
  ~intCache(){for(auto o:ints) delete o;}  
  void add(intObj* i){ints.push_back(i);}
  intObj* get(){intObj* r=ints.back();ints.pop_back();return r;}
  int size(){return ints.size();}
};
	
intCache theIntCache;
int intrecycle=0;

void showIntCache(){cout << "intCache size:" << theIntCache.size() << " recycled:" << intrecycle << endl;}

bool intObj::reclaim(){
  generation=0;
  theIntCache.add(this);
  //cout << intcache.size() << endl;
  return false;
}

pcodeIntConst::pcodeIntConst(int v):ipcode(v),theValue{new intObj(v)}{code=P_INT_CONST;}

obj* intObj::plus(const obj* o) const {
  if (!is_int(o)) return floatPlus(value,to_float(o,"integer + with a non integer"));
  const intObj* oo=static_cast<const intObj*>(o);
  if (theIntCache.size()>0) {intObj* v=theIntCache.get();v->value=value+oo->value;stdGC().addRecycled(v);intrecycle++;return v;}
  return new intObj(value+oo->value);
}

obj* intObj::minus(const obj* o) const {
  if (!is_int(o)) return floatMinus(value,to_float(o,"integer - with a non integer"));
  const intObj* oo=static_cast<const intObj*>(o);
  if (theIntCache.size()>0) {intObj* v=theIntCache.get();v->value=value-oo->value;stdGC().addRecycled(v);intrecycle++;return v;}
  return new intObj(value-oo->value);
}

obj* intObj::uminus() const {
  // negazione in aritmetica senza segno: -INT_MIN fa il giro senza comportamento indefinito
  int r=(int)(0u-(unsigned)value);
  if (theIntCache.size()>0) {intObj* v=theIntCache.get();v->value=r;stdGC().addRecycled(v);intrecycle++;return v;}
  return new intObj(r);
}

obj* intObj::mult(const obj* o) const {
  if (!is_int(o)) return floatMult(value,to_float(o,"integer * with a non integer"));
  const intObj* oo=static_cast<const intObj*>(o);
  return new intObj(value*oo->value);
}

obj* intObj::div(const obj* o) const {
  if (!is_int(o)) return floatDiv(value,to_float(o,"integer / with a non integer"));
  const intObj* oo=static_cast<const intObj*>(o);
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
  virtual const obj* getBaseType() const override;
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
  if (!is_int(o)) return floatEq(value,to_float(o,"int op = with a non int"));
  const intObj* oo=static_cast<const intObj*>(o);
  return (value == oo->value?theTrue:theFalse);
}

obj* intObj::lt(const obj* o) const {
  if (!is_int(o)) return floatLt(value,to_float(o,"int op < with a non int"));
  const intObj* oo=static_cast<const intObj*>(o);
  return (value < oo->value?theTrue:theFalse);
}

obj* intObj::le(const obj* o) const {
  if (!is_int(o)) return floatLe(value,to_float(o,"int op < with a non int"));
  const intObj* oo=static_cast<const intObj*>(o);
  return (value <= oo->value?theTrue:theFalse);
}

obj* intObj::ge(const obj* o) const {
  if (!is_int(o)) return floatGe(value,to_float(o,"int op < with a non int"));
  const intObj* oo=static_cast<const intObj*>(o);
  return (value >= oo->value?theTrue:theFalse);
}

obj* intObj::gt(const obj* o) const {
  if (!is_int(o)) return floatGt(value,to_float(o,"int op < with a non int"));
  const intObj* oo=static_cast<const intObj*>(o);
  return (value > oo->value?theTrue:theFalse);
}

obj* intObj::ne(const obj* o) const {
  if (!is_int(o)) return floatNe(value,to_float(o,"int op < with a non int"));
  const intObj* oo=static_cast<const intObj*>(o);
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
    double value;
    explicit floatObj(double f){value=f;};
    virtual string print() const override;
    virtual obj* uminus() const override {return new floatObj(-value);}  // sempre un oggetto nuovo: la costante e' condivisa
    virtual obj* plus(const obj* o) const override;
    virtual obj* minus(const obj* o) const override;
    virtual obj* mult(const obj* o) const override;
    virtual obj* div(const obj* o) const override;
    virtual obj* eq(const obj* o) const override;
    virtual obj* lt(const obj* o) const override;
    virtual obj* le(const obj* o) const override;
    virtual obj* ge(const obj* o) const override;
    virtual obj* gt(const obj* o) const override;
    virtual obj* ne(const obj* o) const override;
    // mod e idiv restano quelli di obj: sono operazioni intere
    //
    virtual const obj* getBaseType() const override;
    //
    double check_num(const obj* o,const char* msg) const;
};

// il calcolo in double, unico per float op float, float op int e int op float;
// l'aritmetica crea sempre un oggetto nuovo, la divisione per zero segue IEEE (inf, nan)
static obj* floatPlus(double a,double b){return new floatObj(a+b);}
static obj* floatMinus(double a,double b){return new floatObj(a-b);}
static obj* floatMult(double a,double b){return new floatObj(a*b);}
static obj* floatDiv(double a,double b){return new floatObj(a/b);}
static obj* floatEq(double a,double b){return (a==b?theTrue:theFalse);}
static obj* floatLt(double a,double b){return (a<b?theTrue:theFalse);}
static obj* floatLe(double a,double b){return (a<=b?theTrue:theFalse);}
static obj* floatGe(double a,double b){return (a>=b?theTrue:theFalse);}
static obj* floatGt(double a,double b){return (a>b?theTrue:theFalse);}
static obj* floatNe(double a,double b){return (a!=b?theTrue:theFalse);}

obj* floatObj::plus(const obj* o) const {return floatPlus(value,check_num(o,"float + with a non number"));}
obj* floatObj::minus(const obj* o) const {return floatMinus(value,check_num(o,"float - with a non number"));}
obj* floatObj::mult(const obj* o) const {return floatMult(value,check_num(o,"float * with a non number"));}
obj* floatObj::div(const obj* o) const {return floatDiv(value,check_num(o,"float / with a non number"));}
obj* floatObj::eq(const obj* o) const {return floatEq(value,check_num(o,"float = with a non number"));}
obj* floatObj::lt(const obj* o) const {return floatLt(value,check_num(o,"float < with a non number"));}
obj* floatObj::le(const obj* o) const {return floatLe(value,check_num(o,"float <= with a non number"));}
obj* floatObj::ge(const obj* o) const {return floatGe(value,check_num(o,"float >= with a non number"));}
obj* floatObj::gt(const obj* o) const {return floatGt(value,check_num(o,"float > with a non number"));}
obj* floatObj::ne(const obj* o) const {return floatNe(value,check_num(o,"float <> with a non number"));}

// la forma piu' corta che riletta da' lo stesso double; senza punto ne' esponente
// si aggiunge .0, cosi' un float non si confonde mai con un intero (2.0 e non 2)
string floatObj::print() const {
  char buf[64];
  auto [fine,ec]=to_chars(buf,buf+sizeof(buf),value);
  string r(buf,fine);
  if (r.find_first_of(".eEinfa")==string::npos) r+=".0";
  return r;
}

// --- i tipi di base

class intType: public obj {
public:
    virtual string print() const override {return "int";}
};

class strType: public obj {
public:
    virtual string print() const override {return "str";}
};

class boolType: public obj {
public:
    virtual string print() const override {return "bool";}
};

class floatType: public obj {
public:
    virtual string print() const override {return "float";}
};

lockgc_ptr<obj> theIntType{new intType()};
lockgc_ptr<obj> theStrType{new strType()};
lockgc_ptr<obj> theBoolType{new boolType()};
lockgc_ptr<obj> theFloatType{new floatType()};
lockgc_ptr<obj> theFloatSonda{new floatObj(0.0)};          // un float qualsiasi: adattaTipo lo usa per chiedere se il tipo accetta un float

const obj* intObj::getBaseType() const {return theIntType;}

const obj* boolObj::getBaseType() const {return theBoolType;}

const obj* floatObj::getBaseType() const {return theFloatType;}

// un float resta com'e', un int viene promosso a double, il resto e' un errore
double floatObj::check_num(const obj* o,const char* msg) const {
  const obj* t=o->getBaseType();
  if (t==theFloatType) return static_cast<const floatObj*>(o)->value;
  if (t==theIntType) return (double)static_cast<const intObj*>(o)->value;
  throw domain_error(msg);
}

// l'operando e' il testo della costante come lo scrive il lexer (cifre.cifre, senza _)
pcodeFloatConst::pcodeFloatConst(const string& v):spcode(v),theValue{new floatObj(strtod(v.c_str(),nullptr))}{code=P_FLOAT_CONST;}

const intObj* intObj::check_int(const obj* o,const char* msg) const {
  if (o->getBaseType()!=theIntType) throw domain_error(msg);
  return static_cast<const intObj*>(o);
}

bool intObj::is_int(const obj* o) const {return o->getBaseType()==theIntType;}

double intObj::to_float(const obj* o,const char* msg) const {
  if (o->getBaseType()!=theFloatType) throw domain_error(msg);
  return static_cast<const floatObj*>(o)->value;
}

// --- i tipi composti

class arrayType : public obj {
	obj* atype;
public:
    //
    virtual void mark() override {if (!marked) {marked=true;atype->mark();}}
    virtual void expand(int gen) override {if (generation<gen) generation=gen;if (atype->generation<gen) atype->expand(gen);}
    //
    arrayType(obj* t){atype=t;};
    const obj* elem() const {return atype;}                    // il tipo degli elementi, per is
    virtual string print() const override {string r="array["+atype->print()+"]";return r;}	
};

class dictType : public obj {
	obj* dtype;
public:
    //
    virtual void mark() override {if (!marked) {marked=true;dtype->mark();}}
    virtual void expand(int gen) override {if (generation<gen) generation=gen;if (dtype->generation<gen) dtype->expand(gen);}
    //
    dictType(obj* t){dtype=t;};
    const obj* elem() const {return dtype;}                    // il tipo dei valori, per is
    virtual string print() const override {string r="array["+dtype->print()+"]";return r;}	
};

class tupleType : public obj {
    vector<obj*> types;
public:
    tupleType(){}
    //
    virtual void mark() override {if (!marked) {marked=true;for(const auto& o:types) o->mark();}}
    virtual void expand(int gen) override {if (generation<gen) generation=gen;for(const auto& o:types) {if (o->generation<gen) o->expand(gen);}}
    //
    virtual void addSubType(obj* t){types.push_back(t);}    
    const vector<obj*>& tipi() const {return types;}           // i tipi posizione per posizione, per is
    virtual string print() const override {
		string r="tuple[";
		int p=1;
		for(obj* t:types) {r+=(p==1?"":",")+t->print();p=0;}
		r+="]";
		return r;
	  }
};

class orType : public obj {
    vector<obj*> altTypes;
public:
    orType(){}
    //
    virtual void mark() override {if (!marked) {marked=true;for(const auto& o:altTypes) o->mark();}}
    virtual void expand(int gen) override {if (generation<gen) generation=gen;for(const auto& o:altTypes) {if (o->generation<gen) o->expand(gen);}}
    //
    virtual void addAltType(obj* t){altTypes.push_back(t);}
    const vector<obj*>& alts() const {return altTypes;}       // le alternative, per il controllo di tipo
    virtual string print() const override {
		string r="";
		int p=1;
		for(obj* t:altTypes) {r+=(p==1?"":"|")+t->print();p=0;}
		return r;
	  }
};

// il tipo marcato con NOT nella dichiarazione: rifiuta il valore nil. Il tipo base
// resta avvolto dentro, cosi' le regole di compatibilita' si applicano igualmente
class notType : public obj {
	obj* t;
public:
    //
    virtual void mark() override {if (!marked) {marked=true;t->mark();}}
    virtual void expand(int gen) override {if (generation<gen) generation=gen;if (t->generation<gen) t->expand(gen);}
    //
    explicit notType(obj* bt){t=bt;}
    obj* inter() const {return t;}                            // il tipo base avvolto dal NOT
    virtual string print() const override {return "not "+t->print();}
};

class procType : public obj {
    vector<obj*> prmTypes;
public:
    procType(){}
    //
    virtual void mark() override {if (!marked) {marked=true;for(const auto& o:prmTypes) o->mark();}}
    virtual void expand(int gen) override {if (generation<gen) generation=gen;for(const auto& o:prmTypes) {if (o->generation<gen) o->expand(gen);}}
    //
    virtual void addPrmType(obj* t){prmTypes.push_back(t);}
    virtual string print() const override {
		string r="(";
		int p=1;
		for(obj* t:prmTypes) {r+=(p==1?"":", ")+t->print();p=0;}
		r+=")";
		return r;
	  }
};
	
class funcType : public obj {
    vector<obj*> prmTypes;
    obj* retType;
public:
    funcType(){}
    //
    virtual void mark() override {if (!marked) {marked=true;for(const auto& o:prmTypes) o->mark();retType->mark();}}
    virtual void expand(int gen) override {if (generation<gen) generation=gen;if (retType->generation<gen) retType->expand(gen);for(const auto& o:prmTypes) {if (o->generation<gen) o->expand(gen);}}
    //
    virtual void addPrmType(obj* t){prmTypes.push_back(t);}
    virtual void setRetType(obj* t){retType=t;}
    virtual string print() const override {
		string r="("+retType->print()+":";
		int p=1;
		for(obj* t:prmTypes) {r+=(p==1?"":", ")+t->print();p=0;}
		r+=")";
		return r;
	  }
};

// --- gli array, le tuple e i dizionari

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
  const vector<obj*>& elementi() const {return a;}           // gli elementi in sola lettura, per is
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

class tupleObj: public obj {
	vector<obj*> a;
public:
  explicit tupleObj(const int& sz){a.resize(sz);}
  //
  virtual void mark() override {if (!marked) {marked=true;for(const auto& o:a) o->mark();}}
  virtual void expand(int gen) override {if (generation<gen) generation=gen;for(const auto& o:a) {if (o->generation<gen) o->expand(gen);}}
  //
  virtual obj* slice(const obj* idx) override {
	    const intObj* pos=dynamic_cast<const intObj*>(idx);
	    if (pos!=nullptr)
	      return a[pos->value];
	    throw domain_error("slicing a tuple with a non integer index");
	  };
  virtual void storeslice(obj* idx,obj* value) override {
	    const intObj* pos=dynamic_cast<intObj*>(idx);
	    if (pos!=nullptr) {
		  a[pos->value]=value;
		} else {
	      throw domain_error("storing in a tuple with a non integer index");
	    }
	  };
  void setelem(int idx,obj* value){a[idx]=value;}
  int size() const {return a.size();}
  obj* elem(int idx) const {return a[idx];}
  const vector<obj*>& elementi() const {return a;}           // gli elementi in sola lettura, per is
  //
  virtual string print() const override;
};

string tupleObj::print() const {
  string res="|";
  int p=1;
  for(const obj* o:a){
	if (p==0) res+=",";
	res+=o->print();
	p=0;
  }
  res+="|";
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
  // per il for each: le chiavi nell'ordine (non specificato) della hash map, e la ricerca che non inserisce
  void chiavi(vector<string>& k) const {k.reserve(map.size());for(const auto& [key,o]:map) k.push_back(key);}
  obj* trova(const string& key) const {auto ff=map.find(key);return ff==map.end()?nullptr:ff->second;}
  const unordered_map<string,obj*>& elementi() const {return map;} // chiavi e valori in sola lettura, per is
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

// --- gli iteratori del for each

// stanno solo sullo stack, fra FOREACH_INIT e il POP di fine ciclo; i nomi delle
// variabili li registrano FOREACH_VAR/FOREACH_KEY (-1 = nessuna variabile).
// next() restituisce false a collezione finita; la chiave la calcola solo se serve.
// In futuro i generatori saranno un'altra implementazione (per questo next riceve l'interprete).
class iterObj : public obj {
public:
  int nomeValore=-1,nomeChiave=-1;
  virtual bool haChiave() const {return true;}
  virtual bool next(obj*& valore,obj*& chiave,interp& interpreter)=0;
  virtual string print() const override {return "<iteratore>";}
};

static intObj* nuovoInt(int v){
  if (theIntCache.size()>0) {intObj* r=theIntCache.get();r->value=v;stdGC().addRecycled(r);intrecycle++;return r;}
  return new intObj(v);
}

// array: visita viva per posizione, la lunghezza e' riletta a ogni passo
class arrayIter : public iterObj {
  arrayObj* a;
  int pos=0;
public:
  explicit arrayIter(arrayObj* aa):a{aa}{}
  virtual void mark() override {if (!marked) {marked=true;a->mark();}}
  virtual void expand(int gen) override {if (generation<gen) generation=gen;if (a->generation<gen) a->expand(gen);}
  virtual bool next(obj*& valore,obj*& chiave,interp&) override {
	if (pos>=a->len()) return false;
	valore=a->sliceidx(pos);
	if (nomeChiave>=0) chiave=nuovoInt(pos);
	pos++;
	return true;
  }
};

class tupleIter : public iterObj {
  tupleObj* t;
  int pos=0;
public:
  explicit tupleIter(tupleObj* tt):t{tt}{}
  virtual void mark() override {if (!marked) {marked=true;t->mark();}}
  virtual void expand(int gen) override {if (generation<gen) generation=gen;if (t->generation<gen) t->expand(gen);}
  virtual bool haChiave() const override {return false;}
  virtual bool next(obj*& valore,obj*&,interp&) override {
	if (pos>=t->size()) return false;
	valore=t->elem(pos++);
	return true;
  }
};

// dizionario: le chiavi sono copiate all'inizio (un rehash invaliderebbe un iteratore
// della hash map), il valore e' quello del momento della visita
class dictIter : public iterObj {
  dictObj* d;
  vector<string> k;
  size_t pos=0;
public:
  explicit dictIter(dictObj* dd):d{dd}{d->chiavi(k);}
  virtual void mark() override {if (!marked) {marked=true;d->mark();}}
  virtual void expand(int gen) override {if (generation<gen) generation=gen;if (d->generation<gen) d->expand(gen);}
  virtual bool next(obj*& valore,obj*& chiave,interp&) override {
	while (pos<k.size()) {
	  obj* v=d->trova(k[pos]);
	  if (v!=nullptr) {
		valore=v;
		if (nomeChiave>=0) chiave=new strObj(k[pos]);
		pos++;
		return true;
	  }
	  pos++;                                              // chiave che non c'e' piu': saltata
	}
	return false;
  }
};

// visita parallela (FOREACH_INIT 2): il valore viene dal primo iteratore, la "chiave" e' il
// valore del secondo; i due interni non calcolano chiavi. Se ne finisce uno solo e' un errore
class zipIter : public iterObj {
  iterObj* a;
  iterObj* b;
public:
  zipIter(iterObj* aa,iterObj* bb):a{aa},b{bb}{}
  virtual void mark() override {if (!marked) {marked=true;a->mark();b->mark();}}
  virtual void expand(int gen) override {
	if (generation<gen) generation=gen;
	if (a->generation<gen) a->expand(gen);
	if (b->generation<gen) b->expand(gen);
  }
  virtual bool next(obj*& valore,obj*& chiave,interp& interpreter) override;
};

// --- i contenitori con nomi (classi, variabili locali, moduli ...)

// compatibilita' fra il tipo dichiarato di una variabile e il valore che le viene
// assegnato (check di tipo in STORE/VAR_STORE/inizializzazione dei parametri).
// La definizione e' in fondo, dopo la dichiarazione di tutte le classi dei valori:
// qui serve solo il prototipo perche' lo usano i metodi del contesto
bool tipoCompatibile(const obj* tipo,const obj* valore);
// il valore da memorizzare in un nome di quel tipo: un int dove serve un float diventa float
obj* adattaTipo(const obj* tipo,obj* valore);

class contextObj : public obj {
protected:
  contextObj* superlevel;                               // il contesto dove verranno cercate tutte le etichette non trovate in questo contesto
  unordered_map<int,obj*> objs;              
  unordered_map<int,obj*> types;
public:
  contextObj();                                         // se non viene specificato un superlevel il superlevel sarà built-in
  explicit contextObj(contextObj* sl):superlevel{sl}{}  // il costruttore che specifica quale è il contesto che fa da superlevel
  //
  virtual void mark() override {if (!marked) {marked=true;for(auto const& [k,o]:objs) o->mark();for(auto const& [k,o]:types) o->mark();}}
  virtual void expand(int gen) override {
	if (generation<gen) generation=gen;
	for(auto const& [k,o]:objs) {if (o->generation<gen) o->expand(gen);} 
	for(auto const& [k,o]:types) {if (o->generation<gen) o->expand(gen);}
  }
  //
  // il valore che fallisce il check di tipo: il nome in mezzo al messaggio per
  // dire dove e' successo, e la consegna come fa il container dei builtin col
  // "name not found": messaggio su stdout e poi l'eccezione
  void checkTipo(int intern,const obj* tipo,const obj* valore) const {
	if (!tipocheckOpzione) return;                        // senza --typecheck/-c il runtime assegna senza controllare
	if (!tipoCompatibile(tipo,valore)) {
	  string err=theStringIntern.get(intern)+": il valore "+valore->print()+" non e' compatibile con il tipo "+tipo->print();
	  cout << err << endl;
	  throw domain_error(err);
	}
  }
  virtual void add(int intern, obj* type) {
	auto ff=objs.find(intern);
	if (ff!=objs.end()) throw out_of_range("name already in context");
	objs[intern]=theNil;                                  // la dichiarazione senza inizializzazione parte nil: il NOT si controllo' alla prima assegnazione
	types[intern]=type;
  }
  virtual void add(int intern, obj* type, obj* value) {
	auto ff=objs.find(intern);
	if (ff!=objs.end()) throw out_of_range("name already in context");
	value=adattaTipo(type,value);                         // un int in un float diventa float, anche senza --typecheck
	checkTipo(intern,type,value);                         // VAR_STORE e inizializzazione dei parametri: il valore deve stare nel tipo
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
	  auto ft=types.find(intern);                         // senza voce = senza tipo dichiarato: nessun controllo (come i tipi any)
	  if (ft!=types.end()) {
		value=adattaTipo(ft->second,value);               // un int in un float diventa float, anche senza --typecheck
		checkTipo(intern,ft->second,value);
	  }
	  ff->second=value;	
	} else {
	  superlevel->store(intern,value);
    }
  }
  // il nome e' dichiarato in questo livello? non risale il superlevel (variabile del for)
  bool isLocal(int intern) const {return objs.find(intern)!=objs.end();}
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

class pcodeProgram {
protected:	
  vector<pcode*> prg;    // i pcode che costituiscono il programma
  vector<int> labelPos;  // la posizione delle label nel pcode, per effettuare i salti
public:
  virtual ~pcodeProgram(){release();}
  void add(pcode* p){
	prg.push_back(p);
	if (p->getCode()==P_LABEL) putLabelPos(p->getIntValue());
  }
  const pcode& get(int i){return *prg[i];}
  int getLabelPos(int l){return labelPos[l];}
  int loadPcd(string fn);
  void putLabelPos(int id){
	if ((int)labelPos.size()<=id) labelPos.resize(id+1);
 	labelPos[id]=prg.size()-1;
  }
  const pcode& operator[](int i) const {return *prg[i];}
  int size() const {return (int)prg.size();}
  void release(){
	//cout << "elimina i pcode del programma" << endl; 
	for(auto p:prg) delete p;
	prg.clear();
	labelPos.clear();
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
	  cerr << "not a compiled cpl2 file ..." <<endl;
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
	  cerr << fn << " has errors!" << endl;
	  return 0;
	}
    return 1;
  }
  // file non apribile: prima non diceva niente (vedi D4). Il messaggio nomina
  // il percorso derivato, cosi' si vede che cosa e' stato cercato.
  cerr << "cannot open p-code file: " << fn << endl;
  prg.push_back(new pcodePCodeEnd(0));
  return 0;
};

class moduleObj: public contextObj {
  pcodeProgram* prg;	
  string name;
public:
  moduleObj(const string n,pcodeProgram* p):contextObj(theBuiltIn){prg=p;name=n;}
  virtual ~moduleObj(){release();}
  virtual void release(){
	  if (locked) unlock();
	  if (prg) prg->release();
	  delete prg;
	  prg=nullptr;
  }
  virtual string print() const override;

};

string moduleObj::print() const {
  return "<module "+name+">";
}


class sys {
public:
  // i moduli caricati
  unordered_map<string,moduleObj*> modules;
  string currentDir;
  // dei valori che esistono sempre
  // metodi del sistema 
  sys(): currentDir(".") {}
  virtual ~sys(){release();};
  moduleObj* loadModule(string name);
  void unloadModules();
  int execModule(string name);
  // 
  virtual void release(){
	//cout << "elimina i contesti dei moduli" << endl;  
	for(auto& [k,c]:modules) {
		//cout << "elimina " << k << endl;
		c->release(); 
	}
	modules.clear();
  }
  string dirOfPath(const string& fn) const;
  string modulePathFor(const string& name, const string& baseDir) const;
};
sys theSys;

string sys::dirOfPath(const string& fn) const {
  if (fn.empty()) return ".";
  size_t slash = fn.find_last_of('/');
  if (slash == string::npos) return ".";
  if (slash == 0) return "/";
  return fn.substr(0, slash);
}

string sys::modulePathFor(const string& name, const string& baseDir) const {
  // trasforma una specifica di modulo del tipo a.b.c in una path di caricamento del pcd dalla cartella di default del sistema	
  string path = name;
  for (size_t i = 0; i < path.size(); i++) {
    if (path[i] == '.') path[i] = '/';
  }
  path += ".pcd";
  if (baseDir.empty() || baseDir == ".") return path;
  if (baseDir == "/") return "/" + path;
  return baseDir + "/" + path;
}

class classObj: public contextObj {
protected:
  vector<classObj*> baseClasses;
  int name;
public:
   explicit classObj(int n, interp& i);
   virtual obj* load(int intern) override;
};

obj* classObj::load(int intern) {
  return nullptr;
}

// istanza di una class, oggetto creato "lazy"?
class instanceObj: public contextObj {
  classObj* theClass;
public:
   virtual obj* load(int intern) override;
};

obj* instanceObj::load(int intern) {
  // cerca tra gli oggetti già creati, se non trova l'oggetto lo crea cercando la definizione nella classe
  return nullptr;
}

//#define PRINT_PCODE_EXECUTION

class interp {
public:
  vector<obj*> stack;
  int sp,pc,currentSourceLine;
  contextObj* context;
  pcodeProgram* prg;
  bool stop;
  gc_array_<obj>* stack_gc;
  contextObj* saved_ctx; 
  //
  explicit interp(contextObj* c, pcodeProgram* p):context{c},prg{p},stack_gc{new gc_array_<obj>(stack)}{init();}
  void init(){
	sp=-1;pc=0;stop=false;
	currentSourceLine=0;
	stack.reserve(20);
  }
  ~interp(){stack_gc->unlock();};
  //
  
#define RUN_WITH_SWITCH

  void run() {
#ifdef RUN_WITH_SWITCH
    static void* pcodejump[256];
    static bool initPcodeJumps=true;
    if (initPcodeJumps){
      for(int i=0;i<256;i++) pcodejump[i]=&&default_pcode;
      pcodejump[P_PLUS]=&&p_plus;
      pcodejump[P_MINUS]=&&p_minus;
      pcodejump[P_LT]=&&p_lt;
      pcodejump[P_LE]=&&p_le;
      pcodejump[P_POP]=&&p_pop;
      pcodejump[P_LOAD]=&&p_load;
      pcodejump[P_INT_CONST]=&&p_int_const;
      pcodejump[P_STORE_RESULT]=&&p_store_result;    
      pcodejump[P_GOTO]=&&p_goto;
      pcodejump[P_IF_FALSE]=&&p_if_false;
      pcodejump[P_CALL]=&&p_call;
      pcodejump[P_ENDPROC]=&&p_endproc;
      pcodejump[P_ENDFUNC]=&&p_endfunc;
      pcodejump[P_ENDPARM]=&&p_endparm;
      pcodejump[P_LINE]=&&p_line;
      initPcodeJumps=false;
    } 
#endif	  
	stop=false;  
	while(!stop){
#ifdef PRINT_PCODE_EXECUTION
	  //int instr=prg->get(pc)->getCode();
	  cout << "pc:" << pc << " sp:" << sp << " sz:" << stack.size() << " cap:" << stack.capacity() << " " << (*prg)[pc].print() << endl;
	  //cout << "pc:" << pc << " sp:" << sp << " " << prg->get(pc)->getCode() << " " << prg->get(pc)->print() << endl;
#endif
#ifdef RUN_WITH_SWITCH
      const pcode& ppp=(*prg)[pc];
      goto *pcodejump[ppp.getCode()];
      p_plus: stack[sp-1]=stack[sp-1]->plus(stack[sp]);stack.pop_back();sp--;goto endpcode;
      p_minus:stack[sp-1]=stack[sp-1]->minus(stack[sp]);stack.pop_back();sp--;goto endpcode;
      p_lt:stack[sp-1]=stack[sp-1]->lt(stack[sp]);stack.pop_back();sp--;goto endpcode;
      p_le:stack[sp-1]=stack[sp-1]->le(stack[sp]);stack.pop_back();sp--;goto endpcode;
      p_pop:stack.pop_back();sp--;goto endpcode;
      p_load:stack.push_back(context->load(ppp.getIntValue()));sp++;goto endpcode;
      p_int_const: stack.push_back(ppp.getObjValue()); sp++; goto endpcode;
      p_store_result:context->store_result(stack[sp]);goto endpcode;
      p_goto:pc=prg->getLabelPos(ppp.getIntValue());goto endpcode;
      p_if_false:{
		const obj* v=stack[sp--];stack.pop_back();
		if (v!=theTrue && v!= theFalse) throw out_of_range("if with a non boolean expression");
        if (v==theFalse) pc=prg->getLabelPos(ppp.getIntValue());
	    }
        goto endpcode;
      p_call:stack[sp-ppp.getIntValue()]->call(ppp.getIntValue(),*this);goto endpcode;
      p_endproc:stop=true;goto endpcode;
      p_endfunc:stop=true;goto endpcode;
      p_endparm:stop=true;goto endpcode;
      p_line: currentSourceLine=ppp.getIntValue();/*ppp.exec(*this);*/goto endpcode;
      default_pcode: ppp.exec(*this);
	  endpcode:
#else      
	  (*prg)[pc].exec(*this);
#endif	  
	  pc++;
	}
  }
};

moduleObj* sys::loadModule(string name) {
  // cout << "module:" << name << endl;
  auto it = modules.find(name);
  if (it != modules.end()) {
    return modules[name];
  }
  string oldDir = currentDir;
  string modPath = modulePathFor(name, currentDir);
  pcodeProgram* prg = new pcodeProgram();
  int loaded = prg->loadPcd(modPath);
  if (!loaded) {
    delete prg;
    return nullptr;
  }
  moduleObj* ctx = new moduleObj(name,prg);
  ctx->lock();
  modules[name] = ctx;
  try {
    currentDir = dirOfPath(modPath);
    if (currentDir.empty()) currentDir = ".";
    interp intp(ctx, prg);
    intp.run();
    assert(intp.sp == -1);
    currentDir = oldDir;
  } catch (...) {
    currentDir = oldDir;
    throw;
  }
  return ctx;
}

void sys::unloadModules(){
  release();
}

int sys::execModule(string name){
  obj* ctx=loadModule(name);
  return ctx!=nullptr?1:0;
}
	
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
  // bookkeeping dei default: per ogni parametro con valore di default memorizza
  // l'intervallo p-code [inizio espressione, posizione di ENDPARM_DEFAULT];
  // nessuna voce per il parametro = nessun default (sentinella)
  unordered_map<int,pair<int,int>> defExp;
  void add(int intern, obj* type) override {
	contextObj::add(intern,type);
	prmOrder.push_back(intern);
  }
  void setDefaultExp(int intern,int start,int end){defExp[intern]={start,end};}
  // intervallo dell'espressione di default del parametro, o {-1,-1} se non ha default
  pair<int,int> getDefaultExp(int intern) const {
	auto ff=defExp.find(intern);
	if (ff!=defExp.end()) return ff->second;
	return {-1,-1};
  }
  obj* tipoDi(int intern) const {                       // il tipo dichiarato del parametro, per costruire la firma
	auto ff=types.find(intern);
	return ff!=types.end()?ff->second:theNil;
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
  procParm* getParm() const {return prm;}               // la lista dei parametri, per risalire alla firma della func
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
  virtual void store_result(obj* value) override {
	static const int resultIntern=theStringIntern.add("result");  // il nome nel messaggio del check, calcolato alla prima chiamata
	value=adattaTipo(result_type,value);                    // func float con result:=1 restituisce 1.0
	checkTipo(resultIntern,result_type,value);              // result:=... deve stare nel tipo di ritorno dichiarato della func
	result_value=value;
  }
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

procObj::procObj(int n, interp& i):ctx{i.context},prg{*i.prg}{
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
  pcodeProgram* retPrg=interpreter.prg;               // il programma che sta eseguendo l'interprete
  contextObj* retCtx=interpreter.context;  // il contesto attuale dell'interprete
  // setta l'interprete allo stato della procedura
  procVars* vars=mkVars();                        // crea il nuovo ambiente delle variabili che ha come contesto di base il modulo dove è stata definita la procedura 
  vars->lock();
  // ^--- func/proc shared_ptr<contextObj> vars=make_shared<contextObj>(*ctx); // crea il nuovo ambiente delle variabili che ha come contesto di base il modulo dove è stata definita la procedura
  // prg e contesto vanno settati prima di initParms, perché initParms può dover
  // valutare le espressioni dei parametri con valore di default
  interpreter.context=vars;                                  // setta le variabili come nuovo contesto dell'interprete
  interpreter.prg=&prg;                                       // setta come programma il pcode della procedura
  prm->initParms(vars,interpreter,parmCnt);                  // crea le variabili dalla lista dei parametri
  // ora ha tolto dallo stack i parametri e sono state create delle variabili locali inizalizzate con i valori passati o con i loro valori di default
  interpreter.pc=pc+1;                                       // mette il program counte alla posizione del codice della procedura
  // ora può interpretare i pcode della procedura
  interpreter.run();
  // mette un nil che è sempre il risultato di una procedura
  interpreter.stack.pop_back(); // toglie l'oggetto procedura che è nella cima dello stack
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
	if (j<parmCnt) {
	  vars->add(p,types[p],i.stack[sp+j]);                 // parametro passato: valore posizionale dallo stack
	} else {
	  // parametro non passato: se ha un default lo valuta, altrimenti nil
	  pair<int,int> d=getDefaultExp(p);
	  if (d.first>=0) {
		int retPc=i.pc;
		i.pc=d.first;                                      // i.pc all'inizio dell'espressione
		i.stop=false;
		i.run();                                           // EndParmDefault ferma la valutazione (default_pcode, non hot)
		obj* v=i.stack[i.sp];                              // il valore calcolato è in cima allo stack
		i.stack.pop_back();
		i.sp--;
		i.pc=retPc;
		i.stop=false;
		vars->add(p,types[p],v);
	  } else {
		vars->add(p,types[p],static_cast<obj*>(theNil));
	  }
	}
  }
  // sistema lo stack togliendo i valori dei parametri passati alla funzione 
  for(int j=0;j<parmCnt;j++)
    i.stack.pop_back();
  i.sp-=parmCnt;  
}

classObj::classObj(int n, interp& i):contextObj{i.context}{
  name=n;
}


// --- le regole di compatibilita' fra il tipo dichiarato e il valore assegnato ---

// Il valore e' compatibile col tipo se ci sta dentro secondo le regole del
// sistema dei tipi. Ogni ramo di tipo avvolto (not) e di tipo unione ricorre
// sulle alternative cosi' le stesse regole valgono dentro. I tipi semplici si
// confrontano col dynamic_cast sul valore: e' il modo in cui gia' lavorano
// check_int/check_str. I tipi che non sapremmo riconoscere (i nomi utente, per
// ora) non bloccano niente.
bool tipoCompatibile(const obj* tipo,const obj* valore){
  if (tipo==(const obj*)theNil || tipo==nullptr)                 // senza tipo dichiarato (any): accetta qualunque valore
	return true;
  if (const orType* t=dynamic_cast<const orType*>(tipo)) {       // tipo unione: basta una alternativa
	for(const obj* a:t->alts())
	  if (a!=(const obj*)theNil && tipoCompatibile(a,valore)) return true;
	return false;
  }
  if (const notType* t=dynamic_cast<const notType*>(tipo))       // tipo NOT: rifiuta il valore nil e poi vale il tipo base
	return valore!=(const obj*)theNil && tipoCompatibile(t->inter(),valore);
  if (valore==(const obj*)theNil)                                // senza NOT anche un tipo definito accetta il nil
	return true;
  if (tipo==(const obj*)theIntType)     return dynamic_cast<const intObj*>(valore)!=nullptr;
  if (tipo==(const obj*)theStrType)     return dynamic_cast<const strObj*>(valore)!=nullptr;
  if (tipo==(const obj*)theBoolType)    return dynamic_cast<const boolObj*>(valore)!=nullptr;
  if (tipo==(const obj*)theFloatType)   return dynamic_cast<const floatObj*>(valore)!=nullptr;
  if (dynamic_cast<const arrayType*>(tipo)) return dynamic_cast<const arrayObj*>(valore)!=nullptr;
  if (dynamic_cast<const dictType*>(tipo))  return dynamic_cast<const dictObj*>(valore)!=nullptr;
  if (dynamic_cast<const tupleType*>(tipo)) return dynamic_cast<const tupleObj*>(valore)!=nullptr;
  if (dynamic_cast<const procType*>(tipo) || dynamic_cast<const funcType*>(tipo))
	return dynamic_cast<const procObj*>(valore)!=nullptr;       // proc e func (una func e' anche un proc)
  return true;                                                   // tipo non riconosciuto: nessun controllo
}

// --- la promozione int -> float nelle assegnazioni ---

// Un int assegnato a un nome il cui tipo richiede un float diventa un float nuovo con
// lo stesso valore: la variabile float contiene sempre un float. Il tipo richiede un
// float se accetta un float ma non l'int cosi' com'e' (float, not float, un'unione con
// float e senza int); la sonda float fa valere le regole di tipoCompatibile senza
// duplicarle. Ogni altro valore torna invariato e lo giudica checkTipo.
// I primi due confronti sono il percorso veloce delle variabili int e senza tipo.
obj* adattaTipo(const obj* tipo,obj* valore){
  if (tipo==(const obj*)theNil || tipo==nullptr || tipo==(const obj*)theIntType) return valore;
  const intObj* i=dynamic_cast<const intObj*>(valore);
  if (i==nullptr || tipoCompatibile(tipo,valore)) return valore;
  if (!tipoCompatibile(tipo,theFloatSonda)) return valore;
  return new floatObj((double)i->value);
}

// --- le regole di appartenenza dell'operatore is ---

// is risponde "il valore e' di questo tipo" e ha regole sue, diverse da quelle
// di tipoCompatibile (che restano quelle delle assegnazioni):
//  - nil appartiene solo ad any (?) e alle unioni che hanno any fra le
//    alternative: (nil is int) e' false anche se var int i := nil e' accettato;
//  - i contenitori si guardano in profondita': ogni elemento dell'array, ogni
//    valore del dict, l'arita' e ogni posizione della tupla;
//  - un contenitore puo' contenere se stesso (a[0]:=a): le coppie valore/tipo
//    in corso di verifica stanno in visita e una coppia che si ripresenta vale
//    true, il ciclo da solo non porta controesempi;
//  - un tipo che non sappiamo riconoscere e' un errore, non una risposta.
// proc e func si riconoscono come in tipoCompatibile, senza confrontare la firma.
static bool tipoIs(const obj* tipo,const obj* valore,vector<pair<const obj*,const obj*>>& visita);

// scende in un contenitore: la coppia resta in visita finche' non e' finito il controllo dei figli
template<typename F> static bool tipoIsDentro(const obj* tipo,const obj* valore,vector<pair<const obj*,const obj*>>& visita,F figli){
  for(const auto& [v,t]:visita)
	if (v==valore && t==tipo) return true;                       // gia' in verifica: struttura ciclica
  visita.emplace_back(valore,tipo);
  bool r=figli();
  visita.pop_back();
  return r;
}

static bool tipoIs(const obj* tipo,const obj* valore,vector<pair<const obj*,const obj*>>& visita){
  if (tipo==(const obj*)theNil)                                  // any: accetta qualunque valore, nil compreso
	return true;
  if (const orType* t=dynamic_cast<const orType*>(tipo)) {       // tipo unione: basta una alternativa, any compreso
	for(const obj* a:t->alts())
	  if (tipoIs(a,valore,visita)) return true;
	return false;
  }
  if (const notType* t=dynamic_cast<const notType*>(tipo))       // tipo NOT: niente nil e poi vale il tipo base
	return valore!=(const obj*)theNil && tipoIs(t->inter(),valore,visita);
  bool riconosciuto=tipo==(const obj*)theIntType || tipo==(const obj*)theStrType || tipo==(const obj*)theBoolType ||
	tipo==(const obj*)theFloatType || dynamic_cast<const arrayType*>(tipo) || dynamic_cast<const dictType*>(tipo) ||
	dynamic_cast<const tupleType*>(tipo) || dynamic_cast<const procType*>(tipo) || dynamic_cast<const funcType*>(tipo);
  if (!riconosciuto) throw domain_error("is: "+tipo->print()+" non e' un tipo");
  if (valore==(const obj*)theNil)                                // nil non e' di nessun tipo definito
	return false;
  if (tipo==(const obj*)theIntType)     return dynamic_cast<const intObj*>(valore)!=nullptr;
  if (tipo==(const obj*)theStrType)     return dynamic_cast<const strObj*>(valore)!=nullptr;
  if (tipo==(const obj*)theBoolType)    return dynamic_cast<const boolObj*>(valore)!=nullptr;
  if (tipo==(const obj*)theFloatType)   return dynamic_cast<const floatObj*>(valore)!=nullptr;
  if (const arrayType* t=dynamic_cast<const arrayType*>(tipo)) {
	const arrayObj* a=dynamic_cast<const arrayObj*>(valore);
	if (a==nullptr) return false;
	return tipoIsDentro(tipo,valore,visita,[&]{
	  for(const obj* e:a->elementi()) if (!tipoIs(t->elem(),e,visita)) return false;
	  return true;});
  }
  if (const dictType* t=dynamic_cast<const dictType*>(tipo)) {
	const dictObj* d=dynamic_cast<const dictObj*>(valore);
	if (d==nullptr) return false;
	return tipoIsDentro(tipo,valore,visita,[&]{
	  for(const auto& [k,e]:d->elementi()) if (!tipoIs(t->elem(),e,visita)) return false;
	  return true;});
  }
  if (const tupleType* t=dynamic_cast<const tupleType*>(tipo)) {
	const tupleObj* u=dynamic_cast<const tupleObj*>(valore);
	if (u==nullptr || u->elementi().size()!=t->tipi().size()) return false;
	return tipoIsDentro(tipo,valore,visita,[&]{
	  for(size_t i=0;i<t->tipi().size();i++) if (!tipoIs(t->tipi()[i],u->elementi()[i],visita)) return false;
	  return true;});
  }
  return dynamic_cast<const procObj*>(valore)!=nullptr;          // proc e func: resta solo questo caso
}

// IS: in cima il tipo, sotto il valore; al loro posto resta il booleano
void pcodeIs::exec(interp& interpreter) const {
  const obj* tipo=interpreter.stack[interpreter.sp];
  const obj* valore=interpreter.stack[interpreter.sp-1];
  vector<pair<const obj*,const obj*>> visita;
  bool r=tipoIs(tipo,valore,visita);
  interpreter.stack.pop_back();
  interpreter.sp--;
  interpreter.stack[interpreter.sp]=(r?theTrue:theFalse);
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

void pcodeFloatConst::exec(interp& interpreter) const {
  interpreter.sp++;
  interpreter.stack.push_back(theValue);
}

void pcodeImportId::exec(interp& interpreter) const {
  interpreter.sp++;
  interpreter.stack.push_back(new strObj(value));
}

void pcodeImportIdx::exec(interp& interpreter) const {
  interpreter.sp++;
  interpreter.stack.push_back(new strObj(value));
  if (interpreter.sp<1) throw domain_error("importidx without a previous component");
  const strObj* id=dynamic_cast<const strObj*>(interpreter.stack[interpreter.sp]);
  const strObj* previous=dynamic_cast<const strObj*>(interpreter.stack[interpreter.sp-1]);
  if (id==nullptr || previous==nullptr) throw domain_error("importidx with a non string component");
  interpreter.stack[interpreter.sp-1]=new strObj(previous->value+"."+id->value);
  interpreter.stack.pop_back();
  interpreter.sp--;
}

void pcodeImport::exec(interp& interpreter) const {
  // l'operando è il numero di nomi generati dalla lista import
  int n=value;
  if (n<0 || interpreter.sp<n-1) throw domain_error("import with too few names on the stack");
  for (int i=interpreter.sp-n+1;i<=interpreter.sp;i++) {
    if (dynamic_cast<const strObj*>(interpreter.stack[i])==nullptr)
      throw domain_error("import with a non string name");
  }
  for (int i=interpreter.sp-n+1;i<=interpreter.sp;i++) {
	string modname=interpreter.stack[i]->print();
    // carica il modulo, se non ancora presente 
    moduleObj* modctx=theSys.loadModule(modname);
    if (modctx==nullptr) throw domain_error("import failed: " + modname);
    // crea il simbolo del modulo con il nome locale (ultimo componente)
    size_t dot = modname.find_last_of('.');
    string localName = dot == string::npos ? modname : modname.substr(dot+1);
    // first-wins
    interpreter.context->add(theStringIntern.add(localName), theNil, modctx);
  }
  for (int j=0;j<n;j++) {
    interpreter.stack.pop_back();
    interpreter.sp--;
  }
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

void pcodeTuple::exec(interp& interpreter) const {
  // la grammatica garantisce N>=1 (non esiste la costante tuple vuota)
  tupleObj* t=new tupleObj(value);
  int sp=interpreter.sp-value+1;
  for(int i=0;i<value;i++){
	t->setelem(i,interpreter.stack[sp+i]);
  }
  for(int i=sp+1;i<=interpreter.sp;i++)
    interpreter.stack.pop_back();
  interpreter.stack[sp]=t;
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

void pcodeLoadX::exec(interp& interpreter) const {
  // il contenitore in cima allo stack viene sostituito dal valore del nome richiesto
  interpreter.stack[interpreter.sp]=interpreter.stack[interpreter.sp]->load(value);
}

void pcodeSlice::exec(interp& interpreter) const {
  const obj* idx=interpreter.stack[interpreter.sp--];
  obj* cnt=interpreter.stack[interpreter.sp];
  //cout << "cnt:" << cnt->print() << " idx:" << idx->print() << endl;
  interpreter.stack[interpreter.sp]=cnt->slice(idx);
  interpreter.stack.pop_back();
}

void pcodeStoreSlice::exec(interp& interpreter) const {
  // layout: [..., contenitore(sp-2), indice(sp-1), valore(sp)]
  obj* val=interpreter.stack[interpreter.sp];
  obj* idx=interpreter.stack[interpreter.sp-1];
  obj* cnt=interpreter.stack[interpreter.sp-2];
  cnt->storeslice(idx,val);
  interpreter.stack[interpreter.sp-2]=val;
  interpreter.stack.pop_back();
  interpreter.stack.pop_back();
  interpreter.sp-=2;
}

void pcodeStore::exec(interp& interpreter) const {
  interpreter.context->store(value,interpreter.stack[interpreter.sp]);
}

void pcodeStoreX::exec(interp& interpreter) const {
  // layout: [..., contenitore(sp-1), valore(sp)]; il valore resta in cima come STORE
  obj* val=interpreter.stack[interpreter.sp];
  obj* cnt=interpreter.stack[interpreter.sp-1];
  cnt->store(value,val);
  interpreter.stack[interpreter.sp-1]=val;
  interpreter.stack.pop_back();
  interpreter.sp--;
}

void pcodeGoto::exec(interp& interpreter) const {
  //cout << "GoTo " << value << " at " << interpreter->prg->getLabelPos(value) << endl;
  int p=interpreter.prg->getLabelPos(value);
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
}

void pcodeParmDefault::exec(interp& interpreter) const {
  // registra il parametro come fa pcodeParm: consuma il tipo dalla cima dello stack
  interpreter.context->add(value,interpreter.stack[interpreter.sp]);
  interpreter.sp--;
  interpreter.stack.pop_back();
  // cerca avanti il primo ENDPARM_DEFAULT: non può esserci annidamento perché
  // l'elemento è un'espressione e le espressioni non dichiarano proc/func
  int e=interpreter.pc+1;
  while(e<interpreter.prg->size() && (*interpreter.prg)[e].getCode()!=P_ENDPARM_DEFAULT) e++;
  if (e==interpreter.prg->size()) throw domain_error("PARM_DEFAULT without ENDPARM_DEFAULT ...");
  // PARM_DEFAULT viene eseguito solo in fase di definizione, dove il contesto è il procParm
  static_cast<procParm*>(interpreter.context)->setDefaultExp(value,interpreter.pc+1,e);
  // salta l'espressione: pc sulla posizione di ENDPARM_DEFAULT, il pc++ del loop
  // di run() atterra subito dopo e l'espressione non viene valutata alla definizione
  interpreter.pc=e;
}

void pcodeEndParm::exec(interp& interpreter) const {
  interpreter.stop=true;
}

void pcodeEndParmDefault::exec(interp& interpreter) const {
  // marcatore di fine espressione: non tocca lo stack, il valore in cima
  // resta quello che l'interprete assegna al parametro
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
  if (v==theFalse) interpreter.pc=interpreter.prg->getLabelPos(value);
}

// switch: in cima la costante del case, sotto il selettore che resta sullo stack.
// Tipi diversi (o selettore nil) non corrispondono e non danno errore.
void pcodeCase::exec(interp& interpreter) const {
  const obj* c=interpreter.stack[interpreter.sp];
  interpreter.sp--;
  interpreter.stack.pop_back();
  const obj* sel=interpreter.stack[interpreter.sp];
  if (typeid(*sel)==typeid(*c) && sel->eq(c)==theTrue)
    interpreter.pc=interpreter.prg->getLabelPos(value);
}

// for: fine e passo restano sullo stack per tutto il ciclo, la variabile sta nel contesto
// corrente e viene riletta a ogni giro. Gli errori come checkTipo: messaggio e poi l'eccezione.
static void forErrore(const string& err){
  cout << err << endl;
  throw domain_error(err);
}

// layout: [..., inizio(sp-2), fine(sp-1), passo(sp)] -> [..., fine, passo]; la variabile vale inizio
static void forInit(interp& interpreter,int nome,bool crescente){
  string n=theStringIntern.get(nome);
  if (!interpreter.context->isLocal(nome)) forErrore(n+": la variabile del for deve essere dichiarata nel contesto corrente");
  obj* inizio=interpreter.stack[interpreter.sp-2];
  const intObj* fine=dynamic_cast<const intObj*>(interpreter.stack[interpreter.sp-1]);
  const intObj* passo=dynamic_cast<const intObj*>(interpreter.stack[interpreter.sp]);
  if (dynamic_cast<const intObj*>(inizio)==nullptr || fine==nullptr || passo==nullptr)
    forErrore(n+": inizio, fine e passo del for devono essere int");
  if (crescente && passo->value<=0) forErrore(n+": il passo del for con to deve essere positivo");
  if (!crescente && passo->value>=0) forErrore(n+": il passo del for con downto deve essere negativo");
  interpreter.context->store(nome,inizio);
  interpreter.stack[interpreter.sp-2]=interpreter.stack[interpreter.sp-1];
  interpreter.stack[interpreter.sp-1]=interpreter.stack[interpreter.sp];
  interpreter.sp--;
  interpreter.stack.pop_back();
}

void pcodeForTo::exec(interp& interpreter) const {
  forInit(interpreter,value,true);
}

void pcodeForDownto::exec(interp& interpreter) const {
  forInit(interpreter,value,false);
}

// layout: [..., fine(sp-2), passo(sp-1), valore(sp)] -> [..., fine, passo]; il verso lo da' il segno del passo
void pcodeForTest::exec(interp& interpreter) const {
  const intObj* v=dynamic_cast<const intObj*>(interpreter.stack[interpreter.sp]);
  interpreter.sp--;
  interpreter.stack.pop_back();
  if (v==nullptr) forErrore("la variabile del for deve essere int");
  const intObj* fine=static_cast<const intObj*>(interpreter.stack[interpreter.sp-1]);
  const intObj* passo=static_cast<const intObj*>(interpreter.stack[interpreter.sp]);
  if (passo->value>0 ? v->value>fine->value : v->value<fine->value)
    interpreter.pc=interpreter.prg->getLabelPos(value);
}

// layout: [..., fine, passo(sp)] invariato; variabile := variabile + passo
void pcodeForNext::exec(interp& interpreter) const {
  obj* v=interpreter.context->load(value);
  if (dynamic_cast<const intObj*>(v)==nullptr) forErrore(theStringIntern.get(value)+": la variabile del for deve essere int");
  interpreter.context->store(value,v->plus(interpreter.stack[interpreter.sp]));
}

// for each: l'iteratore resta in cima allo stack per tutto il ciclo (lo toglie il POP
// finale); le variabili sono controllate una volta sola, quando vengono registrate.

// layout: [..., collezione(sp)] -> [..., iteratore(sp)]
static iterObj* nuovoIter(obj* c){
  if (arrayObj* a=dynamic_cast<arrayObj*>(c)) return new arrayIter(a);
  if (dictObj* d=dynamic_cast<dictObj*>(c)) return new dictIter(d);
  if (tupleObj* t=dynamic_cast<tupleObj*>(c)) return new tupleIter(t);
  forErrore("for each su un valore non iterabile: "+c->print());
  return nullptr;
}

bool zipIter::next(obj*& valore,obj*& chiave,interp& interpreter){
  obj* k=nullptr;
  bool ha=a->next(valore,k,interpreter);
  bool hb=b->next(chiave,k,interpreter);
  if (ha!=hb) forErrore("for each parallelo su collezioni di lunghezze diverse");
  return ha;
}

// operando 0: [..., collezione] -> [..., iteratore]
// operando 2: [..., prima, seconda] -> [..., iteratore parallelo]
void pcodeForeachInit::exec(interp& interpreter) const {
  if (value==2) {
	iterObj* a=nuovoIter(interpreter.stack[interpreter.sp-1]);
	interpreter.stack[interpreter.sp-1]=a;                // gia' sullo stack, raggiungibile dal GC
	iterObj* b=nuovoIter(interpreter.stack[interpreter.sp]);
	interpreter.stack[interpreter.sp]=b;
	zipIter* z=new zipIter(a,b);
	interpreter.stack.pop_back();
	interpreter.sp--;
	interpreter.stack[interpreter.sp]=z;
	return;
  }
  interpreter.stack[interpreter.sp]=nuovoIter(interpreter.stack[interpreter.sp]);
}

static iterObj* foreachIter(interp& interpreter){
  return static_cast<iterObj*>(interpreter.stack[interpreter.sp]);
}

static void foreachLocale(interp& interpreter,int nome){
  if (!interpreter.context->isLocal(nome))
    forErrore(theStringIntern.get(nome)+": la variabile del for each deve essere dichiarata nel contesto corrente");
}

// layout: [..., iteratore(sp)] invariato
void pcodeForeachVar::exec(interp& interpreter) const {
  foreachLocale(interpreter,value);
  foreachIter(interpreter)->nomeValore=value;
}

// layout: [..., iteratore(sp)] invariato
void pcodeForeachKey::exec(interp& interpreter) const {
  foreachLocale(interpreter,value);
  iterObj* it=foreachIter(interpreter);
  if (!it->haChiave()) forErrore(theStringIntern.get(value)+": le tuple non hanno chiave");
  it->nomeChiave=value;
}

// layout: [..., iteratore(sp)] invariato; salta alla label a collezione finita
void pcodeForeachNext::exec(interp& interpreter) const {
  iterObj* it=foreachIter(interpreter);
  obj* v=nullptr;
  obj* k=nullptr;
  if (!it->next(v,k,interpreter)) {
	interpreter.pc=interpreter.prg->getLabelPos(value);
	return;
  }
  interpreter.context->store(it->nomeValore,v);
  if (it->nomeChiave>=0) interpreter.context->store(it->nomeChiave,k);
}

// comprehension: il risultato sta sotto l'iteratore, quanto calcolato in cima.
// Solo il compilatore emette questa sequenza e il risultato viene sempre da ARRAY 0 / DICT 0.
// operando 0 (array): [..., risultato(sp-2), iteratore(sp-1), valore(sp)] -> [..., risultato, iteratore]
// operando 2 (dict):  [..., risultato(sp-3), iteratore(sp-2), chiave(sp-1), valore(sp)] -> [..., risultato, iteratore]
void pcodeComprAppend::exec(interp& interpreter) const {
  if (value==2) {
	// storeslice converte la chiave con print() come DICT n, e un doppione sovrascrive
	interpreter.stack[interpreter.sp-3]->storeslice(interpreter.stack[interpreter.sp-1],interpreter.stack[interpreter.sp]);
	interpreter.stack.pop_back();
	interpreter.stack.pop_back();
	interpreter.sp-=2;
	return;
  }
  static_cast<arrayObj*>(interpreter.stack[interpreter.sp-2])->append(interpreter.stack[interpreter.sp]);
  interpreter.stack.pop_back();
  interpreter.sp--;
}

void pcodeIfAnd::exec(interp& interpreter) const {
  const obj* v=interpreter.stack[interpreter.sp];
  if (v!=theTrue && v!= theFalse) throw out_of_range("and with a non boolean expression");
  if (v==theFalse)
    interpreter.pc=interpreter.prg->getLabelPos(value);
  else {
    interpreter.sp--;
	interpreter.stack.pop_back();
  }
}

void pcodeIfOr::exec(interp& interpreter) const {
  const obj* v=interpreter.stack[interpreter.sp];
  if (v!=theTrue && v!= theFalse) throw out_of_range("or with a non boolean expression");
  if (v==theTrue)
    interpreter.pc=interpreter.prg->getLabelPos(value);
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

void pcodeNotType::exec(interp& interpreter) const {
  // avvolge il tipo gia' in cima allo stack: dopo il NOT il tipo conta come non-nil
  interpreter.stack[interpreter.sp]=new notType(interpreter.stack[interpreter.sp]);
}

void pcodeIntType::exec(interp& interpreter) const {
  interpreter.sp++;
  interpreter.stack.push_back(theIntType);
}

void pcodeBoolType::exec(interp& interpreter) const {
  interpreter.sp++;
  interpreter.stack.push_back(theBoolType);
}

void pcodeFloatType::exec(interp& interpreter) const {
  interpreter.sp++;
  interpreter.stack.push_back(theFloatType);
}

void pcodeStrType::exec(interp& interpreter) const {
  interpreter.sp++;
  interpreter.stack.push_back(theStrType);
}

void pcodeArrayType::exec(interp& interpreter) const {
  obj* t=interpreter.stack[interpreter.sp];
  interpreter.stack[interpreter.sp]=new arrayType(t);
}

void pcodeDictType::exec(interp& interpreter) const {
  obj* t=interpreter.stack[interpreter.sp];
  interpreter.stack[interpreter.sp]=new dictType(t);
}

void pcodeTupleType::exec(interp& interpreter) const {
  tupleType* t=new tupleType();
  for(int i=0;i<value;i++){
	  t->addSubType(interpreter.stack[interpreter.sp-value+1+i]);
  }
  for(int i=0;i<value-1;i++) interpreter.stack.pop_back();
  interpreter.sp-=(value-1);
  interpreter.stack[interpreter.sp]=t; 	 
}

void pcodeOrType::exec(interp& interpreter) const {
  orType* t=new orType();
  for(int i=0;i<value;i++){
	  t->addAltType(interpreter.stack[interpreter.sp-value+1+i]);
  }
  for(int i=0;i<value-1;i++) interpreter.stack.pop_back();
  interpreter.sp-=(value-1);
  interpreter.stack[interpreter.sp]=t; 	 
  //cout << t->print() << endl;
}

void pcodeProcType::exec(interp& interpreter) const {
  procType* t=new procType();
  for(int i=0;i<value;i++){
	  t->addPrmType(interpreter.stack[interpreter.sp-value+1+i]);
  }
  if (value==0) {
    interpreter.sp++;
    interpreter.stack.push_back(t);
  } else {
    for(int i=0;i<value-1;i++) interpreter.stack.pop_back();
    interpreter.sp-=(value-1);
    interpreter.stack[interpreter.sp]=t;
  }
  //cout << t->print() << endl;
}

void pcodeFuncType::exec(interp& interpreter) const {
  funcType* t=new funcType();
  for(int i=0;i<value;i++){
	  t->addPrmType(interpreter.stack[interpreter.sp-value+1+i]);
  }
  t->setRetType(interpreter.stack[interpreter.sp-value]);
  for(int i=0;i<value;i++) interpreter.stack.pop_back();
  interpreter.sp-=value;
  interpreter.stack[interpreter.sp]=t;
  //cout << t->print() << endl;
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
  // per una func il nome (value) viene legato alla firma completa (result type + tipi
  // dei parametri) e non al solo result type: altrimenti il check di scrittura
  // rifiuterebbe il valore funcObj col tipo int. Il funcType resta in vita perche'
  // viene messo nella mappa dei tipi del contesto, che lo marca al GC
  obj* declared=type;
  funcObj* fo=dynamic_cast<funcObj*>(p);
  if (type!=(obj*)theNil && fo!=nullptr){
	procParm* pp=fo->getParm();
	funcType* ft=new funcType();
	for(const int& q:pp->prmOrder) ft->addPrmType(pp->tipoDi(q));
	ft->setRetType(type);
	declared=ft;
  }
  interpreter.context->add(value,declared,p);
  // --- salta il codice della procedura
  pc=interpreter.pc;
  //cout << "cerca fine proc " << pc << endl;
  int c=interpreter.prg->get(pc++).getCode();
  while (c!=P_ENDPROC && c!=P_ENDFUNC)
	c=interpreter.prg->get(pc++).getCode();
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

void pcodeClass::exec(interp& interpreter) const {
  cout << "declaring class " << theStringIntern.get(value) << endl;
  classObj* c=new classObj(value,interpreter);
  // aggiunge l'oggetto al modulo (container corrente)
  interpreter.context->add(value,c,c); // il tipo è la classe stessa
  // sostituisce il container corrente in modo che variabili, procedure e funzioni vengano aggiunte alla classe	
  interpreter.saved_ctx=interpreter.context;
  interpreter.context=c;
}

void pcodeInherit::exec(interp& interpreter) const {
  cout << "inherith " << value << endl;	
}

void pcodeEndClass::exec(interp& interpreter) const {
  cout << "end declaring class " << endl;
  // ripristina il contaner del modulo
  interpreter.context=interpreter.saved_ctx;
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
  interp intp(ctx,&prg);
  theStringIntern.add("i");

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
  
  lockgc_ptr<intObj> c_1(new intObj(1));
  lockgc_ptr<intObj> c_10000000(new intObj(10000000));
      
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
  intp.stack.push_back(c_10000000); intp.sp++; //prg.get(8)->exec(intp);  // int const 1000000
  
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

static lockgc_ptr<intObj> c_1(new intObj(1));
static lockgc_ptr<intObj> c_2(new intObj(2));
 
void fib_cc(interp& intp){
  lockgc_ptr<obj> n;
  // inizializza il parametro n
  n=intp.stack[intp.sp];
  intp.stack.pop_back();
  intp.sp--;
  //cout << "dopo n sp:" << intp.sp << " n:" << n->print() << endl;
  // n<2
  intp.stack.push_back(n);intp.sp++;
  intp.stack.push_back(c_2);intp.sp++;
  intp.stack[intp.sp-1]=intp.stack[intp.sp-1]->lt(intp.stack[intp.sp]);
  intp.stack.pop_back();
  intp.sp--;
  // if false 0
  bool b0=intp.stack[intp.sp--]==theFalse;
  intp.stack.pop_back();
  if (b0) goto label0;
  // result:=1
  intp.stack.push_back(c_1);intp.sp++;
  goto label1;  
  label0:
  // n-1
  intp.stack.push_back(n);intp.sp++;
  intp.stack.push_back(c_1);intp.sp++;
  intp.stack[intp.sp-1]=intp.stack[intp.sp-1]->minus(intp.stack[intp.sp]);
  intp.stack.pop_back();
  intp.sp--;  
  // fib
  fib_cc(intp);
  // n-2
  intp.stack.push_back(n);intp.sp++;
  intp.stack.push_back(c_2);intp.sp++;
  intp.stack[intp.sp-1]=intp.stack[intp.sp-1]->minus(intp.stack[intp.sp]);
  intp.stack.pop_back();
  intp.sp--;  
  // fib
  fib_cc(intp);
  // +
  intp.stack[intp.sp-1]=intp.stack[intp.sp-1]->plus(intp.stack[intp.sp]);
  intp.stack.pop_back();
  intp.sp--;    
  label1:
  //cout << "fine sp:" << intp.sp << " result:" << intp.stack[intp.sp]->print() << endl;
  ;;
} 

void test_fib_cc(){
  pcodeProgram prg;	
  contextObj* ctx=new contextObj();
  interp intp(ctx,&prg);
  //
  lockgc_ptr<obj> c_34(new intObj(34));  
  lockgc_ptr<obj> str1(new strObj("fib(34)="));
  // push str
  intp.stack.push_back(str1);
  intp.sp++;  
  // push int const 34
  intp.stack.push_back(c_34);
  intp.sp++;  
  // call "fib"
  fib_cc(intp);  
  //cout << "terminato sp:" << intp.sp << " result:" << intp.stack[intp.sp]->print() << endl;;
  // print 1
  pcodePrint pp(2);
  pp.exec(intp);
}  

int bench_fib_cc(){
  ankerl::nanobench::Bench().run("fib(34) in cc", [&] {
	test_fib_cc();
  });
  return 0;
}

// D2: dal nome scritto in command line al percorso del p-code da caricare.
// E' la regola opposta di pcodePath() del compilatore: li' si sostituisce
// l'estensione, qui se nel nome di base non c'e' alcun punto si accoda ".pcd",
// altrimenti il percorso resta quello digitato (vedi anche D6: nessun flag).
// Il punto si cerca solo nel nome di base, quindi "dir.d/x" resta "dir.d/x.pcd".
string pcodePathFor(const string& arg){
  size_t slash=arg.find_last_of('/');
  string base=slash==string::npos?arg:arg.substr(slash+1);
  if (base.find_last_of('.')==string::npos) return arg+".pcd";
  return arg;
}

int runPCode(pcodeProgram& prg){
  lockgc_ptr<contextObj> ctx{new contextObj()};
  interp intp(ctx,&prg);
  intp.run();
  assert(intp.sp==-1);
  return 1;
}

int runPcd(const string& fn){
  pcodeProgram prg;
  int r=prg.loadPcd(fn);  
  if (r) {
    theSys.currentDir = theSys.dirOfPath(fn);
    if (theSys.currentDir.empty()) theSys.currentDir = ".";
    runPCode(prg);
  }
  return r?0:1;
}

string extractModName(const string& fn){
  string res;
  size_t p=fn.rfind('/');
  if (p==string::npos)
    res=fn;
  else
    res=fn.substr(p+1);
  return res.substr(0,res.length()-4);    
}
	
int runModule(const string& fn){
   // setta la directory di partenza	
   theSys.currentDir = theSys.dirOfPath(fn);
   if (theSys.currentDir.empty()) theSys.currentDir = ".";
   string modname = extractModName(fn);
   //cout << "path:" << theSys.currentDir << " fn:" << fn << " modname:" << modname << endl;   
   int r=theSys.execModule(modname);
   return r?0:1;
}

int benchPcd(const string& fn){
  ankerl::nanobench::Bench().run("esegue "+fn, [&] {
	runModule(fn);
	theSys.unloadModules();
  });
  return 0;
}

int bench_cc(){
  ankerl::nanobench::Bench().run("conta fino a 10000000 in cc", [&] {
	test_cc();
  });
  return 0;
}

void releaseSysObjs(){
  //cout << "--- removing system objects ---\n";
  theSys.release();
  // 
  theNil=nullptr;
  theTrue=nullptr;
  theFalse=nullptr;
  theIntType=nullptr;
  theStrType=nullptr;
  theBoolType=nullptr;
  theFloatType=nullptr;
  theFloatSonda=nullptr;
  theBuiltIn=nullptr;
  //
  c_1=nullptr;
  c_2=nullptr;
}

void test_cc_translation(){
  //test("primo.pcd");
  //bench("primo.pcd");
  //test("terzo.pcd");
  runModule("cnt10000000.pcd");
  benchPcd("cnt10000000.pcd");
  test_cc();
  bench_cc();
  runModule("fib.pcd");
  benchPcd("fib.pcd");
  test_fib_cc();
  bench_fib_cc();
}

// D7: l'opzione di misura si scrive con due grafie, `--bench` e `-b`: il nome
// lungo e il nome corto. Il terzo campo della riga resta vuoto perche' la terza
// grafia che c'era prima, `-bench` con un solo trattino, e' sparita: non e' un
// nome lungo ne' un nome corto, e' un residuo del confronto con argv[1] che
// precede la tabella. `-bench` ora e' un'opzione sconosciuta come `-z`, e resta il
// ramo CLI_ERRORE_SCONOSCIUTA a segnalarla.
// Tabella e variabili sono fuori da main() perche' il campo `dest` deve puntare
// a qualcosa che sopravviva alla chiamata.
static int benchOpzione=0;
static int helpOpzione=0;
static int testOpzione=0;
#define N_OPT 5
static const cliOpzione tabOpzioni[N_OPT]={
  {"bench",'b',NULL,"misura il file invece di eseguirlo una volta",CLI_OPZIONE,&benchOpzione},
  {"help",'h',NULL,"stampa queste istruzioni d'uso",CLI_OPZIONE,&helpOpzione},
  {"test",'t',NULL,"esegue i test standard",CLI_OPZIONE,&testOpzione},
  {"gc",'g',NULL,"stampa il report del garbage collector",CLI_OPZIONE,&gcOpzione},
  {"typecheck",'c',NULL,"accende il check di tipo nelle assegnazioni",CLI_OPZIONE,&tipocheckOpzione}
};

// D8 (libera): l'elenco delle opzioni non e' scritto a mano, arriva dalla stessa
// tabella che usa la scansione, quindi un flag nuovo compare nell'uso senza toccare
// qui.
// D2: il testo dell'uso e' uno solo ed esce su due stream: stderr quando accompagna
// un errore di command line (codice 1), stdout quando l'utente lo chiede con `-h`
// (codice 0). Per questo `uso()` riceve il FILE* su cui scrivere e non sceglie da
// se', e `cliOpzioni()` riceve lo stesso puntatore.
static void uso(FILE* out){
  fprintf(out,"uso: interpreter [opzioni] <file.pcd>\n");
  cliOpzioni(tabOpzioni,N_OPT,out);
  fprintf(out,"  un nome senza estensione viene completato con .pcd\n");
  fprintf(out,"  dopo -- ogni argomento e' un percorso\n");
}

// D1: tre controlli in sequenza, e non uno. Prima gli errori della scansione (opzione
// sconosciuta, valore mancante, valore non accettato): sono un errore di sintassi
// della riga e vanno segnalati anche se l'utente ha scritto `-h`, quindi il ramo
// dell'errore non cede. Poi l'help richiesto esplicitamente: stampa su stdout e
// esce con 0 senza eseguire ne' misurare nulla, e cede sul numero di percorsi perche'
// l'utente che scrive `-h fib.pcd` vuole leggere le istruzioni, non provare a
// mettere le mani su `fib.pcd`. Infine il numero di percorsi: ne deve essere
// esattamente uno.
// D1/D3: le opzioni si riconoscono in qualunque posizione e "--" le termina,
// quindi non serve piu' contare le forme della command line. Un argomento che
// comincia con "-" e non e' il solo "-" non e' piu' un percorso, quindi
// "./interpreter -h" non prova piu' ad aprire il file di nome "-h", e
// "./interpreter -bench fib.pcd" non prova piu' a misurare: `-bench` non e' piu'
// una grafia dell'opzione di misura ed e' segnalata come sconosciuta.
int main(int argc, char** argv){
  const char* percorsi[N_OPT];
  cliScansione sc;
  int nPercorsi;
  benchOpzione=0;
  helpOpzione=0;
  testOpzione=0;
  gcOpzione=0;
  tipocheckOpzione=0;
  cliInizia(&sc,argc,argv);
  nPercorsi=cliScansiona(&sc,tabOpzioni,N_OPT,percorsi,N_OPT);
  if (sc.errore!=CLI_ERRORE_NESSUNO) {
    switch (sc.errore) {
      case CLI_ERRORE_VALORE:
        cerr << "l'opzione " << sc.erroreNome << " richiede un valore" << endl;
        break;
      case CLI_ERRORE_NONVALORE:
        cerr << "l'opzione " << sc.erroreNome << " non accetta un valore: " << sc.erroreValore << endl;
        break;
      case CLI_ERRORE_SCONOSCIUTA:
        cerr << "opzione sconosciuta: " << sc.erroreNome << endl;
        break;
      default:
        break;
    }
    uso(stderr);
    return 1;
  }
  int rc=-1;
  // D4: l'help esce qui, prima di pcodePathFor(), quindi non deriva il percorso, non
  // carica nessun file e non stampa il rapporto del GC che segue la run.
  if (helpOpzione) {
    uso(stdout);
    return 0;
  } else if (testOpzione==1){
	test_cc_translation();
  } else if (nPercorsi!=1) {
    uso(stderr);
    return 1;
  } else {
    // D2: la derivazione resta qui, una volta sola, cosi' la tabella del bench mostra
    // il percorso realmente caricato e non quello digitato.
    if (!benchOpzione)
      rc=runModule(pcodePathFor(percorsi[0]));
    else 
      rc=benchPcd(pcodePathFor(percorsi[0]));
  }
  if (gcOpzione) {
    cout << "--- status on exit ---\n";  
    stdGC().status();
    //cout << "--- collect 0 ---\n";  
    //stdGC().collect(0);
    //stdGC().status();
  }
  releaseSysObjs();
  stdGC().collectall();
  if (gcOpzione || stdGC().size()){
    stdGC().status();
    cout << "--- status after destruction of system objects ---\n";   
    stdGC().status();
    cout << "--- final status,locked objs ---\n";
    stdGC().printLocked();
    cout << "--- end ---\n";
  }
  return rc;
}
