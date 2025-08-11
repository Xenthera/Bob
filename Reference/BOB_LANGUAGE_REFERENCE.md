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
```go
var integer = 42;
var float = 3.14;
var negative = -10;
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
```go
& | ^ << >> ~     // Bitwise operations
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
type(42);           // "number"
```

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

### Utility
```go
assert(condition, "message");  // Testing
time();                        // Current time in microseconds
sleep(1.5);                   // Sleep for 1.5 seconds
random();                     // Random number 0-1
eval("print('Hello');");     // Execute string as code
exit(0);                     // Exit program
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
- toNumber(s): parses string to number or returns none
- toInt(n): truncates number to integer
- toBoolean(x): converts to boolean using truthiness rules
- type(x): returns the type name as string
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
- random(): float in [0,1)
- eval(code): executes code string in current environment
- exit(code?): terminates the program

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

## Interactive Mode (REPL)

Bob includes an interactive mode for experimenting:

```bash
$ ./build/bin/bob
Bob Interactive Mode
> var x = 42;
> print(x * 2);
84
> exit();
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

for (var i = 0; i < people.len(); i = i + 1) {
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