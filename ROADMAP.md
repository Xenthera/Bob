# Bob Language Development Roadmap

## Current Status
- Basic expressions (arithmetic, comparison, logical)
- Variables and assignment
- Print statements (converted to standard library function)
- Block statements
- Environment/scoping
- Function implementation (COMPLETED)
- Return statements (COMPLETED)
- Closures (COMPLETED)
- Assert function (COMPLETED)
- Standard library infrastructure (COMPLETED)
- First-class functions and higher-order functions (COMPLETED)
- String + number concatenation with smart formatting (COMPLETED)
- String multiplication (COMPLETED)
- Alphanumeric identifiers (COMPLETED)
- Comprehensive testing framework (COMPLETED)

## Phase 1: Core Language Features (High Priority)

### 1. Control Flow
```bob
// If statements
if (x > 10) {
    print "big";
} else {
    print "small";
}

// While loops
var i = 0;
while (i < 5) {
    print i;
    i = i + 1;
}
```

**Implementation:**
- Add `IfStmt` and `WhileStmt` to Statement.h
- Update parser to handle `if` and `while` keywords
- Implement control flow in interpreter

### 2. Logical Operators
```bob
// Currently missing: and, or, not operators
if (x > 0 and y < 10) {
    print "valid range";
}
```

**Implementation:**
- Add `and`, `or`, `not` operator parsing
- Implement logical operator evaluation in interpreter

### 3. Better Error Handling
```bob
// Current: Basic error messages
// Goal: Line numbers, better context
Error at line 5: Expected ';' after expression
    print 42
         ^
```

## Phase 2: Data Structures (Medium Priority)

### 4. Arrays/Lists
```bob
var numbers = [1, 2, 3, 4];
print numbers[0];  // 1
numbers[1] = 42;
```

### 5. Maps/Dictionaries
```bob
var person = {"name": "Bob", "age": 25};
print person["name"];
person["city"] = "NYC";
```

## Phase 3: Standard Library (Medium Priority)

### 6. Additional Built-in Functions
```bob
len("hello");           // String length
input("Enter name: ");  // User input
random(1, 100);        // Random numbers
type(42);              // Type checking
```

### 7. File I/O
```bob
var content = readFile("data.txt");
writeFile("output.txt", "Hello World");
```

## Phase 4: Advanced Features (Lower Priority)

### 8. Classes & Objects
```bob
class Person {
    init(name, age) {
        this.name = name;
        this.age = age;
    }
    
    greet() {
        print "Hello, I'm " + this.name;
    }
}

var bob = Person("Bob", 25);
bob.greet();
```

### 9. Modules/Imports
```bob
import "math.bob";
import "utils.bob";
```

### 10. Type System
```bob
// Optional type annotations
fun add(a: number, b: number): number {
    return a + b;
}
```

## Implementation Tips

### For Each Feature:
1. Lexer: Add new tokens if needed
2. Parser: Add new expression/statement types
3. AST: Define new node types
4. Interpreter: Implement evaluation logic
5. Test: Create test cases using assert function

### Testing Strategy:
```bob
// Use the new assert function for comprehensive testing
assert(add(2, 3) == 5, "add(2, 3) should equal 5");
assert(x > 0, "x should be positive");
```

## Recommended Next Steps

1. Add if statements (fundamental control flow)
2. Add while loops (enables iteration)
3. Implement logical operators (and, or, not)
4. Improve error messages (better developer experience)
5. Add arrays (most useful data structure)

## Success Metrics

- [x] Can write simple functions
- [x] Can use return statements
- [x] Can use closures
- [x] Has assert function for testing
- [x] Has standard library infrastructure
- [x] Supports first-class functions
- [x] Has comprehensive testing framework
- [ ] Can use if/else statements
- [ ] Can use while loops
- [ ] Can use logical operators
- [ ] Can work with arrays
- [ ] Can read/write files
- [ ] Has good error messages

## Resources

- [Crafting Interpreters](https://craftinginterpreters.com/) - Excellent resource for language implementation
- [Bob's current source code](./source/) - Your implementation
- [Test files](./*.bob) - Examples of current functionality

## Recent Achievements

### Function Implementation (COMPLETED)
- Function declarations with parameters
- Function calls with arguments
- Return statements
- Proper scoping and closures
- Nested function calls

### Standard Library (COMPLETED)
- `print()` function (converted from statement)
- `assert()` function with custom messages
- Extensible architecture for adding more functions

### Testing Framework (COMPLETED)
- Comprehensive test suite using assert
- Tests for all language features
- Proper error handling and execution stopping

### Advanced Language Features (COMPLETED)
- First-class functions and higher-order functions
- Function passing as arguments
- Function composition patterns
- Callback patterns and function storage
- String + number concatenation with smart formatting
- String multiplication (string * number, number * string)
- Alphanumeric identifiers support
- Stress testing with 100-parameter functions 