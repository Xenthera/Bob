# Bob Language Reference

## Quick Start

```bash
# Build Bob
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
ninja -C build

# Run a file
./build/bin/bob script.bob

# Interactive mode
./build/bin/bob
```

## Data Types

### Numbers
Bob supports three numeric types with automatic type promotion:

```go
var integer = 42;                    // Regular integer (long long)
var float = 3.14;                    // Floating-point number (double)
var negative = -10;                  // Negative numbers
var bigint = 9999999999999999999;    // BigInt (arbitrary-precision)
```

**Type System:**
- **`integer`**: 64-bit signed integers (long long range)
- **`number`**: 64-bit floating-point numbers (double)
- **`bigint`**: Arbitrary-precision integers (no size limit)

**Automatic Type Promotion:**
- Integers automatically promote to BigInt when they exceed long long range
- Arithmetic operations between different types work seamlessly
- Use `type()` to check the runtime type of a value

**Examples:**
```go
var small = 42;                      // type: "integer"
var large = 9999999999999999999;     // type: "bigint"
var result = small + large;          // type: "bigint"

print(type(42));                     // "integer"
print(type(3.14));                   // "number"
print(type(9999999999999999999));    // "bigint"
```

### Strings
```go
var text = "Hello, World!";
var empty = "";
var escaped = "Line 1\nLine 2\t\"quoted\"";
var concat = "Hello" + " " + "World";
var repeat = "hi" * 3;  // "hihihi"
```

### Booleans
```go
var yes = true;
var no = false;
```

### None
```go
var nothing = none;
```

### Arrays
```go
var numbers = [1, 2, 3];
var mixed = [42, "hello", true];
var nested = [[1, 2], [3, 4]];

// Access and modify
print(numbers[0]);  // 1
numbers[1] = 99;

// Array properties (read-only)
print(numbers.length);  // 3
print(numbers.first);   // 1
print(numbers.last);    // 3
print(numbers.empty);   // false

var empty = [];
print(empty.length);    // 0
print(empty.first);     // none
print(empty.empty);     // true

// Array methods
var fruits = ["apple", "banana", "cherry"];
print(fruits.contains("banana"));  // true
print(fruits.contains("grape"));   // false

var numbers = [1, 2, 3, 4, 5];
var slice = numbers.slice(1, 4);   // [2, 3, 4]
var reversed = numbers.slice(0);   // Copy
reversed.reverse();                // [5, 4, 3, 2, 1]

// Array concatenation
var arr1 = [1, 2];
var arr2 = [3, 4];
var combined = arr1 + arr2;        // [1, 2, 3, 4]
```

### Dictionaries
```go
var person = {"name": "Alice", "age": 30};

// Access and modify (bracket notation)
print(person["name"]);  // Alice
person["city"] = "NYC";

// Access and modify (dot notation - cleaner syntax)
print(person.name);     // Alice
person.city = "NYC";
person.age = 31;

// Both notations are equivalent
assert(person.name == person["name"]);

// Dictionary properties (built-in)
print(person.length);   // 3 (number of key-value pairs)
print(person.empty);    // false
print(person.keys);     // ["name", "age", "city"]
print(person.values);   // ["Alice", 31, "NYC"]
```

## Variables

```go
var x = 10;       // Declaration
x = 20;           // Reassignment
y += 5;           // Compound assignment
z++;              // Increment (for array elements)
```

## Operators

### Arithmetic
```go
+ - * / %         // Basic math
-x                // Unary minus
```

### Comparison
```go
== != < > <= >=   // Comparisons
```

### Logical
```go
&& || !           // AND, OR, NOT (short-circuit)
```

### Bitwise
Bob supports full bitwise operations on all numeric types, including BigInt:

```go
& | ^ << >> ~     // Bitwise operations
```

**Supported Types:**
- **`integer`**: Full bitwise support
- **`number`**: Converted to integer for bitwise operations
- **`bigint`**: Full arbitrary-precision bitwise support (Python-like)

**BigInt Bitwise Operations:**
Bob provides Python-like BigInt bitwise operations with no size limitations:

```go
var big_num = 9999999999999999999;
print(big_num & 10);     // 10 (type: bigint)
print(big_num | 10);     // 9999999999999999999 (type: bigint)
print(big_num ^ 10);     // 9999999999999999989 (type: bigint)
print(big_num << 2);     // 39999999999999999996 (type: bigint)
print(big_num >> 2);     // 2499999999999999999 (type: bigint)
```

