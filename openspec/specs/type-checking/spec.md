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
