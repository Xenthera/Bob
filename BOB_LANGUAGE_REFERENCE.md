# Bob Language Reference

## Overview

Bob is a dynamically typed programming language with a focus on simplicity and expressiveness. It features automatic type conversion, closures, and a clean syntax inspired by modern programming languages.

## Table of Contents

1. [Data Types](#data-types)
2. [Variables](#variables)
3. [Operators](#operators)
4. [Functions](#functions)
5. [Control Flow](#control-flow)
6. [Standard Library](#standard-library)
7. [Error Handling](#error-handling)
8. [Examples](#examples)
9. [Language Nuances](#language-nuances)

## Data Types

### Numbers
- **Integers**: `42`, `-10`, `0`
- **Floats**: `3.14`, `2.718`, `-1.5`
- **Automatic conversion**: Numbers are stored as doubles internally

### Strings
- **Literal strings**: `"Hello, World!"`
- **Empty strings**: `""`
- **Escape sequences**: Not currently supported

### Booleans
- **True**: `true`
- **False**: `false`

### None
- **Null value**: `none` (represents absence of value)

## Variables

### Declaration
```bob
var name = "Bob";
var age = 25;
var isActive = true;
```

### Assignment
```bob
var x = 10;
x = 20;  // Reassignment
```

### Scoping
- **Global scope**: Variables declared at top level
- **Local scope**: Variables declared inside functions
- **Shadowing**: Local variables can shadow global variables
- **No `global` keyword**: Unlike Python, Bob doesn't require explicit global declaration

### Variable Behavior
```bob
var globalVar = 100;

func testScope() {
    var localVar = 50;  // Local variable
    return localVar + globalVar;  // Can access global
}
```

## Operators

### Arithmetic Operators
- **Addition**: `+`
- **Subtraction**: `-`
- **Multiplication**: `*`
- **Division**: `/`
- **Modulo**: `%`

### Comparison Operators
- **Equal**: `==`
- **Not equal**: `!=`
- **Greater than**: `>`
- **Less than**: `<`
- **Greater than or equal**: `>=`
- **Less than or equal**: `<=`

### String Operators

#### Concatenation
Bob supports bidirectional string + number concatenation with automatic type conversion:

```bob
// String + Number
"Hello " + 42;           // → "Hello 42"
"Pi: " + 3.14;           // → "Pi: 3.14"

// Number + String
42 + " items";           // → "42 items"
3.14 + " is pi";         // → "3.14 is pi"
```

#### String Multiplication
```bob
"hello" * 3;             // → "hellohellohello"
3 * "hello";             // → "hellohellohello"
```

**Note**: String multiplication requires whole numbers.

### Number Formatting
Bob automatically formats numbers to show only significant digits:

```bob
"Count: " + 2.0;         // → "Count: 2" (no trailing zeros)
"Pi: " + 3.14;           // → "Pi: 3.14" (exact precision)
"Integer: " + 42;        // → "Integer: 42" (no decimal)
```

## Functions

### Function Declaration
```bob
func add(a, b) {
    return a + b;
}
```

### Function Call
```bob
var result = add(2, 3);  // result = 5
```

### Parameters
- **Any number of parameters** supported
- **No default parameters** (not implemented)
- **No keyword arguments** (not implemented)

### Return Values
- **Explicit return**: `return value;`
- **Implicit return**: Functions return `none` if no return statement
- **Early return**: Functions can return from anywhere

### Closures
Bob supports lexical closures with variable capture:

```bob
var outerVar = "Outer";

func makeGreeter(greeting) {
    return greeting + " " + outerVar;
}

var greeter = makeGreeter("Hello");
// greeter captures outerVar from its lexical scope
```

### Nested Functions
```bob
func outer() {
    func inner() {
        return 42;
    }
    return inner();
}
```

## Control Flow

### Current Status
**Control flow statements are NOT implemented yet:**
- `if` statements
- `while` loops
- `for` loops
- `else` clauses

### Planned Features
- Conditional execution
- Looping constructs
- Logical operators (`and`, `or`, `not`)

## Standard Library

### Print Function
```bob
print("Hello, World!");
print(42);
print(true);
```

**Usage**: `print(expression)`
- Prints the result of any expression
- Automatically converts values to strings
- Adds newline after output

### Assert Function
```bob
assert(condition, "optional message");
```

**Usage**: 
- `assert(true)` - passes silently
- `assert(false)` - throws error and stops execution
- `assert(condition, "message")` - includes custom error message

**Behavior**:
- Terminates program execution on failure
- No exception handling mechanism (yet)
- Useful for testing and validation

## Error Handling

### Current Error Types
- **Division by zero**: `DivisionByZeroError`
- **Type errors**: `Operands must be of same type`
- **String multiplication**: `String multiplier must be whole number`
- **Assertion failures**: `Assertion failed: condition is false`

### Error Behavior
- **No try-catch**: Exception handling not implemented
- **Program termination**: Errors stop execution immediately
- **Error messages**: Descriptive error messages printed to console

### Common Error Scenarios
```bob
// Division by zero
10 / 0;  // Error: DivisionByZeroError

// Type mismatch
"hello" - "world";  // Error: Cannot use '-' on two strings

// Invalid string multiplication
"hello" * 3.5;  // Error: String multiplier must be whole number

// Undefined variable
undefinedVar;  // Error: Undefined variable
```

## Examples

### Basic Calculator
```bob
func add(a, b) {
    return a + b;
}

func multiply(a, b) {
    return a * b;
}

var result = add(5, multiply(3, 4));
print("Result: " + result);  // Result: 17
```

### String Processing
```bob
func greet(name) {
    return "Hello, " + name + "!";
}

func repeat(str, count) {
    return str * count;
}

var greeting = greet("Bob");
var repeated = repeat("Ha", 3);
print(greeting + " " + repeated);  // Hello, Bob! HaHaHa
```

### Variable Scoping Example
```bob
var globalCounter = 0;

func increment() {
    var localCounter = 1;
    globalCounter = globalCounter + localCounter;
    return globalCounter;
}

print("Before: " + globalCounter);  // Before: 0
increment();
print("After: " + globalCounter);   // After: 1
```

## Language Nuances

### Type System
- **Dynamic typing**: Variables can hold any type
- **Automatic conversion**: Numbers and strings convert automatically
- **No type annotations**: Types are inferred at runtime

### Memory Management
- **Reference counting**: Uses `std::shared_ptr` for automatic memory management
- **No manual memory management**: No `delete` or `free` needed
- **Garbage collection**: Automatic cleanup of unused objects

### Performance Characteristics
- **Interpreted**: Code is executed by an interpreter
- **AST-based**: Abstract Syntax Tree for execution
- **No compilation**: Direct interpretation of source code

### Syntax Rules
- **Semicolons**: Required at end of statements
- **Parentheses**: Required for function calls
- **Curly braces**: Required for function bodies
- **Case sensitive**: `var` and `Var` are different

### Comparison with Other Languages

#### vs Python
- **Similar**: Dynamic typing, functions, closures
- **Different**: No `global` keyword, automatic string conversion, different error handling

#### vs JavaScript
- **Similar**: Automatic type conversion, string concatenation
- **Different**: No `undefined`, different syntax, no `null`

#### vs Lua
- **Similar**: Dynamic typing, functions
- **Different**: No `local` keyword, different scoping rules

### Limitations
- **No control flow**: No if/while/for statements
- **No logical operators**: No `and`/`or`/`not`
- **No exception handling**: No try-catch blocks
- **No modules**: No import/export system
- **No classes**: No object-oriented features
- **No arrays/lists**: No built-in collection types
- **No dictionaries**: No key-value data structures

### Future Features
- Control flow statements
- Logical operators
- Exception handling
- Collection types (arrays, dictionaries)
- Modules and imports
- Object-oriented programming
- Standard library expansion

## Getting Started

### Running Bob Code
```bash
# Compile the interpreter
make

# Run a Bob file
./build/bob your_file.bob

# Run the comprehensive test suite
./build/bob test_bob_language.bob
```

### File Extension
- **`.bob`**: Standard file extension for Bob source code

### Interactive Mode
- **Not implemented**: No REPL (Read-Eval-Print Loop) yet
- **File-based**: All code must be in `.bob` files

---

*This documentation covers Bob language version 1.0. For the latest updates, check the ROADMAP.md file.* 