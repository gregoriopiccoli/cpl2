# type-checking Specification

## Purpose

Enforce type compatibility for variable declarations and assignments. Every variable has associated type information, and assignments must be checked to ensure the assigned value's type is compatible with the variable's declared type.

## Requirements

### Requirement: Variables must have associated type information
The system SHALL associate each declared variable with its declared type. Type information MUST be stored and accessible for type checking purposes.

#### Scenario: Variable declared with type
- **WHEN** a variable is declared with a type (e.g., `var int i`)
- **THEN** the variable is registered with its declared type information

### Requirement: Assignment to variables must be type-compatible
The system SHALL check type compatibility when assigning values to variables. If a value's type is not compatible with the variable's declared type, the assignment MUST fail with an error.

#### Scenario: Compatible assignment
- **WHEN** a variable of type `int` is assigned an `int` value
- **THEN** the assignment succeeds

#### Scenario: Incompatible assignment
- **WHEN** a variable of type `int` is assigned a value of incompatible type (e.g., `str`)
- **THEN** the assignment fails with a type error

### Requirement: Initialization at declaration must be type-compatible
The system SHALL check type compatibility when initializing a variable at declaration time. If the initializer's type is not compatible with the declared type, the declaration/initialization MUST fail with an error.

#### Scenario: Compatible initialization
- **WHEN** a variable is declared and initialized with a compatible value (e.g., `var int i := 5`)
- **THEN** the initialization succeeds

#### Scenario: Incompatible initialization
- **WHEN** a variable is declared and initialized with an incompatible value
- **THEN** the initialization fails with a type error

### Requirement: Type compatibility follows the type system rules
Type compatibility SHALL respect nil/nullable semantics. A type declared as non-nil (e.g., using `not`) SHALL not accept `nil` values unless explicitly allowed by compatibility rules. Union types (`|`) SHALL accept values compatible with any of the union members.

#### Scenario: Nil assignment to non-nil type
- **WHEN** a variable is declared as non-nil type (e.g., `var int not i`) and assigned `nil`
- **THEN** the assignment fails with a type error

#### Scenario: Union type assignment
- **WHEN** a variable is declared as a union type (e.g., `var int|str x`) and assigned a value compatible with one of the members
- **THEN** the assignment succeeds

### Requirement: Assignment to a function result must be type-compatible
The system SHALL check type compatibility when a function assigns a value to its implicit `result` variable, using the function's declared return type and the same compatibility rules as variable assignment. If the value is not compatible, the assignment MUST fail with a type error naming `result`, and the program MUST stop as it does for any other rejected assignment.

#### Scenario: Compatible result assignment
- **WHEN** type checking is enabled and a function declared `func int f()` executes `result := 5`
- **THEN** the assignment succeeds and the call returns `5`

#### Scenario: Incompatible result assignment
- **WHEN** type checking is enabled and a function declared `func int f()` executes `result := "pippo"`
- **THEN** the assignment fails with a type error reporting `result: il valore pippo non e' compatibile con il tipo int`, and the program stops

#### Scenario: Union return type
- **WHEN** type checking is enabled and a function declared `func int|str f()` assigns either an `int` or a `str` to `result`
- **THEN** the assignment succeeds

#### Scenario: Nil result in a non-nil return type
- **WHEN** type checking is enabled and a function whose return type is declared non-nil (e.g., `func int not f()`) executes `result := nil`
- **THEN** the assignment fails with a type error

#### Scenario: Nil result in a nullable return type
- **WHEN** type checking is enabled and a function declared `func int f()` executes `result := nil`
- **THEN** the assignment succeeds

#### Scenario: Any return type
- **WHEN** type checking is enabled and a function declared `func ? f()` assigns a value of any type to `result`
- **THEN** the assignment succeeds

#### Scenario: Type checking disabled
- **WHEN** type checking is not enabled and a function declared `func int f()` executes `result := "pippo"`
- **THEN** the assignment succeeds without any check, as for every other assignment

### Requirement: An int assigned to a float-typed name is promoted to float
When an `int` value is assigned to a name whose declared type requires a float, the system SHALL store a float with the same numeric value instead of the int. A declared type requires a float when it is `float`, `not float`, or a union that has `float` among its alternatives and does not already accept the int as it is. The promotion SHALL apply to variable initialization and assignment, module names, proc/func parameters and a func's `result`.

#### Scenario: Initialization of a float variable with an int
- **WHEN** the program `var float f := 0` followed by `? f` and `? (f is float)` is executed
- **THEN** the output is `0.0` followed by `true`

#### Scenario: Assignment of an int to a float variable
- **WHEN** the program `var float f := 1.5` followed by `f := 3` and `? f / 2` is executed
- **THEN** the output is `1.5` (float division, not integer division)

#### Scenario: Mixed arithmetic keeps working after the promotion
- **WHEN** the program `var float f1 := 10.12`, `var float f2 := 0`, `? f1 + f2` is executed
- **THEN** the output is `10.12`

#### Scenario: Float parameter receiving an int argument
- **WHEN** a function declared `func float meta(float x)` with body `result := x / 2` is called as `meta(3)`
- **THEN** the call returns `1.5`

#### Scenario: Float result receiving an int
- **WHEN** a function declared `func float uno()` executes `result := 1` and `? uno()` is executed
- **THEN** the output is `1.0`

#### Scenario: Non-nil float type
- **WHEN** the program `var float not f := 2` followed by `? f` is executed
- **THEN** the output is `2.0`

#### Scenario: Union with float and without int
- **WHEN** the program `var float|str x := 1` followed by `? x` is executed
- **THEN** the output is `1.0`

#### Scenario: Union that already accepts the int
- **WHEN** the program `var int|float x := 1` followed by `? x` and `? (x is int)` is executed
- **THEN** the output is `1` followed by `true`

#### Scenario: Untyped and any variables keep the int
- **WHEN** the program `var ? v := 1` followed by `? v` is executed
- **THEN** the output is `1`

#### Scenario: Nil is not promoted
- **WHEN** the program `var float f := 1.0` followed by `f := nil` and `? f` is executed
- **THEN** the output is `nil`

### Requirement: The int-to-float promotion does not depend on type checking
The promotion of an `int` assigned to a float-typed name SHALL happen whether or not type checking is enabled. With type checking enabled, an `int` assigned to a float-typed name SHALL NOT be reported as a type error.

#### Scenario: Promotion without type checking
- **WHEN** type checking is not enabled and the program `var float f := 0` followed by `? f` is executed
- **THEN** the output is `0.0`

#### Scenario: Promotion with type checking
- **WHEN** type checking is enabled and the program `var float f := 0` followed by `? f` is executed
- **THEN** the assignment succeeds without a type error and the output is `0.0`

### Requirement: Only int is promoted, and only to float
The system SHALL NOT convert any other value on assignment: a float assigned to an `int` name, a `str` or `bool` assigned to a `float` name, and the elements of a typed array, dict or tuple SHALL be stored unchanged, with type checking applying the existing compatibility rules to them.

#### Scenario: Float assigned to an int variable
- **WHEN** type checking is enabled and the program `var int i := 1.5` is executed
- **THEN** the assignment fails with a type error reporting `i: il valore 1.5 non e' compatibile con il tipo int`

#### Scenario: String assigned to a float variable
- **WHEN** type checking is enabled and the program `var float f := "a"` is executed
- **THEN** the assignment fails with a type error reporting `f: il valore a non e' compatibile con il tipo float`

#### Scenario: Elements of a typed array are not promoted
- **WHEN** the program `var array[float] a := [1, 2]` followed by `? a` is executed
- **THEN** the elements are printed as the ints `1` and `2`
