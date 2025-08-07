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
- **Escape sequences**: `\n` (newline), `\t` (tab), `\"` (quote), `\\` (backslash)
- **String indexing**: Access individual characters with `str[index]`
- **Immutable**: Strings cannot be modified after creation

### Booleans
- **True**: `true`
- **False**: `false`

### None
- **Null value**: `none` (represents absence of value)

### Arrays
- **Array literals**: `[1, 2, 3, 4, 5]`
- **Empty arrays**: `[]`
- **Mixed types**: `[1, "hello", true, 3.14]`
- **Nested arrays**: `[[1, 2], [3, 4]]`

#### String Indexing
```go
var str = "Hello, Bob!";

// Basic character access
print(str[0]);   // H
print(str[1]);   // e
print(str[7]);   // B
print(str[10]);  // !

// Get string length
var length = len(str);
print(length);   // 11

// Access last character
var last = str[len(str) - 1];
print(last);     // !

// String reversal
var reversed = "";
for (var i = len(str) - 1; i >= 0; i = i - 1) {
    reversed = reversed + str[i];
}
print(reversed); // !boB ,olleH

// Character counting
var text = "hello world";
var count = 0;
for (var i = 0; i < len(text); i = i + 1) {
    if (text[i] == "l") {
        count = count + 1;
    }
}
print(count);    // 3

// String search
var haystack = "Hello, Bob!";
var needle = "Bob";
var found = false;
for (var i = 0; i <= len(haystack) - len(needle); i = i + 1) {
    var match = true;
    for (var j = 0; j < len(needle); j = j + 1) {
        if (haystack[i + j] != needle[j]) {
            match = false;
            break;
        }
    }
    if (match) {
        found = true;
        break;
    }
}
print(found);    // true

// String parsing (CSV-like)
var csvLine = "Alice,30,Engineer";
var fields = [];
var currentField = "";
for (var i = 0; i < len(csvLine); i = i + 1) {
    var char = csvLine[i];
    if (char == ",") {
        push(fields, currentField);
        currentField = "";
    } else {
        currentField = currentField + char;
    }
}
push(fields, currentField);
print(fields[0]); // Alice
print(fields[1]); // 30
print(fields[2]); // Engineer
```

**String Indexing Rules:**
- **Zero-based indexing**: First character is at index 0
- **Bounds checking**: Index must be >= 0 and < string length
- **Number indices only**: Index must be a number (float indices auto-truncate)
- **Returns single character**: `str[index]` returns a single-character string
- **Immutable**: Cannot assign to string indices (strings are immutable)
- **Error handling**: Out-of-bounds access throws runtime error

### Dictionaries
- **Dictionary literals**: `{"key": "value", "number": 42}`
- **Empty dictionaries**: `{}`
- **String keys only**: Keys must be string literals
- **Any value types**: Values can be any type including nested structures
- **Nested dictionaries**: `{"user": {"name": "Bob", "age": 30}}`

#### Array Access
```go
var arr = [10, 20, 30, 40, 50];

// Basic indexing
print(arr[0]);  // 10
print(arr[2]);  // 30

// Float indices auto-truncate (like JavaScript/Lua)
print(arr[3.14]);  // 40 (truncated to arr[3])
print(arr[2.99]);  // 30 (truncated to arr[2])

// Assignment
arr[1] = 25;
print(arr[1]);  // 25

// Increment/decrement on array elements
arr[0]++;
print(arr[0]);  // 11
++arr[1];
print(arr[1]);  // 26
```

#### Array Built-in Functions
```go
var arr = [1, 2, 3];

// Get array length
print(len(arr));  // 3

// Add element to end
push(arr, 4);
print(arr);  // [1, 2, 3, 4]

// Remove and return last element
var last = pop(arr);
print(last);  // 4
print(arr);   // [1, 2, 3]
```

#### Dictionary Access
```go
var person = {"name": "Alice", "age": 30, "city": "New York"};

// Basic access
print(person["name"]);  // Alice
print(person["age"]);   // 30

// Missing keys return none
print(person["email"]); // none

// Assignment
person["email"] = "alice@example.com";
person["age"] = 31;

// Nested access
var gameState = {
    "player": {"health": 100, "level": 5},
    "world": {"name": "Bobland", "difficulty": "hard"}
};
print(gameState["player"]["health"]);  // 100
```

