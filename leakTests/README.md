# Bob Memory Leak Test Suite

This directory contains comprehensive memory leak tests for the Bob programming language. Each test file focuses on different scenarios that could potentially cause memory leaks.

## Test Files

### `leaktest_functions.bob`
Tests function-related memory scenarios:
- Recursive function closures
- Function factories (functions returning functions)
- Deep function nesting
- Circular function references
- Expected behavior: Memory should be freed when functions are cleared

### `leaktest_collections.bob`
Tests collection (arrays/dictionaries) memory scenarios:
- Large nested arrays
- Large nested dictionaries
- Mixed array/dict structures
- Self-referencing structures
- Large string collections
- Expected behavior: Collections should be properly freed when reassigned

### `leaktest_mixed.bob`
Tests mixed type scenarios and edge cases:
- Functions capturing collections
- Collections containing functions and mixed types
- Dynamic property assignment patterns
- Type reassignment chains
- Rapid allocation/deallocation cycles
- Expected behavior: Memory should be freed regardless of type mixing

### `leaktest_loops.bob`
Tests memory behavior in loops and repetitive operations:
- Nested loop allocation
- While loop accumulation
- Variable reassignment in loops
- Do-while function creation
- Complex loop control flow
- Memory churn tests
- Expected behavior: Loop-created objects should be freed when variables are reassigned

### `leaktest_builtin.bob`
Tests builtin function and stdlib memory behavior:
- Heavy string operations
- Type conversion stress tests
- Array/Dict builtin operations
- Eval function stress tests
- File I/O operations
- Random number generation
- Expected behavior: Builtin operations should not leak memory

## How to Run Tests

Run each test individually and monitor memory usage:

```bash
# Monitor memory before, during, and after each test
./build-ninja/bin/bob leakTests/leaktest_functions.bob
./build-ninja/bin/bob leakTests/leaktest_collections.bob
./build-ninja/bin/bob leakTests/leaktest_mixed.bob
./build-ninja/bin/bob leakTests/leaktest_loops.bob
./build-ninja/bin/bob leakTests/leaktest_builtin.bob
```

## Expected Behavior

After the memory leak fixes:
1. **Memory should increase** during object creation phases
2. **Memory should decrease** significantly when objects are cleared (set to `none`, `[]`, different types, etc.)
3. **Memory should return close to baseline** after each test section
4. **No gradual memory increase** across multiple test cycles

## Memory Monitoring

Use system tools to monitor memory:
- **macOS**: Activity Monitor or `top -pid $(pgrep bob)`
- **Linux**: `top`, `htop`, or `ps aux | grep bob`
- **Windows**: Task Manager or Process Monitor

Look for:
- Memory spikes during creation phases ✅ Expected
- Memory drops after "cleared" messages ✅ Expected  
- Memory staying high after clearing ❌ Potential leak
- Gradual increase across test cycles ❌ Potential leak

## Test Scenarios Covered

- **Object Types**: Functions, Arrays, Dictionaries, Strings, Numbers, Booleans
- **Memory Patterns**: Allocation, Deallocation, Reassignment, Type Changes
- **Edge Cases**: Circular references, Deep nesting, Self-references, Mixed types
- **Operations**: Loops, Builtin functions, File I/O, Type conversions
- **Cleanup Triggers**: Setting to `none`, `[]`, `{}`, different types, string values

This comprehensive test suite should help identify any remaining memory leak scenarios in the Bob interpreter.