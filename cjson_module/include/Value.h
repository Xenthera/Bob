#pragma once
#include "helperFunctions/ErrorUtils.h"
#include "GMPWrapper.h"
#include "ValuePool.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <iostream>

// Forward declarations
struct Environment;
struct Function;
struct BuiltinFunction;
struct Thunk;
struct Module;

// Type tags for the Value union
enum ValueType {
    VAL_NONE,
    VAL_NUMBER,
    VAL_INTEGER,
    VAL_BIGINT,
    VAL_BOOLEAN,
    VAL_STRING,
    VAL_FUNCTION,
    VAL_BUILTIN_FUNCTION,
    VAL_THUNK,
    VAL_ARRAY,
    VAL_DICT,
    VAL_MODULE
};

// Tagged value system (like Lua) - no heap allocation for simple values
struct Value {
    union {
        double number;
        long long integer;
        bool boolean;
    };
    ValueType type;
    std::string string_value; // Store strings outside the union for safety
    std::shared_ptr<GMPWrapper::BigInt> bigint_value; // Store big integers using GMP
    std::shared_ptr<std::vector<Value> > array_value; // Store arrays as shared_ptr for mutability
    std::shared_ptr<std::unordered_map<std::string, Value> > dict_value; // Store dictionaries as shared_ptr for mutability
    std::shared_ptr<Module> module_value; // Module object
    
    // Store functions as shared_ptr for proper reference counting
    std::shared_ptr<Function> function;
    std::shared_ptr<BuiltinFunction> builtin_function;
    std::shared_ptr<Thunk> thunk;


    // Constructors
    Value() : number(0.0), type(ValueType::VAL_NONE) {}
    Value(double n) : number(n), type(ValueType::VAL_NUMBER) {}
    Value(long long i) : integer(i), type(ValueType::VAL_INTEGER) {}
    Value(const GMPWrapper::BigInt& bigint) : type(ValueType::VAL_BIGINT), bigint_value(std::make_shared<GMPWrapper::BigInt>(bigint)) {}
    Value(bool b) : boolean(b), type(ValueType::VAL_BOOLEAN) {}
    Value(const char* s) : type(ValueType::VAL_STRING), string_value(s ? s : "") {}
    Value(const std::string& s) : type(ValueType::VAL_STRING), string_value(s) {}
    Value(std::string&& s) : type(ValueType::VAL_STRING), string_value(std::move(s)) {}
    Value(std::shared_ptr<Function> f) : function(f), type(ValueType::VAL_FUNCTION) {}
    Value(std::shared_ptr<BuiltinFunction> bf) : builtin_function(bf), type(ValueType::VAL_BUILTIN_FUNCTION) {}
    Value(std::shared_ptr<Thunk> t) : thunk(t), type(ValueType::VAL_THUNK) {}
    Value(const std::vector<Value>& arr) : type(ValueType::VAL_ARRAY), array_value(std::make_shared<std::vector<Value> >(arr)) {}
    Value(std::vector<Value>&& arr) : type(ValueType::VAL_ARRAY), array_value(std::make_shared<std::vector<Value> >(std::move(arr))) {}
    Value(const std::unordered_map<std::string, Value>& dict) : type(ValueType::VAL_DICT), dict_value(std::make_shared<std::unordered_map<std::string, Value> >(dict)) {}
    Value(std::unordered_map<std::string, Value>&& dict) : type(ValueType::VAL_DICT), dict_value(std::make_shared<std::unordered_map<std::string, Value> >(std::move(dict))) {}
    Value(std::shared_ptr<Module> m) : type(ValueType::VAL_MODULE), module_value(std::move(m)) {}
    
    // Destructor to clean up functions and thunks
    ~Value() {
        // Functions and thunks are managed by the Interpreter, so we don't delete them
        // Arrays and dictionaries are managed by shared_ptr, so they clean up automatically
    }
    


