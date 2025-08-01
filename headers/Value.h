#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <cmath>
#include <stdexcept>
#include <algorithm>

// Forward declarations
class Environment;
class Function;
class BuiltinFunction;

// Type tags for the Value union
enum ValueType : uint8_t {
    VAL_NONE,
    VAL_NUMBER,
    VAL_BOOLEAN,
    VAL_STRING,
    VAL_FUNCTION,
    VAL_BUILTIN_FUNCTION
};

// Tagged value system (like Lua) - no heap allocation for simple values
struct Value {
    union {
        double number;
        bool boolean;
        Function* function;
        BuiltinFunction* builtin_function;
    };
    ValueType type;
    std::string string_value; // Store strings outside the union for safety

    // Constructors
    Value() : number(0.0), type(ValueType::VAL_NONE) {}
    Value(double n) : number(n), type(ValueType::VAL_NUMBER) {}
    Value(bool b) : boolean(b), type(ValueType::VAL_BOOLEAN) {}
    Value(const char* s) : type(ValueType::VAL_STRING), string_value(s ? s : "") {}
    Value(const std::string& s) : type(ValueType::VAL_STRING), string_value(s) {}
    Value(std::string&& s) : type(ValueType::VAL_STRING), string_value(std::move(s)) {}
    Value(Function* f) : function(f), type(ValueType::VAL_FUNCTION) {}
    Value(BuiltinFunction* bf) : builtin_function(bf), type(ValueType::VAL_BUILTIN_FUNCTION) {}

    // Move constructor
    Value(Value&& other) noexcept 
        : type(other.type), string_value(std::move(other.string_value)) {
        if (type != ValueType::VAL_STRING) {
            number = other.number; // Copy the union
        }
        other.type = ValueType::VAL_NONE;
    }

    // Move assignment
    Value& operator=(Value&& other) noexcept {
        if (this != &other) {
            type = other.type;
            if (type == ValueType::VAL_STRING) {
                string_value = std::move(other.string_value);
            } else {
                number = other.number; // Copy the union
            }
            other.type = ValueType::VAL_NONE;
        }
        return *this;
    }

    // Copy constructor (only when needed)
    Value(const Value& other) : type(other.type) {
        if (type == ValueType::VAL_STRING) {
            string_value = other.string_value;
        } else {
            number = other.number; // Copy the union
        }
    }

    // Copy assignment (only when needed)
    Value& operator=(const Value& other) {
        if (this != &other) {
            type = other.type;
            if (type == ValueType::VAL_STRING) {
                string_value = other.string_value;
            } else {
                number = other.number; // Copy the union
            }
        }
        return *this;
    }

    // Type checking (fast, no dynamic casting) - inline for performance
    inline bool isNumber() const { return type == ValueType::VAL_NUMBER; }
    inline bool isBoolean() const { return type == ValueType::VAL_BOOLEAN; }
    inline bool isString() const { return type == ValueType::VAL_STRING; }
    inline bool isFunction() const { return type == ValueType::VAL_FUNCTION; }
    inline bool isBuiltinFunction() const { return type == ValueType::VAL_BUILTIN_FUNCTION; }
    inline bool isNone() const { return type == ValueType::VAL_NONE; }

    // Value extraction (safe, with type checking) - inline for performance
    inline double asNumber() const { return isNumber() ? number : 0.0; }
    inline bool asBoolean() const { return isBoolean() ? boolean : false; }
    inline const std::string& asString() const { return string_value; }
    inline Function* asFunction() const { return isFunction() ? function : nullptr; }
    inline BuiltinFunction* asBuiltinFunction() const { return isBuiltinFunction() ? builtin_function : nullptr; }

    // Truthiness check - inline for performance
    inline bool isTruthy() const {
        switch (type) {
            case ValueType::VAL_NONE: return false;
            case ValueType::VAL_BOOLEAN: return boolean;
            case ValueType::VAL_NUMBER: return number != 0.0;
            case ValueType::VAL_STRING: return !string_value.empty();
            case ValueType::VAL_FUNCTION: return function != nullptr;
            case ValueType::VAL_BUILTIN_FUNCTION: return builtin_function != nullptr;
            default: return false;
        }
    }