**Mixed Type Operations:**
Bitwise operations work seamlessly between different numeric types:

```go
var small = 42;
var large = 9999999999999999999;

print(small & large);    // 42 (type: bigint)
print(large & small);    // 42 (type: bigint)
print(small | large);    // 9999999999999999999 (type: bigint)
```

**Examples:**
```go
// Basic bitwise operations
var a = 42;
var b = 10;
print(a & b);    // 10 (AND)
print(a | b);    // 42 (OR)
print(a ^ b);    // 32 (XOR)
print(a << 2);   // 168 (left shift)
print(a >> 1);   // 21 (right shift)

// BigInt operations
var huge = 9999999999999999999;
print(huge & 255);       // 255 (works with any size)
print(huge << 10);       // 10239999999999999998976
print(huge >> 10);       // 9765624999999999

// Mixed operations
print(42 & huge);        // 42 (automatic type conversion)
print(huge & 42);        // 42 (same result)
```

### Compound Assignment
```go
+= -= *= /= %=    // Arithmetic compound
&= |= ^= <<= >>=  // Bitwise compound
```

### Ternary
```go
var result = condition ? "yes" : "no";
```

## Control Flow

### If Statements
```go
if (x > 0) {
    print("positive");
} else if (x < 0) {
    print("negative");
} else {
    print("zero");
}
```

### While Loops
```go
while (i < 10) {
    print(i);
    i = i + 1;
}
```

### Do-While Loops
```go
do {
    print(i);
    i = i + 1;
} while (i < 10);
```

### For Loops
```go
for (var i = 0; i < 10; i = i + 1) {
    print(i);
}

// Break and continue work in all loops
for (var i = 0; i < 10; i = i + 1) {
    if (i == 5) break;
    if (i % 2 == 0) continue;
    print(i);
}
```

### Foreach Loops
```go
// Iterate over arrays
var numbers = [1, 2, 3, 4, 5];
foreach(var num : numbers) {
    print(num);
}

// Iterate over dictionaries (provides [key, value] pairs)
var person = {"name": "Alice", "age": 30};
foreach(var pair : person) {
    print(pair[0], ":", pair[1]);  // key : value
}

// Iterate over strings (character by character)
var text = "Hello";
foreach(var char : text) {
    print(char);  // H, e, l, l, o
}

// Control flow works in foreach loops
foreach(var num : [1, 2, 3, 4, 5]) {
    if (num == 3) break;
    if (num % 2 == 0) continue;
    print(num);  // prints 1
}
```

## Functions

### Basic Functions
```go
func greet(name) {
    return "Hello, " + name;
}

var message = greet("Alice");
```

### Anonymous Functions
```go
var square = func(x) { return x * x; };
var result = square(5);
```

### Closures
```go
func makeCounter() {
    var count = 0;
    return func() {
        count = count + 1;
        return count;
    };
}

var counter = makeCounter();
print(counter());  // 1
print(counter());  // 2
```

### First-Class Functions
```go
func apply(fn, x) {
    return fn(x);
}

var result = apply(square, 10);  // 100
```

## Built-in Functions

### I/O
```go
print("Hello");           // Output with newline
printRaw("No newline");   // Output without newline
var input = input("Enter something: ");
```

### Type Conversion
```go
toString(42);        // "42"
toNumber("3.14");    // 3.14
toInt(3.9);         // 3
toBoolean(1);       // true
type(42);           // "number" (user-friendly)
type(3.14);         // "number"
type(9999999999999999999); // "number"
typeRaw(42);        // "integer" (debug - actual internal type)
typeRaw(3.14);      // "number"
typeRaw(9999999999999999999); // "bigint"
```

**Type System:**
- `type()` returns user-friendly type: `"number"` for all numeric types, `"string"`, `"boolean"`, `"array"`, `"dict"`, `"function"`, `"none"`
- `typeRaw()` returns actual internal type: `"integer"`, `"number"`, `"bigint"`, `"string"`, `"boolean"`, `"array"`, `"dict"`, `"function"`, `"none"`
- `toNumber()` converts strings to numbers (including BigInt for large values)
- `toInt()` truncates numbers to integers (handles BigInt overflow gracefully)
- Automatic type promotion occurs for arithmetic operations
- **Overflow Detection**: All arithmetic operations automatically promote to BigInt when they would overflow

