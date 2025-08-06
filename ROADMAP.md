# Bob Language Development Roadmap

## Current Status

Bob is a working programming language with a solid foundation. Here's what's currently implemented:

### ✅ **Core Language Features**

#### **Data Types & Variables**
- **Numbers**: Integers, floats, automatic conversion
- **Strings**: Literals, concatenation, multiplication
- **Booleans**: `true`, `false`
- **None**: Null value representation
- **Variables**: Declaration, assignment, scoping
- **Assignment System**: Dual system (statements + expressions for loops)

#### **Operators**
- **Arithmetic**: `+`, `-`, `*`, `/`, `%`
- **Comparison**: `==`, `!=`, `>`, `<`, `>=`, `<=`
- **Logical**: `&&`, `||`, `!` (with short-circuit evaluation)
- **Bitwise**: `&`, `|`, `^`, `<<`, `>>`, `~`
- **Compound Assignment**: `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`, `<<=`, `>>=`
- **Ternary**: `condition ? valueIfTrue : valueIfFalse`

#### **Control Flow**
- **If Statements**: `if`, `else`, `else if` chains
- **While Loops**: Basic, nested, complex conditions
- **For Loops**: All clause variations, nested loops
- **Do-While Loops**: Basic, nested, break/continue support
- **Break/Continue**: Full support in all loop types

#### **Functions**
- **Function Declaration**: `func name(params) { body }`
- **Parameters**: Any number of parameters
- **Return Values**: Explicit and implicit returns
- **Closures**: Lexical scoping with variable capture
- **First-Class Functions**: Functions as values, parameters, return values
- **Anonymous Functions**: `func(params) { body }`
- **Nested Functions**: Functions defined inside other functions
- **Recursion**: Full support including deep recursion

#### **Standard Library**
- **`print()`**: Output with automatic type conversion
- **`assert()`**: Testing with custom error messages
- **`type()`**: Runtime type checking
- **`toString()`**: Universal string conversion
- **`toNumber()`**: String-to-number conversion
- **`input()`**: User input capability
- **`time()`**: Microsecond precision timing

#### **Advanced Features**
- **String Operations**: Bidirectional string + number concatenation
- **Number Formatting**: Smart significant digits
- **Memory Management**: Automatic cleanup
- **Error Handling**: Basic error reporting
- **Testing Framework**: Built-in assert function
- **Operator Precedence**: Full precedence hierarchy
- **Variable Shadowing**: Proper scoping rules

### **Current Limitations**

#### **Data Structures**
- **No Arrays/Lists**: No built-in collection types
- **No Maps/Dictionaries**: No key-value data structures
- **No Sets**: No unique value collections

#### **Advanced Language Features**
- **No Classes/Objects**: No object-oriented programming
- **No Modules/Imports**: No code organization system
- **No Exception Handling**: No try-catch blocks
- **No Type Annotations**: No static type checking
- **No Generics**: No parametric polymorphism

#### **Standard Library Gaps**
- **No File I/O**: No reading/writing files
- **No Network I/O**: No HTTP or socket operations
- **No Math Library**: No advanced mathematical functions
- **No Date/Time**: No date manipulation (except `time()`)
- **No Random Numbers**: No random number generation

#### **Development Tools**
- **No REPL**: No interactive mode
- **No Debugger**: No debugging tools
- **No Profiler**: No performance analysis
- **No Package Manager**: No dependency management

## **Future Development Phases**

### **Phase 1: Data Structures (High Priority)**

#### **Arrays/Lists**
```go
var numbers = [1, 2, 3, 4, 5];
print(numbers[0]);           // 1
numbers[1] = 42;            // Modify element
var length = len(numbers);  // Get length
```

**Implementation Plan:**
- Add array literal syntax `[expr, expr, ...]`
- Implement array indexing `array[index]`
- Add array assignment `array[index] = value`
- Create `len()` function for arrays
- Support nested arrays

#### **Maps/Dictionaries**
```go
var person = {"name": "Bob", "age": 25};
print(person["name"]);      // "Bob"
person["city"] = "NYC";     // Add/modify entry
var keys = keys(person);    // Get all keys
```