    // Move constructor
    Value(Value&& other) noexcept 
        : type(other.type), string_value(std::move(other.string_value)), bigint_value(std::move(other.bigint_value)), array_value(std::move(other.array_value)), dict_value(std::move(other.dict_value)),
          function(std::move(other.function)), builtin_function(std::move(other.builtin_function)), thunk(std::move(other.thunk)), module_value(std::move(other.module_value)) {
        if (type != ValueType::VAL_STRING && type != ValueType::VAL_BIGINT && type != ValueType::VAL_ARRAY && type != ValueType::VAL_DICT && 
            type != ValueType::VAL_FUNCTION && type != ValueType::VAL_BUILTIN_FUNCTION && type != ValueType::VAL_THUNK) {
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
            } else if (type == ValueType::VAL_BIGINT) {
                bigint_value = std::move(other.bigint_value);
            } else if (type == ValueType::VAL_ARRAY) {
                array_value = std::move(other.array_value);
            } else if (type == ValueType::VAL_DICT) {
                dict_value = std::move(other.dict_value);
            } else if (type == ValueType::VAL_FUNCTION) {
                function = std::move(other.function);
            } else if (type == ValueType::VAL_BUILTIN_FUNCTION) {
                builtin_function = std::move(other.builtin_function);
            } else if (type == ValueType::VAL_THUNK) {
                thunk = std::move(other.thunk);
            } else if (type == ValueType::VAL_MODULE) {
                module_value = std::move(other.module_value);
            } else {
                number = other.number;
            }

            other.type = ValueType::VAL_NONE;
        }
        return *this;
    }

    // Copy constructor (only when needed)
    Value(const Value& other) : type(other.type) {
        if (type == ValueType::VAL_STRING) {
            string_value = other.string_value;
        } else if (type == ValueType::VAL_BIGINT) {
            bigint_value = other.bigint_value; // shared_ptr automatically handles sharing
        } else if (type == ValueType::VAL_ARRAY) {
            array_value = other.array_value; // shared_ptr automatically handles sharing
        } else if (type == ValueType::VAL_DICT) {
            dict_value = other.dict_value; // shared_ptr automatically handles sharing
        } else if (type == ValueType::VAL_FUNCTION) {
            function = other.function; // shared_ptr automatically handles sharing
        } else if (type == ValueType::VAL_BUILTIN_FUNCTION) {
            builtin_function = other.builtin_function; // shared_ptr automatically handles sharing
        } else if (type == ValueType::VAL_THUNK) {
            thunk = other.thunk; // shared_ptr automatically handles sharing
        } else if (type == ValueType::VAL_MODULE) {
            module_value = other.module_value; // shared module
        } else {
            number = other.number;
        }
    }

    // Copy assignment (only when needed)
    Value& operator=(const Value& other) {
        if (this != &other) {
            // First, clear all old shared_ptr members to release references
            bigint_value.reset();
            array_value.reset();
            dict_value.reset();
            function.reset();
            builtin_function.reset();
            thunk.reset();
            
            // Then set the new type and value
            type = other.type;
            if (type == ValueType::VAL_STRING) {
                string_value = other.string_value;
            } else if (type == ValueType::VAL_BIGINT) {
                bigint_value = other.bigint_value; // shared_ptr automatically handles sharing
            } else if (type == ValueType::VAL_ARRAY) {
                array_value = other.array_value; // shared_ptr automatically handles sharing
            } else if (type == ValueType::VAL_DICT) {
                dict_value = other.dict_value;
            } else if (type == ValueType::VAL_FUNCTION) {
                function = other.function; // shared_ptr automatically handles sharing
            } else if (type == ValueType::VAL_BUILTIN_FUNCTION) {
                builtin_function = other.builtin_function; // shared_ptr automatically handles sharing
            } else if (type == ValueType::VAL_THUNK) {
                thunk = other.thunk; // shared_ptr automatically handles sharing
            } else if (type == ValueType::VAL_MODULE) {
                module_value = other.module_value;
            } else {
                number = other.number;
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
    inline bool isArray() const { return type == ValueType::VAL_ARRAY; }
    inline bool isDict() const { return type == ValueType::VAL_DICT; }
    inline bool isModule() const { return type == ValueType::VAL_MODULE; }
    inline bool isThunk() const { return type == ValueType::VAL_THUNK; }
    inline bool isNone() const { return type == ValueType::VAL_NONE; }
    inline bool isInteger() const { return type == ValueType::VAL_INTEGER; }
    inline bool isBigInt() const { return type == ValueType::VAL_BIGINT; }
    inline bool isNumeric() const { return type == ValueType::VAL_NUMBER || type == ValueType::VAL_INTEGER || type == ValueType::VAL_BIGINT; }
    
    // Preprocessor method to automatically downgrade BigInt to regular integer if possible
    // (Kept for backward compatibility, but now we have full BigInt bitwise support)
    inline Value preprocessForBitwise() const {
        if (isBigInt() && bigint_value->fitsInLongLong()) {
            return Value(bigint_value->toLongLong());
        }
        return *this;
    }

    // Get type name as string for error messages
    inline std::string getType() const {
        switch (type) {
            case ValueType::VAL_NONE: return "none";
            case ValueType::VAL_NUMBER: return "number";
            case ValueType::VAL_INTEGER: return "integer";
            case ValueType::VAL_BIGINT: return "bigint";
            case ValueType::VAL_BOOLEAN: return "boolean";
            case ValueType::VAL_STRING: return "string";
            case ValueType::VAL_FUNCTION: return "function";
            case ValueType::VAL_BUILTIN_FUNCTION: return "builtin_function";
            case ValueType::VAL_THUNK: return "thunk";
            case ValueType::VAL_ARRAY: return "array";
            case ValueType::VAL_DICT: return "dict";
            case ValueType::VAL_MODULE: return "module";
            default: return "unknown";
        }
    }
    


    // Value extraction (safe, with type checking) - inline for performance
    inline double asNumber() const { return isNumber() ? number : 0.0; }
    inline long long asInteger() const { return isInteger() ? integer : 0; }
    inline const GMPWrapper::BigInt& asBigInt() const { return *bigint_value; }
    inline bool asBoolean() const { return isBoolean() ? boolean : false; }
    inline const std::string& asString() const { return string_value; }
    inline const std::vector<Value>& asArray() const { 
        return *array_value; 
    }
    inline std::vector<Value>& asArray() { 
        return *array_value; 
    }
    inline const std::unordered_map<std::string, Value>& asDict() const { 
        return *dict_value; 
    }
    inline std::unordered_map<std::string, Value>& asDict() { 
        return *dict_value; 
    }
    inline Module* asModule() const { return isModule() ? module_value.get() : nullptr; }
    inline Function* asFunction() const { return isFunction() ? function.get() : nullptr; }
    inline BuiltinFunction* asBuiltinFunction() const { return isBuiltinFunction() ? builtin_function.get() : nullptr; }
    inline Thunk* asThunk() const { return isThunk() ? thunk.get() : nullptr; }

    // Truthiness check - inline for performance
    inline bool isTruthy() const {
        switch (type) {
            case ValueType::VAL_NONE: return false;
            case ValueType::VAL_BOOLEAN: return boolean;
            case ValueType::VAL_NUMBER: return number != 0.0;
            case ValueType::VAL_INTEGER: return integer != 0;
            case ValueType::VAL_BIGINT: return *bigint_value != GMPWrapper::BigInt(0);
            case ValueType::VAL_STRING: return !string_value.empty();
            case ValueType::VAL_FUNCTION: return function != nullptr;
            case ValueType::VAL_BUILTIN_FUNCTION: return builtin_function != nullptr;
            case ValueType::VAL_THUNK: return thunk != nullptr;
            case ValueType::VAL_ARRAY: return !array_value->empty();
            case ValueType::VAL_DICT: return !dict_value->empty();
            case ValueType::VAL_MODULE: return module_value != nullptr;
            default: return false;
        }
    }

    // Equality comparison - inline for performance
    inline bool equals(const Value& other) const {
        // Special handling for numeric types - allow cross-type comparison
        if (isNumeric() && other.isNumeric()) {
            if (isInteger() && other.isInteger()) {
                return integer == other.integer;
            }
            if (isNumber() && other.isNumber()) {
                return number == other.number;
            }
            if (isInteger() && other.isNumber()) {
                return static_cast<double>(integer) == other.number;
            }
            if (isNumber() && other.isInteger()) {
                return number == static_cast<double>(other.integer);
            }
            if (isBigInt() && other.isBigInt()) {
                return *bigint_value == *other.bigint_value;
            }
            // For mixed bigint comparisons, convert appropriately
            if (isBigInt() && other.isInteger()) {
                return *bigint_value == GMPWrapper::BigInt::fromLongLong(other.integer);
            }
            if (isInteger() && other.isBigInt()) {
                return GMPWrapper::BigInt::fromLongLong(integer) == *other.bigint_value;
            }
            if (isBigInt() && other.isNumber()) {
                return *bigint_value == GMPWrapper::doubleToBigInt(other.number);
            }
            if (isNumber() && other.isBigInt()) {
                return GMPWrapper::doubleToBigInt(number) == *other.bigint_value;
            }
        }
        
        // JavaScript-like truthiness comparison
        if (isBoolean() && other.isNumeric()) {
            if (other.isInteger()) {
                return (boolean && other.integer != 0) || (!boolean && other.integer == 0);
            } else if (other.isNumber()) {
                return (boolean && other.number != 0.0) || (!boolean && other.number == 0.0);
            } else if (other.isBigInt()) {
                return (boolean && *other.bigint_value != GMPWrapper::BigInt(0)) || (!boolean && *other.bigint_value == GMPWrapper::BigInt(0));
            }
        }
        
        if (isNumeric() && other.isBoolean()) {
            if (isInteger()) {
                return (other.boolean && integer != 0) || (!other.boolean && integer == 0);
            } else if (isNumber()) {
                return (other.boolean && number != 0.0) || (!other.boolean && number == 0.0);
            } else if (isBigInt()) {
                return (other.boolean && *bigint_value != GMPWrapper::BigInt(0)) || (!other.boolean && *bigint_value == GMPWrapper::BigInt(0));
            }
        }
        
        if (type != other.type) return false;
        
        switch (type) {
            case ValueType::VAL_NONE: return true;
            case ValueType::VAL_BOOLEAN: return boolean == other.boolean;
            case ValueType::VAL_NUMBER: return number == other.number;
            case ValueType::VAL_INTEGER: return integer == other.integer;
            case ValueType::VAL_BIGINT: return *bigint_value == *other.bigint_value;
            case ValueType::VAL_STRING: return string_value == other.string_value;
            case ValueType::VAL_FUNCTION: return function == other.function;
            case ValueType::VAL_BUILTIN_FUNCTION: return builtin_function == other.builtin_function;
            case ValueType::VAL_THUNK: return thunk == other.thunk;
            case ValueType::VAL_ARRAY: {
                if (array_value->size() != other.array_value->size()) return false;
                for (size_t i = 0; i < array_value->size(); i++) {
                    if (!(*array_value)[i].equals((*other.array_value)[i])) return false;
                }
                return true;
            }
            case ValueType::VAL_DICT: {
                if (dict_value->size() != other.dict_value->size()) return false;
                for (const auto& pair : *dict_value) {
                    auto it = other.dict_value->find(pair.first);
                    if (it == other.dict_value->end() || !pair.second.equals(it->second)) return false;
                }
                return true;
            }
            default: return false;
        }
    }

    // String representation
    std::string toString() const {
        switch (type) {
            case ValueType::VAL_NONE: return "none";
            case ValueType::VAL_BOOLEAN: return boolean ? "true" : "false";
            case ValueType::VAL_INTEGER: return std::to_string(integer);
            case ValueType::VAL_BIGINT: return bigint_value->toString();
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
            case ValueType::VAL_THUNK: return "<thunk>";
            case ValueType::VAL_ARRAY: {
                const std::vector<Value>& arr = *array_value;
                std::string result = "[";
                
                for (size_t i = 0; i < arr.size(); i++) {
                    if (i > 0) result += ", ";
                    result += arr[i].toString();
                }
                
                result += "]";
                return result;
            }
            case ValueType::VAL_DICT: {
                const std::unordered_map<std::string, Value>& dict = *dict_value;
                std::string result = "{";
                
                bool first = true;
                for (const auto& pair : dict) {
                    if (!first) result += ", ";
                    result += "\"" + pair.first + "\": " + pair.second.toString();
                    first = false;
                }
                
                result += "}";
                return result;
            }
            case ValueType::VAL_MODULE: {
                // Avoid accessing Module fields when it's still an incomplete type in some TUs.
                // Delegate formatting to a small helper defined out-of-line in Value.cpp.
                extern std::string formatModuleForToString(const std::shared_ptr<Module>&);
                return formatModuleForToString(module_value);
            }
            default: return "unknown";
        }
    }

    // Equality operator
    bool operator==(const Value& other) const {
        return equals(other);
    }
    
    bool operator!=(const Value& other) const {
        return !equals(other);
    }

    // Comparison operators
    bool operator<(const Value& other) const {
        // Special handling for numeric types - allow cross-type comparison
        if (isNumeric() && other.isNumeric()) {
            if (isInteger() && other.isInteger()) {
                return integer < other.integer;
            }
            if (isNumber() && other.isNumber()) {
                return number < other.number;
            }
            if (isInteger() && other.isNumber()) {
                return static_cast<double>(integer) < other.number;
            }
            if (isNumber() && other.isInteger()) {
                return number < static_cast<double>(other.integer);
            }
            if (isBigInt() && other.isBigInt()) {
                return *bigint_value < *other.bigint_value;
            }
            // For mixed bigint comparisons, convert appropriately
            if (isBigInt() && other.isInteger()) {
                return *bigint_value < GMPWrapper::BigInt::fromLongLong(other.integer);
            }
            if (isInteger() && other.isBigInt()) {
                return GMPWrapper::BigInt::fromLongLong(integer) < *other.bigint_value;
            }
            if (isBigInt() && other.isNumber()) {
                return *bigint_value < GMPWrapper::doubleToBigInt(other.number);
            }
            if (isNumber() && other.isBigInt()) {
                return GMPWrapper::doubleToBigInt(number) < *other.bigint_value;
            }
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError("<", getType(), other.getType()));
    }

    bool operator<=(const Value& other) const {
        // Special handling for numeric types - allow cross-type comparison
        if (isNumeric() && other.isNumeric()) {
            if (isInteger() && other.isInteger()) {
                return integer <= other.integer;
            }
            if (isNumber() && other.isNumber()) {
                return number <= other.number;
            }
            if (isInteger() && other.isNumber()) {
                return static_cast<double>(integer) <= other.number;
            }
            if (isNumber() && other.isInteger()) {
                return number <= static_cast<double>(other.integer);
            }
            if (isBigInt() && other.isBigInt()) {
                return *bigint_value <= *other.bigint_value;
            }
            // For mixed bigint comparisons, convert appropriately
            if (isBigInt() && other.isInteger()) {
                return *bigint_value <= GMPWrapper::BigInt::fromLongLong(other.integer);
            }
            if (isInteger() && other.isBigInt()) {
                return GMPWrapper::BigInt::fromLongLong(integer) <= *other.bigint_value;
            }
            if (isBigInt() && other.isNumber()) {
                return *bigint_value <= GMPWrapper::doubleToBigInt(other.number);
            }
            if (isNumber() && other.isBigInt()) {
                return GMPWrapper::doubleToBigInt(number) <= *other.bigint_value;
            }
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError("<=", getType(), other.getType()));
    }

    bool operator>(const Value& other) const {
        // Special handling for numeric types - allow cross-type comparison
        if (isNumeric() && other.isNumeric()) {
            if (isInteger() && other.isInteger()) {
                return integer > other.integer;
            }
            if (isNumber() && other.isNumber()) {
                return number > other.number;
            }
            if (isInteger() && other.isNumber()) {
                return static_cast<double>(integer) > other.number;
            }
            if (isNumber() && other.isInteger()) {
                return number > static_cast<double>(other.integer);
            }
            if (isBigInt() && other.isBigInt()) {
                return *bigint_value > *other.bigint_value;
            }
            // For mixed bigint comparisons, convert appropriately
            if (isBigInt() && other.isInteger()) {
                return *bigint_value > GMPWrapper::BigInt::fromLongLong(other.integer);
            }
            if (isInteger() && other.isBigInt()) {
                return GMPWrapper::BigInt::fromLongLong(integer) > *other.bigint_value;
            }
            if (isBigInt() && other.isNumber()) {
                return *bigint_value > GMPWrapper::doubleToBigInt(other.number);
            }
            if (isNumber() && other.isBigInt()) {
                return GMPWrapper::doubleToBigInt(number) > *other.bigint_value;
            }
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError(">", getType(), other.getType()));
    }

    bool operator>=(const Value& other) const {
        // Special handling for numeric types - allow cross-type comparison
        if (isNumeric() && other.isNumeric()) {
            if (isInteger() && other.isInteger()) {
                return integer >= other.integer;
            }
            if (isNumber() && other.isNumber()) {
                return number >= other.number;
            }
            if (isInteger() && other.isNumber()) {
                return static_cast<double>(integer) >= other.number;
            }
            if (isNumber() && other.isInteger()) {
                return number >= static_cast<double>(other.integer);
            }
            if (isBigInt() && other.isBigInt()) {
                return *bigint_value >= *other.bigint_value;
            }
            // For mixed bigint comparisons, convert appropriately
            if (isBigInt() && other.isInteger()) {
                return *bigint_value >= GMPWrapper::BigInt::fromLongLong(other.integer);
            }
            if (isInteger() && other.isBigInt()) {
                return GMPWrapper::BigInt::fromLongLong(integer) >= *other.bigint_value;
            }
            if (isBigInt() && other.isNumber()) {
                return *bigint_value >= GMPWrapper::doubleToBigInt(other.number);
            }
            if (isNumber() && other.isBigInt()) {
                return GMPWrapper::doubleToBigInt(number) >= *other.bigint_value;
            }
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError(">=", getType(), other.getType()));
    }

    // Fast path for common integer operations (deprecated - using regular operators now)
    static Value fastIntegerAdd(long long a, long long b) {
        long long result = a + b;
        // Check for overflow
        if ((result > 0 && a < 0 && b < 0) ||
            (result < 0 && a > 0 && b > 0)) {
            // Overflow occurred, promote to bigint
            return Value(GMPWrapper::BigInt::fromLongLong(a) + GMPWrapper::BigInt::fromLongLong(b));
        }
        return Value(result);
    }
    
    static Value fastIntegerSub(long long a, long long b) {
        long long result = a - b;
        // Check for overflow
        if ((result > 0 && a < 0 && b > 0) ||
            (result < 0 && a > 0 && b < 0)) {
            // Overflow occurred, promote to bigint
            return Value(GMPWrapper::BigInt::fromLongLong(a) - GMPWrapper::BigInt::fromLongLong(b));
        }
        return Value(result);
    }
    
    static Value fastIntegerMul(long long a, long long b) {
        // Check for overflow in multiplication
        if (a != 0 && b != 0) {
            // More robust overflow detection
            if (a > 0 && b > 0 && a > LLONG_MAX / b) {
                // Overflow occurred, promote to bigint
                return Value(GMPWrapper::BigInt::fromLongLong(a) * GMPWrapper::BigInt::fromLongLong(b));
            }
            if (a < 0 && b < 0 && a < LLONG_MAX / b) {
                // Overflow occurred, promote to bigint
                return Value(GMPWrapper::BigInt::fromLongLong(a) * GMPWrapper::BigInt::fromLongLong(b));
            }
            if (a > 0 && b < 0 && b < LLONG_MIN / a) {
                // Overflow occurred, promote to bigint
                return Value(GMPWrapper::BigInt::fromLongLong(a) * GMPWrapper::BigInt::fromLongLong(b));
            }
            if (a < 0 && b > 0 && a < LLONG_MIN / b) {
                // Overflow occurred, promote to bigint
                return Value(GMPWrapper::BigInt::fromLongLong(a) * GMPWrapper::BigInt::fromLongLong(b));
            }
            long long result = a * b;
            return Value(result);
        }
        return Value(0LL);
    }
    
    // Helper function to handle integer overflow and promote to BigInt when needed
    static Value handleIntegerOverflow(long long a, long long b, char operation) {
        long long result;
        bool overflow = false;
        
        switch (operation) {
            case '+':
                // Check for addition overflow
                if (b > 0 && a > LLONG_MAX - b) overflow = true;
                else if (b < 0 && a < LLONG_MIN - b) overflow = true;
                else result = a + b;
                break;
            case '-':
                // Check for subtraction overflow
                if (b < 0 && a > LLONG_MAX + b) overflow = true;
                else if (b > 0 && a < LLONG_MIN + b) overflow = true;
                else result = a - b;
                break;
            case '*':
                // Check for multiplication overflow
                if (a != 0 && b != 0) {
                    if (a > 0 && b > 0 && a > LLONG_MAX / b) overflow = true;
                    else if (a > 0 && b < 0 && b < LLONG_MIN / a) overflow = true;
                    else if (a < 0 && b > 0 && a < LLONG_MIN / b) overflow = true;
                    else if (a < 0 && b < 0 && a < LLONG_MAX / b) overflow = true;
                    else result = a * b;
                } else {
                    result = 0;
                }
                break;
            default:
                result = 0LL; // Shouldn't happen
        }
        
        if (overflow) {
            // Promote to BigInt to handle overflow
            GMPWrapper::BigInt big_a = GMPWrapper::BigInt::fromLongLong(a);
            GMPWrapper::BigInt big_b = GMPWrapper::BigInt::fromLongLong(b);
            
            switch (operation) {
                case '+': return Value(big_a + big_b);
                case '-': return Value(big_a - big_b);
                case '*': return Value(big_a * big_b);
                default: return Value(0LL);
            }
        }
        
        return Value(result);
    }

    // Arithmetic operators
    Value operator+(const Value& other) const {
        // Integer + Integer (fast native operations with overflow detection)
        if (isInteger() && other.isInteger()) {
            return handleIntegerOverflow(integer, other.integer, '+');
        }
        
        // Number + Number (existing logic)
        if (isNumber() && other.isNumber()) {
            double result = number + other.number;
            // Auto-promote to bigint if needed
            if (GMPWrapper::shouldPromoteToBigInt(result)) {
                return Value(GMPWrapper::doubleToBigInt(result));
            }
            return Value(result);
        }
        
        // BigInt operations (existing logic)
        if (isBigInt() && other.isBigInt()) {
            return Value(*bigint_value + *other.bigint_value);
        }
        
        // Mixed operations - convert to appropriate type
        if (isInteger() && other.isBigInt()) {
            return Value(GMPWrapper::BigInt::fromLongLong(integer) + *other.bigint_value);
        }
        if (isBigInt() && other.isInteger()) {
            return Value(*bigint_value + GMPWrapper::BigInt::fromLongLong(other.integer));
        }
        if (isNumber() && other.isBigInt()) {
            GMPWrapper::BigInt num_bigint = GMPWrapper::doubleToBigInt(number);
            return Value(num_bigint + *other.bigint_value);
        }
        if (isBigInt() && other.isNumber()) {
            GMPWrapper::BigInt other_bigint = GMPWrapper::doubleToBigInt(other.number);
            return Value(*bigint_value + other_bigint);
        }
        if (isInteger() && other.isNumber()) {
            // Convert integer to double and add
            double result = static_cast<double>(integer) + other.number;
            // Check if result is actually an integer that fits in long long
            if (result == std::floor(result) && result >= LLONG_MIN && result <= LLONG_MAX) {
                return Value(static_cast<long long>(result));
            }
            return Value(result);
        }
        if (isNumber() && other.isInteger()) {
            // For large integers, avoid double precision loss by doing integer arithmetic
            if (other.integer > 9007199254740991LL || other.integer < -9007199254740991LL) {
                // Convert double to long long if it's a whole number, otherwise keep as double
                if (number == std::floor(number) && number >= LLONG_MIN && number <= LLONG_MAX) {
                    long long num_int = static_cast<long long>(number);
                    long long result = num_int + other.integer;
                    return Value(result);
                }
            }
            // Convert other integer to double and add
            double result = number + static_cast<double>(other.integer);
            // Check if result is actually an integer that fits in long long
            if (result == std::floor(result) && result >= LLONG_MIN && result <= LLONG_MAX) {
                return Value(static_cast<long long>(result));
            }
            return Value(result);
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
        // Handle none values by converting to string
        if (isString() && other.isNone()) {
            return Value(string_value + "none");
        }
        if (isNone() && other.isString()) {
            return Value("none" + other.string_value);
        }
        if (isString() && !other.isString() && !other.isNumber()) {
            return Value(string_value + other.toString());
        }
        if (!isString() && !isNumber() && other.isString()) {
            return Value(toString() + other.string_value);
        }
        if (isArray() && other.isArray()) {
            std::vector<Value> result;
            const std::vector<Value>& arr1 = *array_value;
            const std::vector<Value>& arr2 = *other.array_value;
            result.reserve(arr1.size() + arr2.size());
            result.insert(result.end(), arr1.begin(), arr1.end());
            result.insert(result.end(), arr2.begin(), arr2.end());
            return Value(result);
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError("+", getType(), other.getType()));
    }

    Value operator-(const Value& other) const {
        // Integer - Integer (fast native operations with overflow detection)
        if (isInteger() && other.isInteger()) {
            return handleIntegerOverflow(integer, other.integer, '-');
        }
        
        // Number - Number (existing logic)
        if (isNumber() && other.isNumber()) {
            double result = number - other.number;
            // Auto-promote to bigint if needed
            if (GMPWrapper::shouldPromoteToBigInt(result)) {
                return Value(GMPWrapper::doubleToBigInt(result));
            }
            return Value(result);
        }
        
        // BigInt operations (existing logic)
        if (isBigInt() && other.isBigInt()) {
            return Value(*bigint_value - *other.bigint_value);
        }
        
        // Mixed operations - convert to appropriate type
        if (isInteger() && other.isBigInt()) {
            return Value(GMPWrapper::BigInt::fromLongLong(integer) - *other.bigint_value);
        }
        if (isBigInt() && other.isInteger()) {
            return Value(*bigint_value - GMPWrapper::BigInt::fromLongLong(other.integer));
        }
        if (isNumber() && other.isBigInt()) {
            GMPWrapper::BigInt num_bigint = GMPWrapper::doubleToBigInt(number);
            return Value(num_bigint - *other.bigint_value);
        }
        if (isBigInt() && other.isNumber()) {
            GMPWrapper::BigInt other_bigint = GMPWrapper::doubleToBigInt(other.number);
            return Value(*bigint_value - other_bigint);
        }
        if (isInteger() && other.isNumber()) {
            // Convert integer to double and subtract
            return Value(static_cast<double>(integer) - other.number);
        }
        if (isNumber() && other.isInteger()) {
            // Convert other integer to double and subtract
            return Value(number - static_cast<double>(other.integer));
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError("-", getType(), other.getType()));
    }

    Value operator*(const Value& other) const {
        // Integer * Integer (fast native operations with overflow detection)
        if (isInteger() && other.isInteger()) {
            return handleIntegerOverflow(integer, other.integer, '*');
        }
        
        // Number * Number (existing logic)
        if (isNumber() && other.isNumber()) {
            double result = number * other.number;
            // Auto-promote to bigint if needed
            if (GMPWrapper::shouldPromoteToBigInt(result)) {
                return Value(GMPWrapper::doubleToBigInt(result));
            }
            return Value(result);
        }
        
        // BigInt operations (existing logic)
        if (isBigInt() && other.isBigInt()) {
            return Value(*bigint_value * *other.bigint_value);
        }
        
        // Mixed operations - convert to appropriate type
        if (isInteger() && other.isBigInt()) {
            return Value(GMPWrapper::BigInt::fromLongLong(integer) * *other.bigint_value);
        }
        if (isBigInt() && other.isInteger()) {
            return Value(*bigint_value * GMPWrapper::BigInt::fromLongLong(other.integer));
        }
        if (isNumber() && other.isBigInt()) {
            // Convert number to bigint and multiply
            GMPWrapper::BigInt num_bigint = GMPWrapper::doubleToBigInt(number);
            return Value(num_bigint * *other.bigint_value);
        }
        if (isBigInt() && other.isNumber()) {
            // Convert other number to bigint and multiply
            GMPWrapper::BigInt other_bigint = GMPWrapper::doubleToBigInt(other.number);
            return Value(*bigint_value * other_bigint);
        }
        if (isInteger() && other.isNumber()) {
            // Convert integer to double and multiply
            return Value(static_cast<double>(integer) * other.number);
        }
        if (isNumber() && other.isInteger()) {
            // Convert other integer to double and multiply
            return Value(number * static_cast<double>(other.integer));
        }
        if (isString() && other.isNumber()) {
            std::string result;
            for (int i = 0; i < static_cast<int>(other.number); ++i) {
                result += string_value;
            }
            return Value(result);
        }
        if (isString() && other.isInteger()) {
            std::string result;
            for (int i = 0; i < static_cast<int>(other.integer); ++i) {
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
        if (isInteger() && other.isString()) {
            std::string result;
            for (int i = 0; i < static_cast<int>(integer); ++i) {
                result += other.string_value;
            }
            return Value(result);
        }

        throw std::runtime_error(ErrorUtils::makeOperatorError("*", getType(), other.getType()));
    }

    Value operator/(const Value& other) const {
        if (isInteger() && other.isInteger()) {
            if (other.integer == 0) {
                throw std::runtime_error("Division by zero");
            }
            // Integer division - convert to double for result
            return Value(static_cast<double>(integer) / static_cast<double>(other.integer));
        }
        if (isNumber() && other.isNumber()) {
            if (other.number == 0) {
                throw std::runtime_error("Division by zero");
            }
            double result = number / other.number;
            // Auto-promote to bigint if needed
            if (GMPWrapper::shouldPromoteToBigInt(result)) {
                return Value(GMPWrapper::doubleToBigInt(result));
            }
            return Value(result);
        }
        if (isBigInt() && other.isBigInt()) {
            return Value(*bigint_value / *other.bigint_value);
        }
        if (isNumber() && other.isBigInt()) {
            // Convert number to bigint and divide
            GMPWrapper::BigInt num_bigint = GMPWrapper::doubleToBigInt(number);
            return Value(num_bigint / *other.bigint_value);
        }
        if (isBigInt() && other.isNumber()) {
            // Convert other number to bigint and divide
            GMPWrapper::BigInt other_bigint = GMPWrapper::doubleToBigInt(other.number);
            return Value(*bigint_value / other_bigint);
        }
        if (isInteger() && other.isNumber()) {
            // Convert integer to double and divide
            return Value(static_cast<double>(integer) / other.number);
        }
        if (isNumber() && other.isInteger()) {
            // Convert other integer to double and divide
            return Value(number / static_cast<double>(other.integer));
        }
        if (isInteger() && other.isBigInt()) {
            // Convert integer to bigint and divide
            GMPWrapper::BigInt int_bigint = GMPWrapper::BigInt::fromLongLong(integer);
            return Value(int_bigint / *other.bigint_value);
        }
        if (isBigInt() && other.isInteger()) {
            // Convert other integer to bigint and divide
            GMPWrapper::BigInt other_bigint = GMPWrapper::BigInt::fromLongLong(other.integer);
            return Value(*bigint_value / other_bigint);
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError("/", getType(), other.getType()));
    }

    Value operator%(const Value& other) const {
        if (isInteger() && other.isInteger()) {
            if (other.integer == 0) {
                throw std::runtime_error("Modulo by zero");
            }
            return Value(integer % other.integer);
        }
        if (isNumber() && other.isNumber()) {
            double result = fmod(number, other.number);
            // Auto-promote to bigint if needed
            if (GMPWrapper::shouldPromoteToBigInt(result)) {
                return Value(GMPWrapper::doubleToBigInt(result));
            }
            return Value(result);
        }
        if (isBigInt() && other.isBigInt()) {
            return Value(*bigint_value % *other.bigint_value);
        }
        if (isNumber() && other.isBigInt()) {
            // Convert number to bigint and modulo
            GMPWrapper::BigInt num_bigint = GMPWrapper::doubleToBigInt(number);
            return Value(num_bigint % *other.bigint_value);
        }
        if (isBigInt() && other.isNumber()) {
            // Convert other number to bigint and modulo
            GMPWrapper::BigInt other_bigint = GMPWrapper::doubleToBigInt(other.number);
            return Value(*bigint_value % other_bigint);
        }
        if (isInteger() && other.isNumber()) {
            // Convert integer to double and modulo
            return Value(fmod(static_cast<double>(integer), other.number));
        }
        if (isNumber() && other.isInteger()) {
            // Convert other integer to double and modulo
            return Value(fmod(number, static_cast<double>(other.integer)));
        }
        if (isInteger() && other.isBigInt()) {
            // Convert integer to bigint and modulo
            GMPWrapper::BigInt int_bigint = GMPWrapper::BigInt::fromLongLong(integer);
            return Value(int_bigint % *other.bigint_value);
        }
        if (isBigInt() && other.isInteger()) {
            // Convert other integer to bigint and modulo
            GMPWrapper::BigInt other_bigint = GMPWrapper::BigInt::fromLongLong(other.integer);
            return Value(*bigint_value % other_bigint);
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError("%", getType(), other.getType()));
    }

    Value operator&(const Value& other) const {
        if (isInteger() && other.isInteger()) {
            return Value(static_cast<long long>(integer & other.integer));
        }
        if (isNumber() && other.isNumber()) {
            return Value(static_cast<double>(static_cast<long>(number) & static_cast<long>(other.number)));
        }
        if (isInteger() && other.isNumber()) {
            return Value(static_cast<long long>(integer & static_cast<long long>(other.number)));
        }
        if (isNumber() && other.isInteger()) {
            return Value(static_cast<long long>(static_cast<long long>(number) & other.integer));
        }
        if (isBigInt() && other.isBigInt()) {
            return Value(*bigint_value & *other.bigint_value);
        }
        if (isBigInt() && other.isInteger()) {
            return Value(*bigint_value & GMPWrapper::BigInt::fromLongLong(other.integer));
        }
        if (isInteger() && other.isBigInt()) {
            return Value(GMPWrapper::BigInt::fromLongLong(integer) & *other.bigint_value);
        }
        if (isBigInt() && other.isNumber()) {
            return Value(*bigint_value & GMPWrapper::BigInt::fromLongLong(static_cast<long long>(other.number)));
        }
        if (isNumber() && other.isBigInt()) {
            return Value(GMPWrapper::BigInt::fromLongLong(static_cast<long long>(number)) & *other.bigint_value);
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError("&", getType(), other.getType()));
    }

    Value operator|(const Value& other) const {
        if (isInteger() && other.isInteger()) {
            return Value(static_cast<long long>(integer | other.integer));
        }
        if (isNumber() && other.isNumber()) {
            return Value(static_cast<double>(static_cast<long>(number) | static_cast<long>(other.number)));
        }
        if (isInteger() && other.isNumber()) {
            return Value(static_cast<long long>(integer | static_cast<long long>(other.number)));
        }
        if (isNumber() && other.isInteger()) {
            return Value(static_cast<long long>(static_cast<long long>(number) | other.integer));
        }
        if (isBigInt() && other.isBigInt()) {
            return Value(*bigint_value | *other.bigint_value);
        }
        if (isBigInt() && other.isInteger()) {
            return Value(*bigint_value | GMPWrapper::BigInt::fromLongLong(other.integer));
        }
        if (isInteger() && other.isBigInt()) {
            return Value(GMPWrapper::BigInt::fromLongLong(integer) | *other.bigint_value);
        }
        if (isBigInt() && other.isNumber()) {
            return Value(*bigint_value | GMPWrapper::BigInt::fromLongLong(static_cast<long long>(other.number)));
        }
        if (isNumber() && other.isBigInt()) {
            return Value(GMPWrapper::BigInt::fromLongLong(static_cast<long long>(number)) | *other.bigint_value);
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError("|", getType(), other.getType()));
    }

    Value operator^(const Value& other) const {
        if (isInteger() && other.isInteger()) {
            return Value(static_cast<long long>(integer ^ other.integer));
        }
        if (isNumber() && other.isNumber()) {
            return Value(static_cast<double>(static_cast<long>(number) ^ static_cast<long>(other.number)));
        }
        if (isInteger() && other.isNumber()) {
            return Value(static_cast<long long>(integer ^ static_cast<long long>(other.number)));
        }
        if (isNumber() && other.isInteger()) {
            return Value(static_cast<long long>(static_cast<long long>(number) ^ other.integer));
        }
        if (isBigInt() && other.isBigInt()) {
            return Value(*bigint_value ^ *other.bigint_value);
        }
        if (isBigInt() && other.isInteger()) {
            return Value(*bigint_value ^ GMPWrapper::BigInt::fromLongLong(other.integer));
        }
        if (isInteger() && other.isBigInt()) {
            return Value(GMPWrapper::BigInt::fromLongLong(integer) ^ *other.bigint_value);
        }
        if (isBigInt() && other.isNumber()) {
            return Value(*bigint_value ^ GMPWrapper::BigInt::fromLongLong(static_cast<long long>(other.number)));
        }
        if (isNumber() && other.isBigInt()) {
            return Value(GMPWrapper::BigInt::fromLongLong(static_cast<long long>(number)) ^ *other.bigint_value);
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError("^", getType(), other.getType()));
    }

    Value operator<<(const Value& other) const {
        if (isInteger() && other.isInteger()) {
            return Value(static_cast<long long>(integer << other.integer));
        }
        if (isNumber() && other.isNumber()) {
            return Value(static_cast<double>(static_cast<long>(number) << static_cast<long>(other.number)));
        }
        if (isInteger() && other.isNumber()) {
            return Value(static_cast<long long>(integer << static_cast<long long>(other.number)));
        }
        if (isNumber() && other.isInteger()) {
            return Value(static_cast<long long>(static_cast<long long>(number) << other.integer));
        }
        if (isBigInt() && other.isInteger()) {
            return Value(*bigint_value << static_cast<unsigned long>(other.integer));
        }
        if (isInteger() && other.isBigInt()) {
            // For shift operations, we need to check if the shift amount fits in unsigned long
            if (other.bigint_value->fitsInLongLong() && other.bigint_value->toLongLong() >= 0) {
                return Value(GMPWrapper::BigInt::fromLongLong(integer) << static_cast<unsigned long>(other.bigint_value->toLongLong()));
            }
            throw std::runtime_error("Shift amount too large for BigInt operations");
        }
        if (isBigInt() && other.isBigInt()) {
            // For shift operations, we need to check if the shift amount fits in unsigned long
            if (other.bigint_value->fitsInLongLong() && other.bigint_value->toLongLong() >= 0) {
                return Value(*bigint_value << static_cast<unsigned long>(other.bigint_value->toLongLong()));
            }
            throw std::runtime_error("Shift amount too large for BigInt operations");
        }
        if (isBigInt() && other.isNumber()) {
            long long shift = static_cast<long long>(other.number);
            if (shift >= 0) {
                return Value(*bigint_value << static_cast<unsigned long>(shift));
            }
            throw std::runtime_error("Negative shift amount not allowed");
        }
        if (isNumber() && other.isBigInt()) {
            if (other.bigint_value->fitsInLongLong() && other.bigint_value->toLongLong() >= 0) {
                return Value(GMPWrapper::BigInt::fromLongLong(static_cast<long long>(number)) << static_cast<unsigned long>(other.bigint_value->toLongLong()));
            }
            throw std::runtime_error("Shift amount too large for BigInt operations");
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError("<<", getType(), other.getType()));
    }

    Value operator>>(const Value& other) const {
        if (isInteger() && other.isInteger()) {
            return Value(static_cast<long long>(integer >> other.integer));
        }
        if (isNumber() && other.isNumber()) {
            return Value(static_cast<double>(static_cast<long>(number) >> static_cast<long>(other.number)));
        }
        if (isInteger() && other.isNumber()) {
            return Value(static_cast<long long>(integer >> static_cast<long long>(other.number)));
        }
        if (isNumber() && other.isInteger()) {
            return Value(static_cast<long long>(static_cast<long long>(number) >> other.integer));
        }
        if (isBigInt() && other.isInteger()) {
            return Value(*bigint_value >> static_cast<unsigned long>(other.integer));
        }
        if (isInteger() && other.isBigInt()) {
            // For shift operations, we need to check if the shift amount fits in unsigned long
            if (other.bigint_value->fitsInLongLong() && other.bigint_value->toLongLong() >= 0) {
                return Value(GMPWrapper::BigInt::fromLongLong(integer) >> static_cast<unsigned long>(other.bigint_value->toLongLong()));
            }
            throw std::runtime_error("Shift amount too large for BigInt operations");
        }
        if (isBigInt() && other.isBigInt()) {
            // For shift operations, we need to check if the shift amount fits in unsigned long
            if (other.bigint_value->fitsInLongLong() && other.bigint_value->toLongLong() >= 0) {
                return Value(*bigint_value >> static_cast<unsigned long>(other.bigint_value->toLongLong()));
            }
            throw std::runtime_error("Shift amount too large for BigInt operations");
        }
        if (isBigInt() && other.isNumber()) {
            long long shift = static_cast<long long>(other.number);
            if (shift >= 0) {
                return Value(*bigint_value >> static_cast<unsigned long>(shift));
            }
            throw std::runtime_error("Negative shift amount not allowed");
        }
        if (isNumber() && other.isBigInt()) {
            if (other.bigint_value->fitsInLongLong() && other.bigint_value->toLongLong() >= 0) {
                return Value(GMPWrapper::BigInt::fromLongLong(static_cast<long long>(number)) >> static_cast<unsigned long>(other.bigint_value->toLongLong()));
            }
            throw std::runtime_error("Shift amount too large for BigInt operations");
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError(">>", getType(), other.getType()));
    }
};

// Define Module after Value so it can hold Value in exports without incomplete type issues
struct Module {
    std::string name;
    std::shared_ptr<std::unordered_map<std::string, Value>> exports;
    Module() = default;
    Module(const std::string& n, const std::unordered_map<std::string, Value>& dict)
        : name(n), exports(std::make_shared<std::unordered_map<std::string, Value>>(dict)) {}
};

// Global constants for common values
extern const Value NONE_VALUE;
extern const Value TRUE_VALUE;
extern const Value FALSE_VALUE;