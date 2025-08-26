#pragma once
#include "Value.h"
#include "Expression.h"

// Fast evaluator for common operations to avoid virtual function overhead
class FastEvaluator {
public:
    // Fast path for common integer operations
    static inline Value fastIntegerAdd(long long a, long long b) {
        return Value::fastIntegerAdd(a, b);
    }
    
    static inline Value fastIntegerSub(long long a, long long b) {
        return Value::fastIntegerSub(a, b);
    }
    
    // Fast path for common comparisons
    static inline Value fastIntegerCompare(long long a, long long b, TokenType op) {
        switch (op) {
            case GREATER: return Value(a > b);
            case GREATER_EQUAL: return Value(a >= b);
            case LESS: return Value(a < b);
            case LESS_EQUAL: return Value(a <= b);
            default: return Value(false);
        }
    }
};