### Arrays, Strings, and Dictionaries: Method style (preferred)
```go
[1, 2, 3].len();          // 3
"hello".len();            // 5
var a = [1, 2]; a.push(3); // a is now [1, 2, 3]
var v = a.pop();           // v == 3

var d = {"a": 1, "b": 2};
d.len();                   // 2
d.keys();                  // ["a", "b"]
d.values();                // [1, 2]
d.has("a");               // true
```

Note: Global forms like `len(x)`, `push(arr, ...)`, `pop(arr)`, `keys(dict)`, `values(dict)`, `has(dict, key)` have been removed. Use method style.

### Numbers
```go
toInt(3.9);        // 3 (global)
(3.9).toInt();     // 3 (method on number)
```

### Integer Overflow Detection
Bob automatically detects integer overflow and promotes to BigInt to prevent precision loss:

```go
var max_int = 9223372036854775807;  // LLONG_MAX
max_int++;                          // 9223372036854775808 (BigInt)
max_int + 1;                        // 9223372036854775808 (BigInt)
max_int * 2;                        // 18446744073709551614 (BigInt)

// All arithmetic operators support overflow detection:
// +, -, *, ++, --, +=, -=, *=
```

### Utility
```go
assert(condition, "message");  // Testing
time();                        // Current time in microseconds
sleep(1.5);                   // Sleep for 1.5 seconds
rand.random();                // Random number 0-1
eval("print('Hello');");     // Execute string as code
exit(0);                     // Exit program

// Range function (Python-style)
range(5);                     // [0, 1, 2, 3, 4]
range(1, 5);                  // [1, 2, 3, 4]
range(0, 10, 2);              // [0, 2, 4, 6, 8]

// Introspection functions
dir(math);                    // Lists all members of math module
functions(math);              // Lists function members only
values(math);                 // Lists non-function members (constants)
```

### File I/O
```go
var content = readFile("data.txt");
writeFile("output.txt", "Hello");
var lines = readLines("config.txt");
var exists = fileExists("test.txt");
```

## Standard Library Reference

The following built-ins are available by default. Unless specified, functions throw on invalid argument counts/types.

- print(x): prints x with newline
- printRaw(x): prints x without newline
- input(prompt?): reads a line from stdin (optional prompt)
- toString(x): returns string representation
- toNumber(s): parses string to number (including BigInt for large values) or returns none
- toInt(n): truncates number to integer
- toBoolean(x): converts to boolean using truthiness rules
- type(x): returns the user-friendly type name as string
- typeRaw(x): returns the actual internal type name as string (for debugging)
- len(x) / x.len(): length of array/string/dict
- push(arr, ...values) / arr.push(...values): appends values to array in place, returns arr
- pop(arr) / arr.pop(): removes and returns last element
- keys(dict) / dict.keys(): returns array of keys
- values(dict) / dict.values(): returns array of values
- has(dict, key) / dict.has(key): returns true if key exists
- readFile(path): returns entire file contents as string
- writeFile(path, content): writes content to file
- readLines(path): returns array of lines
- fileExists(path): boolean
- time(): microseconds since Unix epoch
- sleep(seconds): pauses execution
- rand.random(): float in [0,1)
- eval(code): executes code string in current environment
- exit(code?): terminates the program
- range(start?, end?, step?): creates sequence of numbers (Python-style)
- dir(obj): lists all members of object/module
- functions(obj): lists function members only
- values(obj): lists non-function members (constants/values)

Notes:
- Arrays support properties: length, first, last, empty
- Dicts support properties: length, empty, keys, values
- Method-style builtins on arrays/strings/dicts are preferred; global forms remain for compatibility.

## Advanced Features

### Classes (Phase 1)
```go
// Declare a class with fields and methods
class Person {
  var name;
  var age;

  // Methods can use implicit `this`
  func setName(n) { this.name = n; }
  func greet() { print("Hi, I'm " + this.name); }
}

// Construct via the class name
var p = Person();
p.setName("Bob");
p.greet();

// Fields are stored on the instance (a dictionary under the hood)
p.age = 30;
```

Notes:
- Instances are plain dictionaries; methods are shared functions placed on the instance.
- On a property call like `obj.method(...)`, the interpreter injects `this = obj` into the call frame (no argument injection).
- Taking a method reference and calling it later does not auto‑bind `this`; call via `obj.method(...)` when needed.

### Extensions (Built‑ins and Classes)
Extend existing types (including built‑ins) with new methods:

