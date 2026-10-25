# Spec Delta

## ADDED Requirements

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
