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
```

### Dictionaries
```go
var person = {"name": "Alice", "age": 30};

// Access and modify
print(person["name"]);  // Alice
person["city"] = "NYC";
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

### Arrays & Strings
```go
len([1, 2, 3]);     // 3
len("hello");       // 5
push(array, value); // Add to end
pop(array);         // Remove from end
```

### Dictionaries
```go
keys(dict);         // Array of keys
values(dict);       // Array of values
has(dict, "key");   // Check if key exists
```

### Utility
```go
assert(condition, "message");  // Testing
time();                        // Current time in microseconds
sleep(1.5);                   // Sleep for 1.5 seconds
random();                     // Random number 0-1
eval("1 + 2");               // Evaluate string as code
exit(0);                     // Exit program
```

### File I/O
```go
var content = readFile("data.txt");
writeFile("output.txt", "Hello");
var lines = readLines("config.txt");
var exists = fileExists("test.txt");
```

## Advanced Features

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

for (var i = 0; i < len(people); i = i + 1) {
    var person = people[i];
    print(person["name"] + " is " + person["age"] + " years old");
}
```

### File Processing
```go
var lines = readLines("data.txt");
var processed = [];

for (var i = 0; i < len(lines); i = i + 1) {
    var line = lines[i];
    if (len(line) > 0) {
        push(processed, "Processed: " + line);
    }
}

writeFile("output.txt", join(processed, "\n"));
```

---

*For more examples, see the comprehensive test suite in `test_bob_language.bob`*