```go
extension array {
  func sum() {
    var i = 0; var s = 0;
    while (i < len(this)) { s = s + this[i]; i = i + 1; }
    return s;
  }
}

extension dict { func size() { return len(this); } }
extension string { func shout() { return toString(this) + "!"; } }
extension any { func tag() { return "<" + type(this) + ">"; } }

assert([1,2,3].sum() == 6);
assert({"a":1,"b":2}.size() == 2);
assert("hi".shout() == "hi!");
assert(42.tag() == "<number>");
```

Notes:
- Lookup order for `obj.method(...)`: instance dictionary → class extensions (for user classes) → built‑in extensions (string/array/dict) → `any`.
- `this` is injected for property calls.

### String Interpolation
```go
var name = "Alice";
var age = 30;
var message = "Name: " + name + ", Age: " + age;
```

### Tail Call Optimization
```go
func factorial(n, acc) {
    if (n <= 1) return acc;
    return factorial(n - 1, n * acc);  // Tail call optimized
}
```

### Assignment System
Bob has a unique assignment system that prevents common bugs:

```go
// Assignment statements (everywhere)
var x = 5;
x = 10;
y += 5;

// Assignment expressions (only in for loops)
for (var i = 0; i < 5; i = i + 1) { }  // OK
for (j = 0; j < 5; j += 1) { }         // OK

// This prevents bugs like:
if (x = 10) { }  // PARSE ERROR - prevents accidental assignment
```

## Memory Management

Bob automatically manages memory - no manual allocation or deallocation needed. Objects are cleaned up when no longer referenced.

## Error Handling

Bob provides helpful error messages with context:

```go
// Runtime errors show line numbers and context
var x = undefined_variable;  // Error: Undefined variable 'undefined_variable' at line 2

// Type errors are caught
var result = "hello" / 5;    // Error: Cannot divide string by number
```

## Built-in Modules

### Math Module
```go
import math;

// Mathematical constants
print(math.PI);      // 3.141592653589793
print(math.E);       // 2.718281828459045
print(math.MAX_INT); // 9223372036854775807

// Mathematical functions
print(math.abs(-5));     // 5
print(math.floor(3.7));  // 3
print(math.ceil(3.2));   // 4
print(math.round(3.5));  // 4
print(math.max(1, 2, 3)); // 3
print(math.min(1, 2, 3)); // 1
```

### Random Module
```go
import random;

// Set seed for reproducible results
random.seed(42);

// Generate random numbers
print(random.random());           // Random float between 0 and 1
print(random.randint(1, 10));     // Random integer between 1 and 10
print(random.choice([1, 2, 3]));  // Random choice from array
```

### Other Built-in Modules
```go
import io;      // File I/O operations
import os;      // Operating system interface
import time;    // Time-related functions
import path;    // Path manipulation
import json;    // JSON parsing and generation
import base64;  // Base64 encoding/decoding
import sys;     // System information
```

## Interactive Mode (REPL)

Bob includes an interactive mode with enhanced features:

```bash
$ ./build/bin/bob
Bob v0.0.3, 2025
-> var x = 42;
-> print(x * 2);
84
-> history;  # Show command history
-> clear;    # Clear history
-> exit();
```

**REPL Features:**
- **Arrow key navigation** (up/down for history)
- **Line editing** (left/right arrows, backspace, insert)
- **Command history** (persistent across sessions)
- **Special commands:** `history`, `clear`
- **Cross-platform** (works on Windows, macOS, Linux)
```

## Examples

### Fibonacci with Tail Call Optimization
```go
func fib(n, a, b) {
    if (n == 0) return a;
    if (n == 1) return b;
    return fib(n - 1, b, a + b);
}

print(fib(40, 0, 1));  // Fast even for large numbers
```

### Working with Data Structures
```go
var people = [
    {"name": "Alice", "age": 30},
    {"name": "Bob", "age": 25}
];

// Using foreach (cleaner syntax)
foreach(var person : people) {
    print(person["name"] + " is " + person["age"] + " years old");
}

// Using range for iteration
foreach(var i : range(people.len())) {
    var person = people[i];
    print(person["name"] + " is " + person["age"] + " years old");
}
```

### File Processing
```go
var lines = readLines("data.txt");
var processed = [];

for (var i = 0; i < lines.len(); i = i + 1) {
    var line = lines[i];
    if (line.len() > 0) {
        processed.push("Processed: " + line);
    }
}

var output = "";
for (var i = 0; i < processed.len(); i = i + 1) {
    output = output + processed[i];
    if (i < processed.len() - 1) {
        output = output + "\n";
    }
}
writeFile("output.txt", output);
```

---

*For more examples, see the comprehensive test suite in `test_bob_language.bob`*