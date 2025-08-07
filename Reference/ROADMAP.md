# Bob Language Development Roadmap

## Current Status

Bob is a mature, working programming language with a modern architecture and comprehensive feature set.

### ✅ **Core Language Features (Complete)**

#### **Data Types & Variables**
- **Numbers**: Integers, floats, automatic conversion
- **Strings**: Literals, concatenation, multiplication, escape sequences
- **Booleans**: `true`, `false`
- **None**: Null value representation
- **Arrays**: Dynamic arrays with indexing, assignment, and built-in functions
- **Dictionaries**: Hash maps with string keys and mixed-type values
- **Functions**: First-class functions as values
- **Variables**: Declaration, assignment, scoping
- **Assignment System**: Dual system (statements + expressions for loops only)

#### **Operators (Complete)**
- **Arithmetic**: `+`, `-`, `*`, `/`, `%`, unary `-`
- **Comparison**: `==`, `!=`, `>`, `<`, `>=`, `<=`
- **Logical**: `&&`, `||`, `!` (with short-circuit evaluation)
- **Bitwise**: `&`, `|`, `^`, `<<`, `>>`, `~`
- **Compound Assignment**: `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`, `<<=`, `>>=`
- **Ternary**: `condition ? valueIfTrue : valueIfFalse`
- **Increment/Decrement**: `++`, `--` (for array elements)

#### **Control Flow (Complete)**
- **If Statements**: `if`, `else`, `else if` chains
- **While Loops**: Basic, nested, complex conditions
- **Do-While Loops**: Basic, nested, break/continue support
- **For Loops**: All clause variations, nested loops
- **Break/Continue**: Full support in all loop types

#### **Functions (Complete)**
- **Function Declaration**: `func name(params) { body }`
- **Parameters**: Any number of parameters (tested up to 100)
- **Return Values**: Explicit and implicit returns
- **Closures**: Lexical scoping with variable capture
- **First-Class Functions**: Functions as values, parameters, return values
- **Anonymous Functions**: `func(params) { body }`
- **Nested Functions**: Functions defined inside other functions
- **Recursion**: Full support including deep recursion
- **Tail Call Optimization**: Trampoline-based optimization preventing stack overflow

#### **Data Structures (Complete)**
- **Arrays**: Full support with indexing, assignment, nested arrays
- **Dictionaries**: Full support with key-value pairs, nested dictionaries
- **Array Operations**: `len()`, `push()`, `pop()`, indexing, assignment
- **Dictionary Operations**: `keys()`, `values()`, `has()`, indexing, assignment
- **Mixed Types**: Arrays and dictionaries can hold any value types

#### **Standard Library (Complete)**
- **I/O Functions**: `print()`, `printRaw()`, `input()`
- **Type System**: `type()`, `toString()`, `toNumber()`, `toInt()`, `toBoolean()`
- **Testing**: `assert()` with custom error messages
- **Timing**: `time()` (microsecond precision), `sleep()`
- **Utility**: `random()` (properly seeded), `eval()`, `exit()`
- **Data Structure**: `len()`, `push()`, `pop()`, `keys()`, `values()`, `has()`
- **File I/O**: `readFile()`, `writeFile()`, `readLines()`, `fileExists()`

#### **Advanced Features (Complete)**
- **String Operations**: Bidirectional string + number concatenation
- **Number Formatting**: Smart significant digits handling
- **Memory Management**: Automatic cleanup with reference counting
- **Error Handling**: Comprehensive error reporting with context
- **Testing Framework**: Built-in assert function with 70+ comprehensive tests
- **Operator Precedence**: Full precedence hierarchy implementation
- **Variable Shadowing**: Proper lexical scoping rules
- **Interactive Mode**: Full REPL with error handling
- **Cross-Type Comparisons**: Smart equality for all types
- **Copy Semantics**: Value vs reference copying for different types

### ✅ **Architecture & Infrastructure (Complete)**

#### **Modern Build System**
- **CMake**: Cross-platform build configuration
- **Ninja**: High-speed build system (3.1x faster than Make)
- **CTest**: Integrated testing framework
- **Cross-Platform**: Windows, macOS, Linux support
- **Performance**: Optimized build times and incremental compilation

#### **Clean Architecture**
- **Modular Design**: Separated parsing, runtime, stdlib, and CLI
- **Tier Separation**: Clear boundaries between language components
- **Header Organization**: Organized by functional area
- **Source Structure**: `src/headers/` and `src/sources/` organization

#### **Refactored Interpreter**
- **Evaluator**: Expression evaluation (visitor pattern)
- **Executor**: Statement execution and control flow
- **RuntimeDiagnostics**: Utility functions and type checking
- **Memory Management**: Smart pointer usage throughout
- **Error System**: Centralized error reporting

### **Current Architecture Status**

```
Bob Language
├── Parsing Layer
│   ├── Lexer (tokenization)
│   ├── Parser (AST generation)
│   └── ErrorReporter (syntax errors)
├── Runtime Layer
│   ├── Evaluator (expression visitor)
│   ├── Executor (statement visitor)
│   ├── Interpreter (orchestration)
│   ├── Environment (variable scoping)
│   ├── Value (type system)
│   └── RuntimeDiagnostics (utilities)
├── Standard Library
│   └── BobStdLib (built-in functions)
└── CLI Interface
    └── Bob (command-line interface)
```