    // Equality comparison - inline for performance
    inline bool equals(const Value& other) const {
        if (type != other.type) return false;
        
        switch (type) {
            case ValueType::VAL_NONE: return true;
            case ValueType::VAL_BOOLEAN: return boolean == other.boolean;
            case ValueType::VAL_NUMBER: return number == other.number;
            case ValueType::VAL_STRING: return string_value == other.string_value;
            case ValueType::VAL_FUNCTION: return function == other.function;
            case ValueType::VAL_BUILTIN_FUNCTION: return builtin_function == other.builtin_function;
            default: return false;
        }
    }

    // String representation
    std::string toString() const {
        switch (type) {
            case ValueType::VAL_NONE: return "none";
            case ValueType::VAL_BOOLEAN: return boolean ? "true" : "false";
            case ValueType::VAL_NUMBER: {
                // Format numbers like the original stringify function
                if (number == std::floor(number)) {
                    return std::to_string(static_cast<long long>(number));
                } else {
                    std::string str = std::to_string(number);
                    // Remove trailing zeros
                    str.erase(str.find_last_not_of('0') + 1, std::string::npos);
                    if (str.back() == '.') str.pop_back();
                    return str;
                }
            }
            case ValueType::VAL_STRING: return string_value;
            case ValueType::VAL_FUNCTION: return "<function>";
            case ValueType::VAL_BUILTIN_FUNCTION: return "<builtin_function>";
            default: return "unknown";
        }
    }

    // Arithmetic operators
    Value operator+(const Value& other) const {
        if (isNumber() && other.isNumber()) {
            return Value(number + other.number);
        }
        if (isString() && other.isString()) {
            return Value(string_value + other.string_value);
        }
        if (isString() && other.isNumber()) {
            return Value(string_value + other.toString());
        }
        if (isNumber() && other.isString()) {
            return Value(toString() + other.string_value);
        }
        throw std::runtime_error("Invalid operands for + operator");
    }

    Value operator-(const Value& other) const {
        if (isNumber() && other.isNumber()) {
            return Value(number - other.number);
        }
        throw std::runtime_error("Invalid operands for - operator");
    }

    Value operator*(const Value& other) const {
        if (isNumber() && other.isNumber()) {
            return Value(number * other.number);
        }
        if (isString() && other.isNumber()) {
            std::string result;
            for (int i = 0; i < static_cast<int>(other.number); ++i) {
                result += string_value;
            }
            return Value(result);
        }
        if (isNumber() && other.isString()) {
            std::string result;
            for (int i = 0; i < static_cast<int>(number); ++i) {
                result += other.string_value;
            }
            return Value(result);
        }
        throw std::runtime_error("Invalid operands for * operator");
    }

    Value operator/(const Value& other) const {
        if (isNumber() && other.isNumber()) {
            if (other.number == 0) {
                throw std::runtime_error("Division by zero");
            }
            return Value(number / other.number);
        }
        throw std::runtime_error("Invalid operands for / operator");
    }

    Value operator%(const Value& other) const {
        if (isNumber() && other.isNumber()) {
            return Value(fmod(number, other.number));
        }
        throw std::runtime_error("Invalid operands for % operator");
    }

    Value operator&(const Value& other) const {
        if (isNumber() && other.isNumber()) {
            return Value(static_cast<double>(static_cast<long>(number) & static_cast<long>(other.number)));
        }
        throw std::runtime_error("Invalid operands for & operator");
    }

    Value operator|(const Value& other) const {
        if (isNumber() && other.isNumber()) {
            return Value(static_cast<double>(static_cast<long>(number) | static_cast<long>(other.number)));
        }
        throw std::runtime_error("Invalid operands for | operator");
    }

    Value operator^(const Value& other) const {
        if (isNumber() && other.isNumber()) {
            return Value(static_cast<double>(static_cast<long>(number) ^ static_cast<long>(other.number)));
        }
        throw std::runtime_error("Invalid operands for ^ operator");
    }

    Value operator<<(const Value& other) const {
        if (isNumber() && other.isNumber()) {
            return Value(static_cast<double>(static_cast<long>(number) << static_cast<long>(other.number)));
        }
        throw std::runtime_error("Invalid operands for << operator");
    }

    Value operator>>(const Value& other) const {
        if (isNumber() && other.isNumber()) {
            return Value(static_cast<double>(static_cast<long>(number) >> static_cast<long>(other.number)));
        }
        throw std::runtime_error("Invalid operands for >> operator");
    }
};

// Global constants for common values
extern const Value NONE_VALUE;
extern const Value TRUE_VALUE;
extern const Value FALSE_VALUE;
extern const Value ZERO_VALUE;
extern const Value ONE_VALUE; 