#pragma once

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

class pcodeToDo: public spcode {
public:
  explicit pcodeToDo(const string& v):spcode(v){code=P_TODO;}
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

class pcodeImplements: public ipcode {
public:
  explicit pcodeImplements(int v):ipcode(v){code=P_IMPLEMENTS;}
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