#### Dictionary Built-in Functions
```go
var person = {"name": "Bob", "age": 25, "city": "San Francisco"};

// Get all keys
var keys = keys(person);
print(keys);  // [name, age, city]

// Get all values
var values = values(person);
print(values);  // [Bob, 25, San Francisco]

// Check if key exists
print(has(person, "name"));   // true
print(has(person, "email"));  // false
```

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

### Compound Assignment Operators
- **Add and assign**: `+=`
- **Subtract and assign**: `-=`
- **Multiply and assign**: `*=`
- **Divide and assign**: `/=`
- **Modulo and assign**: `%=`

```go
var x = 10;
x += 5;   // x = 15
x -= 3;   // x = 12
x *= 2;   // x = 24
x /= 4;   // x = 6
x %= 4;   // x = 2
```

### Increment and Decrement Operators
- **Prefix increment**: `++x`
- **Postfix increment**: `x++`
- **Prefix decrement**: `--x`
- **Postfix decrement**: `x--`

```go
var x = 5;
print(++x);   // 6 (increments first, then prints)
print(x++);   // 6 (prints first, then increments)
print(x);     // 7

// Works with array elements
var arr = [1, 2, 3];
arr[0]++;
print(arr[0]);  // 2
++arr[1];
print(arr[1]);  // 3
```

### Comparison Operators
- **Equal**: `==`
- **Not equal**: `!=`
- **Greater than**: `>`
- **Less than**: `<`
- **Greater than or equal**: `>=`
- **Less than or equal**: `<=`

#### Cross-Type Comparisons
Bob supports cross-type comparisons with intuitive behavior:

```go
// Equality operators work with any types
print(none == "hello");    // false (different types)
print(42 == "42");         // false (different types)
print(true == true);       // true (same type and value)

// Comparison operators only work with numbers
print(5 > 3);              // true
print(5 > "3");            // Error: > not supported between number and string
print(none > 5);           // Error: > not supported between none and number
```

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

### Type Function
```go
type(value);
```

**Usage**:
- Returns the type of a value as a string
- Returns: `"number"`, `"string"`, `"boolean"`, `"none"`, `"array"`, `"function"`

**Examples**:
```go
print(type(42));        // "number"
print(type("hello"));   // "string"
print(type(true));      // "boolean"
print(type(none));      // "none"
print(type([1, 2, 3])); // "array"
print(type(func() {})); // "function"
```

### ToString Function
```go
toString(value);
```

**Usage**:
- Converts any value to a string
- Works with all data types

**Examples**:
```go
print(toString(42));        // "42"
print(toString(3.14));      // "3.14"
print(toString(true));      // "true"
print(toString([1, 2, 3])); // "[1, 2, 3]"
```

### ToNumber Function
```go
toNumber(value);
```

**Usage**:
- Converts a value to a number
- Returns 0 for non-numeric strings
- Returns 0 for boolean false, 1 for boolean true

**Examples**:
```go
print(toNumber("42"));      // 42
print(toNumber("3.14"));    // 3.14
print(toNumber("hello"));   // 0
print(toNumber(true));      // 1
print(toNumber(false));     // 0
```

### ToInt Function
```go
toInt(value);
```

**Usage**:
- Converts a number to an integer (truncates decimal part)
- Throws error for non-numeric values
- Same result as using bitwise OR with 0 (`value | 0`)

**Examples**:
```go
print(toInt(3.7));   // 3
print(toInt(3.2));   // 3
print(toInt(-3.7));  // -3
print(toInt(3.0));   // 3
```

### Input Function
```go
input();
input("prompt");
```

**Usage**:
- Reads a line from standard input
- Optional prompt string
- Returns the input as a string

**Examples**:
```go
var name = input("Enter your name: ");
print("Hello, " + name + "!");
```

### Time Function
```go
time();
```

