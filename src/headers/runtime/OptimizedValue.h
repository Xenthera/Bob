#pragma once
#include <cstdint>
#include <string>
#include <variant>

// Optimized value system using tagged unions and avoiding heap allocation
class OptimizedValue {
public:
    // Tag for the union
    enum class Tag : uint8_t {
        NONE = 0,
        INTEGER = 1,
        DOUBLE = 2,
        BOOLEAN = 3,
        STRING = 4,
        // Complex types still use heap allocation
        BIGINT = 5,
        ARRAY = 6,
        DICT = 7,
        FUNCTION = 8
    };
    
    // Union for simple types (no heap allocation)
    union SimpleValue {
        int64_t integer;
        double number;
        bool boolean;
        const char* string_ptr; // For small strings
        
        SimpleValue() : integer(0) {}
    };
    
    // Constructor for simple types
    OptimizedValue() : tag(Tag::NONE), simple() {}
    OptimizedValue(int64_t val) : tag(Tag::INTEGER), simple() { simple.integer = val; }
    OptimizedValue(double val) : tag(Tag::DOUBLE), simple() { simple.number = val; }
    OptimizedValue(bool val) : tag(Tag::BOOLEAN), simple() { simple.boolean = val; }
    
    // Fast arithmetic operations
    OptimizedValue operator+(const OptimizedValue& other) const;
    OptimizedValue operator-(const OptimizedValue& other) const;
    
    // Type checking (fast)
    bool isInteger() const { return tag == Tag::INTEGER; }
    bool isDouble() const { return tag == Tag::DOUBLE; }
    bool isBoolean() const { return tag == Tag::BOOLEAN; }
    
    // Value extraction (fast)
    int64_t asInteger() const { return simple.integer; }
    double asDouble() const { return simple.number; }
    bool asBoolean() const { return simple.boolean; }
    
private:
    Tag tag;
    SimpleValue simple;
    // Complex types would be stored as pointers here
};
