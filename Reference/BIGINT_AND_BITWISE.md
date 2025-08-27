# BigInt and Bitwise Operations in Bob

## Overview

Bob provides comprehensive support for arbitrary-precision integers (BigInt) and full bitwise operations, matching Python's capabilities. This document covers the BigInt type system, bitwise operations, and their implementation details.

## BigInt Type System

### Three Numeric Types

Bob supports three distinct numeric types with automatic type promotion:

1. **`integer`**: 64-bit signed integers (long long range: -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807)
2. **`number`**: 64-bit floating-point numbers (double precision)
3. **`bigint`**: Arbitrary-precision integers (no size limit)

### Automatic Type Promotion

```bob
var small = 42;                      // type: "integer"
var large = 9999999999999999999;     // type: "bigint"
var result = small + large;          // type: "bigint" (automatic promotion)

print(type(42));                     // "integer"
print(type(3.14));                   // "number"
print(type(9999999999999999999));    // "bigint"
```

### Integer Overflow Detection

Bob automatically detects integer overflow and promotes to BigInt to prevent precision loss:

```bob
var max_int = 9223372036854775807;  // LLONG_MAX

// All these operations detect overflow and promote to BigInt:
max_int++;                          // 9223372036854775808 (BigInt)
var result1 = max_int + 1;          // 9223372036854775808 (BigInt)
var result2 = max_int * 2;          // 18446744073709551614 (BigInt)

// Supported operators with overflow detection:
// +, -, *, ++, --, +=, -=, *=
```

### Type Checking

Use the `type()` function to check the runtime type of any value:

```bob
func checkType(value) {
    var t = type(value);
    if (t == "integer") {
        print("Regular integer");
    } else if (t == "number") {
        print("Floating-point number");
    } else if (t == "bigint") {
        print("Arbitrary-precision integer");
    }
}
```

## Bitwise Operations

### Supported Operations

Bob supports all standard bitwise operations on all numeric types:

- **`&`**: Bitwise AND
- **`|`**: Bitwise OR
- **`^`**: Bitwise XOR
- **`<<`**: Left shift
- **`>>`**: Right shift
- **`~`**: Bitwise NOT (unary)

### BigInt Bitwise Support

Unlike many languages, Bob provides **full BigInt bitwise operations** with no size limitations:

```bob
var big_num = 9999999999999999999;

// All operations work with BigInt
print(big_num & 10);     // 10 (type: bigint)
print(big_num | 10);     // 9999999999999999999 (type: bigint)
print(big_num ^ 10);     // 9999999999999999989 (type: bigint)
print(big_num << 2);     // 39999999999999999996 (type: bigint)
print(big_num >> 2);     // 2499999999999999999 (type: bigint)
```

### Mixed Type Operations

Bitwise operations work seamlessly between different numeric types:

```bob
var small = 42;
var large = 9999999999999999999;

// Integer & BigInt
print(small & large);    // 42 (type: bigint)
print(large & small);    // 42 (type: bigint)

// Number & BigInt
print(3.14 & large);     // 3 (type: bigint) - number converted to integer
print(large & 3.14);     // 3 (type: bigint) - number converted to integer
```

### Examples

#### Basic Bitwise Operations

```bob
var a = 42;
var b = 10;

print(a & b);    // 10 (AND)
print(a | b);    // 42 (OR)
print(a ^ b);    // 32 (XOR)
print(a << 2);   // 168 (left shift)
print(a >> 1);   // 21 (right shift)
```

#### BigInt Operations

```bob
var huge = 9999999999999999999;

// Operations with small values
print(huge & 255);       // 255 (works with any size)
print(huge | 1);         // 9999999999999999999
print(huge ^ 1);         // 9999999999999999998

// Large shift operations
print(huge << 10);       // 10239999999999999998976
print(huge >> 10);       // 9765624999999999
```

#### Edge Cases

```bob
var zero = 0;
var one = 1;
var big_num = 9999999999999999999;

// Zero operations
print(zero & big_num);   // 0
print(zero | big_num);   // 9999999999999999999
print(zero ^ big_num);   // 9999999999999999999

// One operations
print(one & big_num);    // 1
print(one | big_num);    // 9999999999999999999
print(one ^ big_num);    // 9999999999999999998

// Self operations
print(big_num ^ big_num); // 0 (XOR with self always gives 0)
```

## Mathematical Properties

Bob's bitwise operations preserve all standard mathematical properties:

### Commutative Property
```bob
var a = 42;
var b = 9999999999999999999;

print(a & b == b & a);   // true
print(a | b == b | a);   // true
print(a ^ b == b ^ a);   // true
```

### Identity Properties
```bob
var big_num = 9999999999999999999;

print(big_num & 0 == 0);           // true (AND identity)
print(big_num | 0 == big_num);     // true (OR identity)
print(big_num ^ 0 == big_num);     // true (XOR identity)
```

