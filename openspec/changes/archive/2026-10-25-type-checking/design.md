# Design

## Context

See proposal.md - Why. The language already has a type system in the interpreter (type objects like `intType`, `strType`, `arrayType`, `orType`, etc.) and the grammar emits type p-codes (`P_INT_TYPE`, `P_STR_TYPE`, etc.) when parsing types. Variables are stored in `contextObj` which currently tracks name->value mappings and also maintains a separate `types` map (`unordered_map<int,obj*> types`) and has `add(int intern, obj* type)` / `add(int intern, obj* type, obj* value)` methods. The p-codes `P_VAR`, `P_VAR_STORE`, `P_STORE` currently don't do type checking. The request says "fare il controllo di tipo, ogni variabile ha un puntatore al tipo e quando viene fatta una assegnazione bisognerebbe controllare che il tipo del valore che viene assegnato sia compatbile con quello dichiarato" - so every variable has a pointer/reference to its type and assignments must be checked for compatibility.

## Goals / Non-Goals

**Goals:**
- Track type information for each variable (already partially exists in context as `types` map)
- Check type compatibility on variable assignment (`P_STORE`, `P_STOREX`, etc.)
- Check type compatibility on variable declaration/initialization (`P_VAR_STORE`)
- Define type compatibility rules (exact match for base types, handling nil/nullable with `NOT`, union types with `|`, etc.)
- Report type errors when incompatible

**Non-Goals:**
- Full static type checking across all contexts
- Changing the entire type system architecture
- Modifying unrelated parts of the language

## Decisions

### Decision 1: Where to store type information
The interpreter already has `contextObj` with `unordered_map<int,obj*> types` and methods to add with types. We need to ensure types are properly stored and accessible. Also need to be able to retrieve the declared type of a variable during assignment.

### Decision 2: Type compatibility checking
We need a function to check if a value is compatible with a declared type. For base types, compatibility likely means same base type or compatible types. For union types (`orType`), the value must be compatible with at least one alternative. For types with `NOT` (non-nil), nil values must be rejected. For `any`/`?`, they accept anything.

### Decision 3: When to check
- `P_VAR` (declaration without initialization): adds variable with nil value and declared type - need to ensure type is passed correctly
- `P_VAR_STORE` (declaration with initialization): has both type and value - must check compatibility
- `P_STORE` (assignment to variable): must check if new value is compatible with declared type
- `P_STOREX` (field assignment): similar checking needed
- `P_STORESLICE`: for array/dict/tuple elements - may need different rules

### Decision 4: How types flow in p-code
Looking at grammar: `vardecl ::= VAR type var_list const local.` - the type is parsed and emitted via type p-codes (e.g., `P_INT_TYPE` pushes `theIntType` onto stack). Then `var_list` does `P_VAR`/`P_VAR_STORE`. So the type is likely on the stack when `P_VAR`/`P_VAR_STORE` execute? Let us look up how it's actually done.

Looking up how P_VAR is used - in grammar line ~164-167, `P_VAR` takes the variable name as operand (emitted via `emit_s(P_VAR,value)`). The type comes from parsing `type` before it - `type` pushes type objects onto stack (e.g., `btype INT` does `emit(P_INT_TYPE)`). So when declaring variables, the type object must be on the stack. Also looking at how it's done in interpreter: `pcodeVar::exec` does `interpreter.context->add(value,interpreter.stack[interpreter.sp]);` - it passes the type from stack. And `pcodeVarStore` does `interpreter.context->add(value,interpreter.stack[interpreter.sp-1],interpreter.stack[interpreter.sp]);` passing type and value.

But wait - looking at the actual code again:
```cpp
void pcodeVar::exec(interp& interpreter) const {
  interpreter.context->add(value,interpreter.stack[interpreter.sp]);
}
```
It adds with just type? Let us look up the context.add signatures again.
```cpp
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
```
So both forms take type. For `P_VAR` (just declaration, maybe initialized to nil?), it adds with type and sets value to nil. For `P_VAR_STORE`, it adds with type and value from stack.

Also looking at grammar again: in `var_list` rules, when doing just `ID(value)` it emits `P_VAR(value)`. When doing `ID(value) ASSIGN lexpr` it emits `P_VAR_STORE(value)`. The `type` was parsed earlier in `vardecl ::= VAR type var_list const local.` - so the type was pushed onto stack by type parsing rules, and remains there for var_list to consume? Let us trace: `type` rules like `btype ::= INT.` do `emit(P_INT_TYPE)` which pushes type onto stack. So for `var int i`, the type `int` is pushed by the type parsing, then `P_VAR` consumes it. That matches.

Similarly for `P_VAR_STORE`, it needs both type and value - so stack would have [type, value] and it consumes both.

Now for assignments like `i := 5` which becomes `aexpr ::= ID(value) ASSIGN expr.` emitting `P_STORE(value)`. `P_STORE` does `interpreter.context->store(value,interpreter.stack[interpreter.sp]);` - it just stores the value, no type checking. Same for `P_STOREX`. So we need to add type checking there.

Also need to handle the case where a variable might not have type info? Probably all variables should have types.

## Risks / Trade-offs

- **Performance**: Type checking adds overhead to every assignment
- **Compatibility**: Need to get the compatibility rules right - the language has complex types (union types, nilable types, etc.)
- **Integration**: Need to add helper functions for type compatibility checking
