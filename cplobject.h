#pragma once

// --- gli oggetti dell'esecuzione dell'interprete

class obj : public GCObject {
public:
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
  virtual int procType() {return 0;}
  //
  virtual const obj* getBaseType() const;
};