**Implementation Plan:**
- Add map literal syntax `{key: value, ...}`
- Implement map indexing `map[key]`
- Add map assignment `map[key] = value`
- Create `keys()`, `values()` functions
- Support nested maps

### **Phase 2: Standard Library Expansion (Medium Priority)**

#### **File I/O**
```go
var content = readFile("data.txt");
writeFile("output.txt", "Hello World");
var lines = readLines("config.txt");
```

#### **Math Library**
```go
var result = sqrt(16);      // 4.0
var random = rand(1, 100);  // Random number
var max = max(5, 10, 3);    // 10
```

#### **String Processing**
```go
var parts = split("a,b,c", ",");  // ["a", "b", "c"]
var joined = join(parts, "-");    // "a-b-c"
var upper = toUpper("hello");     // "HELLO"
```

### **Phase 3: Advanced Language Features (Lower Priority)**

#### **Exception Handling**
```go
try {
    var result = 10 / 0;
} catch (error) {
    print("Error: " + error);
}
```

#### **Modules/Imports**
```go
import "math.bob";
import "utils.bob" as utils;
```

#### **Classes & Objects**
```go
class Person {
    init(name, age) {
        this.name = name;
        this.age = age;
    }
    
    greet() {
        return "Hello, I'm " + this.name;
    }
}
```

### **Phase 4: Development Tools (Lower Priority)**

#### **Interactive Mode (REPL)**
```bash
$ bob
> var x = 5
> print(x + 3)
8
> func add(a, b) { return a + b; }
> add(2, 3)
5
```

#### **Debugger**
```go
debugger;  // Breakpoint
var x = 5;
// Step through code
```

## **Implementation Guidelines**

### **For Each New Feature:**
1. **Lexer**: Add new tokens if needed
2. **Parser**: Add new expression/statement types
3. **AST**: Define new node types
4. **Interpreter**: Implement evaluation logic
5. **Testing**: Create comprehensive test cases

### **Testing Strategy:**
```go
// Use the built-in assert function for testing
assert(add(2, 3) == 5, "add(2, 3) should equal 5");
assert(len([1, 2, 3]) == 3, "Array length should be 3");
```

### **Code Quality Standards:**
- **Comprehensive Testing**: Every feature needs test coverage
- **Error Handling**: Graceful error messages
- **Documentation**: Update language reference
- **Performance**: Consider memory and speed implications

## **Success Metrics**

### **Completed ✅**
- [x] Core language syntax and semantics
- [x] All basic operators and expressions
- [x] Control flow statements
- [x] Functions and closures
- [x] Standard library basics
- [x] Testing framework
- [x] Basic error handling
- [x] Memory management
- [x] Assignment system design

### **In Progress 🔄**
- [ ] Data structures (arrays, maps)
- [ ] Extended standard library
- [ ] Performance optimizations

### **Planned 📋**
- [ ] Advanced language features
- [ ] Development tools
- [ ] Documentation improvements

## **Resources**

- **[BOB_LANGUAGE_REFERENCE.md](BOB_LANGUAGE_REFERENCE.md)** - Complete language documentation
- **[ASSIGNMENT_DESIGN.md](ASSIGNMENT_DESIGN.md)** - Assignment system design rationale
- **[test_bob_language.bob](test_bob_language.bob)** - Comprehensive test suite
- **[Crafting Interpreters](https://craftinginterpreters.com/)** - Excellent resource for language implementation

## **Recent Major Achievements**

### **Assignment System Design**
- Implemented dual assignment system (statements + expressions)
- Prevents common bugs like `if (x = 10)`
- Maintains practical for loop syntax
- Documentation and testing

### **Control Flow Implementation**
- All loop types: while, for, do-while
- Break/continue support
- Nested scenarios
- Basic edge case handling

### **Function System**
- First-class functions
- Closures and lexical scoping
- Anonymous functions
- Function composition patterns
- Recursion support

---

*Last updated: August 2025* 