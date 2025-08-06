# Bob Language Reference

## Overview

Bob is a dynamically typed programming language focused on safety and clarity. It features automatic type conversion, closures, and a clean syntax inspired by modern programming languages.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Basic Syntax](#basic-syntax)
3. [Data Types](#data-types)
4. [Variables](#variables)
5. [Operators](#operators)
6. [Control Flow](#control-flow)
7. [Functions](#functions)
8. [Standard Library](#standard-library)
9. [Error Handling](#error-handling)
10. [Examples](#examples)

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

## Basic Syntax

### Statements
- **Semicolons**: Required at end of statements
- **Parentheses**: Required for function calls
- **Curly braces**: Required for function bodies
- **Case sensitive**: `var` and `Var` are different

### Comments
```go
// Single line comment
/* Multi-line comment */
```

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
```go
var name = "Bob";
var age = 25;
var isActive = true;
```

### Assignment
```go
var x = 10;
x = 20;  // Reassignment
```

### Scoping
- **Global scope**: Variables declared at top level
- **Local scope**: Variables declared inside functions
- **Shadowing**: Local variables can shadow global variables
- **No `global` keyword**: Unlike Python, Bob doesn't require explicit global declaration

```go
var globalVar = 100;

func testScope() {
    var localVar = 50;  // Local variable
    return localVar + globalVar;  // Can access global
}
```

### Assignment System

Bob uses two assignment systems for safety and practicality:

#### Assignment Statements
- **Purpose**: Main way to assign values
- **Syntax**: `variable = value;` (with semicolon)
- **Context**: Standalone statements

```go
// Assignment statements (most common)
var x = 5;
x = 10;           // Regular assignment
y += 5;           // Compound assignment
z *= 2;           // Compound assignment
```

#### Assignment Expressions (For Loops Only)
- **Purpose**: For loop clauses only
- **Syntax**: `variable = value` (without semicolon)
- **Context**: For loop initializers and increment clauses

```go
// Assignment expressions (for loops only)
for (var i = 0; i < 5; i = i + 1) { }  // Assignment in increment
for (j = 0; j < 5; j += 1) { }         // Assignment in initializer and increment
```

#### Why This Design?

**Prevents Common Bugs:**
```go
// This would be a bug in many languages:
if (x = 10) { }  // Parse error - assignment in conditional

// Bob forces you to write:
x = 10;
if (x == 10) { }  // Clear comparison
```

**Keeps Expressions Pure:**
```go
// Prevents side effects in expressions:
var result = (x = 10) + (y = 20);  // Parse error

// Forces clean code:
x = 10;
y = 20;
var result = x + y;  // Pure expression
```

**What's Blocked:**
```go
// These all cause parse errors:
var result = x = 10;                   // AssignExpr not allowed in expressions
var result = true ? (x = 10) : 0;      // AssignExpr not allowed in ternary
var result = func(x = 10);             // AssignExpr not allowed in function calls
var result = (x = 10) + 5;             // AssignExpr not allowed in arithmetic
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

### Logical Operators
- **And**: `&&`
- **Or**: `||`
- **Not**: `!`

### Conditional (Ternary) Operator
```go
// Basic ternary operator
var result = condition ? valueIfTrue : valueIfFalse;

// Examples
var max = x > y ? x : y;
var status = age >= 18 ? "adult" : "minor";
var message = x > 0 ? "positive" : "non-positive";

// Ternary with expressions
var result = x + y > 10 ? x * y : x + y;

// Ternary with function calls
var value = condition ? getValue() : getOtherValue();

// Nested ternary expressions
var result = x == 5 ? (y == 10 ? "both true" : "x true") : "x false";
```

**Precedence**: The ternary operator has lower precedence than logical operators (`&&`, `||`) but higher than assignment operators.

### String Operators

#### Concatenation
Bob supports bidirectional string + number concatenation with automatic type conversion:

```go
// String + Number
"Hello " + 42;           // → "Hello 42"
"Pi: " + 3.14;           // → "Pi: 3.14"

// Number + String
42 + " items";           // → "42 items"
3.14 + " is pi";         // → "3.14 is pi"
```

#### String Multiplication
```go
"hello" * 3;             // → "hellohellohello"
3 * "hello";             // → "hellohellohello"
```

**Note**: String multiplication requires whole numbers.

### Number Formatting
Bob automatically formats numbers to show only significant digits:

```go
"Count: " + 2.0;         // → "Count: 2" (no trailing zeros)
"Pi: " + 3.14;           // → "Pi: 3.14" (exact precision)
"Integer: " + 42;        // → "Integer: 42" (no decimal)
```

## Control Flow

### If Statements
```go
// Basic if statement
if (x > 10) {
    print("x is greater than 10");
}

// If-else statement
if (x > 10) {
    print("x is greater than 10");
} else {
    print("x is 10 or less");
}

// If-else if-else chain
if (x > 10) {
    print("x is greater than 10");
} else if (x > 5) {
    print("x is greater than 5");
} else {
    print("x is 5 or less");
}
```

### While Loops
```go
// Basic while loop
var i = 0;
while (i < 5) {
    print(i);
    i = i + 1;
}

// While loop with break (using return)
func countToThree() {
    var i = 0;
    while (true) {
        i = i + 1;
        if (i > 3) {
            return i;
        }
    }
}
```

### For Loops
```go
// Basic for loop
for (var i = 0; i < 5; i = i + 1) {
    print(i);
}

// For loop with no initializer
var j = 0;
for (; j < 3; j = j + 1) {
    // Empty body
}

// For loop with no condition (infinite loop with break)
for (var k = 0; ; k = k + 1) {
    if (k >= 2) {
        break; // or return
    }
}

// For loop with no increment
for (var n = 0; n < 3; ) {
    n = n + 1;
}
```

### Do-While Loops
```go
// Basic do-while loop
var i = 0;
do {
    print(i);
    i = i + 1;
} while (i < 5);

// Do-while with break
var j = 0;
do {
    j = j + 1;
    if (j == 3) {
        break;
    }
} while (j < 10);

// Do-while with continue
var k = 0;
var sum = 0;
do {
    k = k + 1;
    if (k == 2) {
        continue;
    }
    sum = sum + k;
} while (k < 5);
```

### Loop Features
- **Nested loops**: While, for, and do-while loops can be nested
- **Complex conditions**: Loops support any boolean expression
- **Function calls**: Loop conditions and bodies can contain function calls
- **Return statements**: Loops can be exited using return statements
- **Break and continue**: All loop types support break and continue statements
- **Variable scoping**: Variables declared in for loop initializers are scoped to the loop
- **Do-while behavior**: Do-while loops always execute the body at least once before checking the condition

## Functions

### Function Declaration
```go
func add(a, b) {
    return a + b;
}
```

### Function Call
```go
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

```go
var outerVar = "Outer";

func makeGreeter(greeting) {
    return greeting + " " + outerVar;
}

var greeter = makeGreeter("Hello");
// greeter captures outerVar from its lexical scope
```

### Nested Functions
```go
func outer() {
    func inner() {
        return 42;
    }
    return inner();
}
```

## Standard Library

### Print Function
```go
print("Hello, World!");
print(42);
print(true);
```

**Usage**: `print(expression)`
- Prints the result of any expression
- Automatically converts values to strings
- Adds newline after output

### Assert Function
```go
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
```go
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
```go
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
```go
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
```go
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

### Loop Examples
```go
// Count from 1 to 5
for (var i = 1; i <= 5; i = i + 1) {
    print("Count: " + i);
}

// Sum numbers from 1 to 10
var sum = 0;
for (var j = 1; j <= 10; j = j + 1) {
    sum = sum + j;
}
print("Sum: " + sum);

// Find first even number
var k = 1;
while (k % 2 != 0) {
    k = k + 1;
}
print("First even: " + k);
```

## Implementation Details

### Assignment System: Statements vs Expressions

Bob uses a dual assignment system to prevent common programming errors:

**Assignment Statements** (general use):
```go
var x = 10;
x = 20;  // This is a statement, not an expression
```

**Assignment Expressions** (for loops only):
```go
for (var i = 0; i < 10; i = i + 1) {  // Assignment in increment clause
    print(i);
}
```

**Why This Design?**
- **Prevents `if (x = 10)` bugs**: Assignment statements can't be used in conditions
- **Maintains loop functionality**: For loops still work with assignment expressions
- **Clear intent**: Distinguishes between intentional assignments and accidental ones

### Memory Management

Bob uses automatic memory management with conservative cleanup:

**Reference Counting**:
- All objects use `std::shared_ptr` for automatic cleanup
- Functions and thunks are tracked by reference count
- Objects are freed when no longer referenced

**Automatic Cleanup**:
- Functions created in loops are automatically cleaned up when unused
- Cleanup occurs every 1000 function/thunk creations
- Only removes objects with `use_count() == 1` (conservative approach)

**Safety Guarantees**:
- Functions referenced anywhere are never cleaned up
- Only truly unused functions are removed
- No dangling pointers or memory corruption

---

*This documentation covers the current Bob language implementation. For development plans, see ROADMAP.md.* 