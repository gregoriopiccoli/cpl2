#include <string>
#include <vector>

using namespace std;

class interp;

#include "gc.h"
#include "recycler.h"
#include "cplobject.h"
#include "basetypes.h"

const obj* obj::getBaseType() const {return theNil;}

recycler<intObj> theIntRecycler;

void intObj::reclaim(){
  theIntRecycler.add(this);
}

void showIntCache(){cout << "intCache size:" << theIntRecycler.size() << " recycled:" << theIntRecycler.recycled << endl;}

obj* intObj::plus(const obj* o) const {
  if (!is_int(o)) return floatPlus(value,to_float(o,"integer + with a non integer"));
  const intObj* oo=static_cast<const intObj*>(o);
  if (theIntRecycler.size()>0) {intObj* v=theIntRecycler.get();v->value=value+oo->value;stdGC().addRecycled(v);return v;}
  return new intObj(value+oo->value);
}

obj* intObj::minus(const obj* o) const {
  if (!is_int(o)) return floatMinus(value,to_float(o,"integer - with a non integer"));
  const intObj* oo=static_cast<const intObj*>(o);
  if (theIntRecycler.size()>0) {intObj* v=theIntRecycler.get();v->value=value-oo->value;stdGC().addRecycled(v);return v;}
  return new intObj(value-oo->value);
}

obj* intObj::uminus() const {
  // negazione in aritmetica senza segno: -INT_MIN fa il giro senza comportamento indefinito
  int r=(int)(0u-(unsigned)value);
  if (theIntRecycler.size()>0) {intObj* v=theIntRecycler.get();v->value=r;stdGC().addRecycled(v);return v;}
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



obj* strObj::plus(const obj* o) const {
  const strObj* oo=check_str(o,"str + with a non str");
  return new strObj(value + oo->value);
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

// i singleton degli oggetti che non richiedono tante copie ...
lockgc_ptr<obj> theTrue{new boolObj(true)};
lockgc_ptr<obj> theFalse(new boolObj(false));
lockgc_ptr<obj> theNil(new nilObj());

