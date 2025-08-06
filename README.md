# Bob

A modern programming language focused on safety and clarity.

## Design Philosophy

Bob prioritizes safety and functional programming while keeping things practical.

## Assignment System

Bob uses two types of assignments to prevent bugs while keeping useful functionality:

### Assignment Statements
```go
var x = 5;
x = 10;           // Regular assignment
y += 5;           // Compound assignment
```

### Assignment Expressions (For Loops Only)
```go
for (var i = 0; i < 5; i = i + 1) { }  // Assignment in for loop
for (j = 0; j < 5; j += 1) { }         // Assignment in for loop
```

### Why This Design?

**Prevents Common Bugs:**
```go
// This would be a bug in many languages:
if (x = 10) { }  // Parse error in Bob - prevents accidental assignment

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

**Supports Functional Programming:**
```go
// Prevents confusing patterns:
var value = condition ? (x = 10) : (x = 20);  // Parse error

// Encourages clean patterns:
x = condition ? 10 : 20;  // Clear assignment
```

**Keeps For Loops Practical:**
```go
// For loops work naturally:
for (var i = 0; i < 10; i = i + 1) { }  // Assignment expressions allowed
for (j = 0; j < 10; j += 1) { }         // Assignment expressions allowed
```

### Benefits

1. **Prevents Common Bugs**: No accidental assignments in conditionals
2. **Expression Purity**: Expressions have no side effects
3. **Clear Semantics**: Assignments are clearly statements
4. **Functional Programming**: Encourages pure function patterns
5. **Practical For Loops**: Maintains useful for loop syntax
6. **Easier Debugging**: Clear separation of assignment and expression logic

## Features

- **Type Safety**: Dynamic typing with runtime type checking
- **Functional Programming**: First-class functions, closures, recursion
- **Memory Management**: Automatic memory management
- **Error Handling**: Comprehensive error reporting
- **Standard Library**: Built-in functions for common operations
- **Tail Call Optimization**: Efficient recursive function calls

## Documentation

See [BOB_LANGUAGE_REFERENCE.md](BOB_LANGUAGE_REFERENCE.md) for complete language documentation.

## Building

```bash
make clean && make
./build/bob your_file.bob
```

## Testing

```bash
./build/bob test_bob_language.bob
```
