#pragma once

/*

  Le classi per i vari tipi di base: nil, Int, Str, Bool, Float,
  
  Ogni tipo di base ha una classe che implementa le operazioni. La base "obj" non implememta nessuna operazioni, i tipi
  di base implmentano tutte le operazioni.
  
  Per ogni tipo di base viene creata una classe, alcune classi hanno una lista per ricilare i valori già creati.
  Questo permette di non allocare nuova memoria (operazione lenta) per tanti nuovi oggetti, ma viene riusato un oggetto dismesso
  in precedenza.

*/

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
  virtual void reclaim() override;
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

