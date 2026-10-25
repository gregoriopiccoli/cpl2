# Proposal

## Why

Currently variables are declared without enforcing type compatibility on assignment. The language already has a rich type system (int, str, bool, float, arrays, dicts, tuples, or-types, proc/func types) and types are represented on the stack/objects, but there's no runtime/static type checking enforcing that assigned values match declared types. Adding type checking will make the language safer and catch errors like assigning wrong types to variables.

## What Changes

- **Type information in variable declarations**: Track declared types for variables. Variables need to store/associate their type information.
- **Type compatibility checking on assignment**: When assigning to variables (via `P_STORE`, `P_VAR_STORE`, etc.), verify that the value's type is compatible with the variable's declared type.
- **Type checking in variable declaration/initialization**: Ensure values assigned at declaration time match declared types.
- **Support for nil/nullable types**: Need to handle `btype NOT` (types that reject nil) vs nullable types correctly.
- **Type checking infrastructure**: Add mechanisms to determine value types and check compatibility between types.

## Capabilities

### New Capabilities
- `type-checking`: Enforce type compatibility for variable declarations and assignments. Each variable has associated type information, and assignments are checked against declared types.

### Modified Capabilities
(None)

## Impact

- **Grammar** (`cpl2.gram`): May need changes to how types are associated with variable declarations
- **Interpreter** (`interpreter.cpp`): Changes to `pcodeVar`, `pcodeVarStore`, `pcodeStore`, context management to track types and perform checks
- **P-code** (`pcodes.*`): Possibly new p-codes or changes to existing ones
- **Type system**: Leverage existing type objects (`intType`, `strType`, etc.) and need compatibility logic