### Shift Properties
```bob
var big_num = 9999999999999999999;

print(big_num << 0 == big_num);    // true (left shift identity)
print(big_num >> 0 == big_num);    // true (right shift identity)
print((big_num << 2) >> 2 == big_num); // true (shift cancellation)
```

## Error Handling

### Invalid Operations

Bob provides clear error messages for invalid bitwise operations:

```bob
try {
    var result = big_num << -1;  // Negative shift amount
} catch (e) {
    print("Error: " + e);  // "Negative shift amount not allowed"
}

try {
    var result = big_num >> -1;  // Negative shift amount
} catch (e) {
    print("Error: " + e);  // "Negative shift amount not allowed"
}
```

### Type Errors

```bob
try {
    var result = "hello" & 42;  // String & integer
} catch (e) {
    print("Error: " + e);  // Type error message
}
```

## Implementation Details

### GMP Integration

Bob uses the GNU Multiple Precision Arithmetic Library (GMP) for BigInt operations:

- **Arithmetic**: `mpz_add`, `mpz_sub`, `mpz_mul`, `mpz_fdiv_q`, `mpz_fdiv_r`
- **Bitwise**: `mpz_and`, `mpz_ior`, `mpz_xor`, `mpz_com`
- **Shifts**: `mpz_mul_2exp`, `mpz_fdiv_q_2exp`
- **Comparison**: `mpz_cmp` for all comparison operations

### Performance Characteristics

- **Small integers**: Operations are as fast as native integer operations
- **Large BigInt**: Operations scale with the size of the numbers
- **Memory**: Automatic memory management with reference counting
- **Type conversion**: Efficient conversion between integer, number, and BigInt

### Memory Management

```bob
// BigInt values are automatically managed
var big1 = 9999999999999999999;
var big2 = big1 * big1;  // Creates new BigInt, old one cleaned up automatically
var big3 = big1 & big2;  // Another BigInt operation
```

## Comparison with Other Languages

### Python Compatibility

Bob's BigInt bitwise operations are designed to match Python's behavior:

```python
# Python
big_num = 9999999999999999999
print(big_num & 10)  # 10
print(big_num | 10)  # 9999999999999999999
print(big_num << 2)  # 39999999999999999996
```

```bob
// Bob (same results)
var big_num = 9999999999999999999;
print(big_num & 10);     // 10
print(big_num | 10);     // 9999999999999999999
print(big_num << 2);     // 39999999999999999996
```

### JavaScript Comparison

Unlike JavaScript (which limits BigInt bitwise operations), Bob provides full support:

```javascript
// JavaScript - limited BigInt bitwise support
const big = 9999999999999999999n;
console.log(big & 10n);  // Works
console.log(big << 2n);  // Works
```

```bob
// Bob - full BigInt bitwise support
var big = 9999999999999999999;
print(big & 10);     // Works
print(big << 2);     // Works
print(big | 10);     // Works
print(big ^ 10);     // Works
```

## Best Practices

### When to Use BigInt

```bob
// Use BigInt for large numbers
var large_calculation = 9999999999999999999 * 9999999999999999999;

// Use regular integers for small numbers
var small_calculation = 42 + 10;

// Automatic promotion handles the rest
var mixed = large_calculation + small_calculation;  // Result is BigInt
```

### Performance Considerations

```bob
// Efficient: Use regular integers when possible
var fast = 42 & 10;  // Regular integer operation

// Still efficient: Small BigInt operations
var small_bigint = 9999999999999999999 - 9999999999999999957;  // 42 as BigInt
var result = small_bigint & 10;  // Fast BigInt operation

// Slower: Very large BigInt operations
var huge = 9999999999999999999 * 9999999999999999999 * 9999999999999999999;
var slow_result = huge & 255;  // Slower but still works
```

### Type Checking

```bob
func safeBitwiseOperation(a, b) {
    var type_a = type(a);
    var type_b = type(b);
    
    if (type_a == "string" || type_b == "string") {
        throw "Cannot perform bitwise operations on strings";
    }
    
    return a & b;  // Safe to proceed
}
```

## Testing

Bob includes comprehensive tests for BigInt and bitwise operations:

- Basic operations with all numeric types
- Mixed type operations
- Edge cases (zero, one, self-operations)
- Very large numbers
- Error handling
- Mathematical property verification

Run the test suite to verify everything works:

```bash
./build/bin/bob test_comprehensive_bitwise.bob
./build/bin/bob test_final_verification.bob
```

## Summary

Bob provides **full Python-like BigInt bitwise operations** with:

- ✅ **No size limitations** for bitwise operations
- ✅ **All operations work** with arbitrary-precision integers
- ✅ **Automatic type conversion** between integer, number, and BigInt
- ✅ **Mathematical correctness** with all standard properties preserved
- ✅ **Efficient implementation** using GMP library
- ✅ **Comprehensive error handling** for invalid operations
- ✅ **Extensive test coverage** ensuring reliability

This makes Bob one of the few languages that provides complete BigInt bitwise support comparable to Python.
