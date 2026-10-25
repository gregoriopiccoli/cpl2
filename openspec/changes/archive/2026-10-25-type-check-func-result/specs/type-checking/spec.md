# Spec Delta

## ADDED Requirements

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