## **Future Development Phases**

### **Phase 1: Advanced Language Features (Medium Priority)**

#### **Exception Handling System**
```bob
try {
    var result = 10 / 0;
} catch (error) {
    print("Error: " + error.message);
} finally {
    print("Cleanup");
}
```

**Implementation Plan:**
- Add `try`/`catch`/`finally` syntax to parser
- Implement exception objects with stack traces
- Add `throw` statement for custom exceptions
- Integrate with existing error system

#### **Pattern Matching**
```bob
match value {
    case 0: "zero"
    case 1 | 2: "small"
    case x if x > 10: "large"
    default: "other"
}
```

**Implementation Plan:**
- Add `match`/`case` syntax
- Implement pattern matching logic
- Support guards with `if` conditions
- Add destructuring for arrays/dictionaries

### **Phase 2: Object System (Lower Priority)**

#### **Simple Objects**
```bob
var person = {
    name: "Alice",
    age: 30,
    greet: func() {
        return "Hello, I'm " + this.name;
    }
};
```

**Implementation Plan:**
- Add object literal syntax
- Implement `this` binding
- Support method calls
- Add property access/assignment

#### **Classes (Optional)**
```bob
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

**Implementation Plan:**
- Add `class` keyword and syntax
- Implement constructors with `init()`
- Support inheritance with `extends`
- Add method definitions

### **Phase 3: Module System (Lower Priority)**

#### **Simple Modules**
```bob
// math.bob
func sqrt(x) { return x ** 0.5; }
func max(a, b) { return a > b ? a : b; }

// main.bob
import "math.bob" as math;
var result = math.sqrt(16);
```

**Implementation Plan:**
- Add `import` statement syntax
- Implement module loading from files
- Support namespace aliases
- Create standard library modules

### **Phase 4: Language Enhancements (Optional)**

#### **Enhanced Standard Library**
```bob
// Additional string functions
var parts = "a,b,c".split(",");
var joined = ["a", "b", "c"].join("-");
var upper = "hello".toUpper();

// Math library
var result = Math.sqrt(16);
var max = Math.max(5, 10, 3);
```

#### **Async/Await (Advanced)**
```bob
async func fetchData() {
    var response = await http.get("api.com/data");
    return response.json();
}
```

## **Implementation Guidelines**

### **For Each New Feature:**
1. **Design**: Plan syntax and semantics carefully
2. **Lexer**: Add new tokens if needed
3. **Parser**: Add new expression/statement types
4. **AST**: Define new node types in Expression.h/Statement.h
5. **Evaluator/Executor**: Implement evaluation logic
6. **Testing**: Write tests for the new feature
7. **Documentation**: Update language reference

### **Development Approach:**
- **Testing**: Write tests for new features
- **Error Messages**: Make errors helpful and clear
- **Memory**: Use smart pointers to avoid leaks
- **Performance**: Don't make things unnecessarily slow
- **Code Style**: Keep it readable and maintainable
- **Portability**: Make sure it works on different platforms

## **Success Metrics**

### **What's Done ✅**
- [x] Core language syntax and semantics
- [x] All operators and expressions
- [x] Control flow (if, while, for, do-while)
- [x] Functions, closures, and tail call optimization
- [x] Arrays and dictionaries
- [x] Standard library (25+ built-in functions)
- [x] File I/O operations
- [x] Interactive REPL
- [x] Test suite with 70+ tests
- [x] Error handling and reporting
- [x] Memory management
- [x] CMake + Ninja build system
- [x] Modular architecture
- [x] Cross-platform support
- [x] Various optimizations

### **Might Add Later 📋**
- [ ] Exception handling (try/catch)
- [ ] Pattern matching
- [ ] Simple objects
- [ ] Module/import system
- [ ] More built-in functions
- [ ] Debugging tools

## **Resources**

- **[Language Reference](BOB_LANGUAGE_REFERENCE.md)** - Language documentation
- **[Build Guide](BUILD.md)** - How to build Bob
- **[Test Suite](../test_bob_language.bob)** - 70+ tests
- **[Crafting Interpreters](https://craftinginterpreters.com/)** - Helpful book for language implementation

## **Recent Work**

### **Architecture Cleanup (2025)**
- Split the interpreter into separate components (Evaluator/Executor/RuntimeDiagnostics)
- Switched to CMake + Ninja build system (3x faster builds)
- Reorganized code into cleaner modules
- Added Windows/macOS/Linux build support

### **Feature Completion**
- Added file I/O and type conversion functions
- Implemented all the operators I wanted (bitwise, compound assignment, etc.)
- Got arrays and dictionaries working properly
- Added tail call optimization and closures

### **Testing & Polish**
- Wrote 70+ tests covering pretty much everything
- Improved error messages to be more helpful
- Fixed memory leaks using smart pointers
- Various performance improvements

---

Bob works well for what I wanted - a programming language with the features and syntax I prefer.

*Last updated: January 2025*