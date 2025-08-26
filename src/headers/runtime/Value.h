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

    // Fast path for common integer operations
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
            long long result = a * b;
            if (result / a != b) {
                // Overflow occurred, promote to bigint
                return Value(GMPWrapper::BigInt::fromLongLong(a) * GMPWrapper::BigInt::fromLongLong(b));
            }
            return Value(result);
        }
        return Value(0LL);
    }
    
    // Arithmetic operators
    Value operator+(const Value& other) const {
        // Integer + Integer (fast native operations)
        if (isInteger() && other.isInteger()) {
            return fastIntegerAdd(integer, other.integer);
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
        throw std::runtime_error(ErrorUtils::makeOperatorError("+", getType(), other.getType()));
    }

    Value operator-(const Value& other) const {
        // Integer - Integer (fast native operations)
        if (isInteger() && other.isInteger()) {
            return fastIntegerSub(integer, other.integer);
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
        throw std::runtime_error(ErrorUtils::makeOperatorError("-", getType(), other.getType()));
    }

    Value operator*(const Value& other) const {
        // Integer * Integer (fast native operations)
        if (isInteger() && other.isInteger()) {
            return fastIntegerMul(integer, other.integer);
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
        throw std::runtime_error(ErrorUtils::makeOperatorError("*", getType(), other.getType()));
    }

    Value operator/(const Value& other) const {
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
        throw std::runtime_error(ErrorUtils::makeOperatorError("/", getType(), other.getType()));
    }

    Value operator%(const Value& other) const {
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
        throw std::runtime_error(ErrorUtils::makeOperatorError("%", getType(), other.getType()));
    }

    Value operator&(const Value& other) const {
        if (isNumber() && other.isNumber()) {
            return Value(static_cast<double>(static_cast<long>(number) & static_cast<long>(other.number)));
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError("&", getType(), other.getType()));
    }

    Value operator|(const Value& other) const {
        if (isNumber() && other.isNumber()) {
            return Value(static_cast<double>(static_cast<long>(number) | static_cast<long>(other.number)));
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError("|", getType(), other.getType()));
    }

    Value operator^(const Value& other) const {
        if (isNumber() && other.isNumber()) {
            return Value(static_cast<double>(static_cast<long>(number) ^ static_cast<long>(other.number)));
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError("^", getType(), other.getType()));
    }

    Value operator<<(const Value& other) const {
        if (isNumber() && other.isNumber()) {
            return Value(static_cast<double>(static_cast<long>(number) << static_cast<long>(other.number)));
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError("<<", getType(), other.getType()));
    }

    Value operator>>(const Value& other) const {
        if (isNumber() && other.isNumber()) {
            return Value(static_cast<double>(static_cast<long>(number) >> static_cast<long>(other.number)));
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