**Usage**:
- Returns current Unix timestamp (seconds since epoch)
- Useful for timing and random seed generation

**Examples**:
```go
var start = time();
// ... do some work ...
var end = time();
print("Elapsed: " + (end - start) + " seconds");
```

### Sleep Function
```go
sleep(seconds);
```

**Usage**:
- Pauses execution for specified number of seconds
- Useful for animations and timing

**Examples**:
```go
print("Starting...");
sleep(1);
print("One second later...");
```

### PrintRaw Function
```go
printRaw("text");
```

**Usage**:
- Prints text without adding a newline
- Supports ANSI escape codes for colors and cursor control
- Useful for animations and formatted output

**Examples**:
```go
// Simple output
printRaw("Hello");
printRaw(" World");  // Prints: Hello World

// ANSI colors
printRaw("\e[31mRed text\e[0m");  // Red text
printRaw("\e[32mGreen text\e[0m"); // Green text

// Cursor positioning
printRaw("\e[2J");   // Clear screen
printRaw("\e[H");    // Move cursor to top-left
```

### Random Function
```go
random();
```

**Usage**:
- Returns a random number between 0.0 and 1.0
- Uses current time as seed

**Examples**:
```go
var randomValue = random();
print(randomValue);  // 0.0 to 1.0

// Generate random integer 1-10
var randomInt = toInt(random() * 10) + 1;
print(randomInt);
```

### Eval Function
```go
eval("code");
```

**Usage**:
- Executes Bob code as a string
- Returns the result of the last expression
- Runs in the current scope (can access variables)

**Examples**:
```go
var x = 10;
var result = eval("x + 5");
print(result);  // 15

var code = "2 + 3 * 4";
var result = eval(code);
print(result);  // 14
```

### Array Functions

#### Len Function
```go
len(array);
```

**Usage**:
- Returns the length of an array
- Also works with strings

**Examples**:
```go
var arr = [1, 2, 3, 4, 5];
print(len(arr));  // 5

var str = "hello";
print(len(str));  // 5
```

#### Push Function
```go
push(array, value);
```

**Usage**:
- Adds a value to the end of an array
- Modifies the array in place
- Returns none

**Examples**:
```go
var arr = [1, 2, 3];
push(arr, 4);
print(arr);  // [1, 2, 3, 4]
```

#### Pop Function
```go
pop(array);
```

**Usage**:
- Removes and returns the last element of an array
- Throws error if array is empty
- Modifies the array in place

**Examples**:
```go
var arr = [1, 2, 3, 4];
var last = pop(arr);
print(last);  // 4
print(arr);   // [1, 2, 3]
```

### Dictionary Functions

#### Keys Function
```go
keys(dictionary);
```

**Usage**:
- Returns an array of all keys in a dictionary
- Keys are returned as strings

**Examples**:
```go
var person = {"name": "Bob", "age": 25, "city": "SF"};
var keys = keys(person);
print(keys);  // [name, age, city]
```

#### Values Function
```go
values(dictionary);
```

**Usage**:
- Returns an array of all values in a dictionary
- Values maintain their original types

**Examples**:
```go
var person = {"name": "Bob", "age": 25, "city": "SF"};
var values = values(person);
print(values);  // [Bob, 25, SF]
```

#### Has Function
```go
has(dictionary, key);
```

**Usage**:
- Returns true if the key exists in the dictionary
- Returns false if the key is missing
- Key must be a string

**Examples**:
```go
var person = {"name": "Bob", "age": 25};
print(has(person, "name"));   // true
print(has(person, "email"));  // false
```

## Error Handling

### Current Error Types
- **Division by zero**: `DivisionByZeroError`
- **Type errors**: `Operands must be of same type`
- **String multiplication**: `String multiplier must be whole number`
- **Assertion failures**: `Assertion failed: condition is false`
- **Function call errors**: `Can only call functions, got [type]`
- **Argument count errors**: `Expected X arguments but got Y`

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

// Function call errors
var notAFunction = 42;
notAFunction();  // Error: Can only call functions, got number

// Argument count errors
func test(a, b) { return a + b; }
test(1);  // Error: Expected 2 arguments but got 1
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