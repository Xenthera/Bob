#include "RuntimeDiagnostics.h"
#include "Value.h"
#include "TypeWrapper.h"  // For Function and BuiltinFunction definitions
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <algorithm>

bool RuntimeDiagnostics::isTruthy(Value object) {
    if(object.isBoolean()) {
        return object.asBoolean();
    }

    if(object.isNone()) {
        return false;
    }

    if(object.isNumber()) {
        return object.asNumber() != 0;
    }

    if(object.isString()) {
        return object.asString().length() > 0;
    }

    return true;
}

bool RuntimeDiagnostics::isEqual(Value a, Value b) {
    // Handle none comparisons first
    if (a.isNone() || b.isNone()) {
        return a.isNone() && b.isNone();
    }
    
    // Handle same type comparisons
    if (a.isNumber() && b.isNumber()) {
        return a.asNumber() == b.asNumber();
    }
    
    if (a.isBoolean() && b.isBoolean()) {
        return a.asBoolean() == b.asBoolean();
    }
    
    if (a.isString() && b.isString()) {
        return a.asString() == b.asString();
    }
    
    if (a.isArray() && b.isArray()) {
        const std::vector<Value>& arrA = a.asArray();
        const std::vector<Value>& arrB = b.asArray();
        
        if (arrA.size() != arrB.size()) {
            return false;
        }
        
        for (size_t i = 0; i < arrA.size(); i++) {
            if (!isEqual(arrA[i], arrB[i])) {
                return false;
            }
        }
        return true;
    }
    
    if (a.isFunction() && b.isFunction()) {
        // Functions are equal only if they are the same object
        return a.asFunction() == b.asFunction();
    }
    
    if (a.isBuiltinFunction() && b.isBuiltinFunction()) {
        // Builtin functions are equal only if they are the same object
        return a.asBuiltinFunction() == b.asBuiltinFunction();
    }
    
    // Cross-type comparisons that make sense
    if (a.isNumber() && b.isBoolean()) {
        // Numbers and booleans: 0 and false are equal, non-zero and true are equal
        if (b.asBoolean()) {
            return a.asNumber() != 0.0;
        } else {
            return a.asNumber() == 0.0;
        }
    }
    
    if (a.isBoolean() && b.isNumber()) {
        // Same as above, but reversed
        if (a.asBoolean()) {
            return b.asNumber() != 0.0;
        } else {
            return b.asNumber() == 0.0;
        }
    }
    
    // For all other type combinations, return false
    return false;
}

std::string RuntimeDiagnostics::stringify(Value object) {
    if(object.isNone()) {
        return "none";
    }
    else if(object.isNumber()) {
        return formatNumber(object.asNumber());
    }
    else if(object.isString()) {
        return object.asString();
    }
    else if(object.isBoolean()) {
        return object.asBoolean() == 1 ? "true" : "false";
    }
    else if(object.isFunction()) {
        return "<function " + object.asFunction()->name + ">";
    }
    else if(object.isBuiltinFunction()) {
        return "<builtin_function " + object.asBuiltinFunction()->name + ">";
    }
    else if(object.isArray()) {
        return formatArray(object.asArray());
    }
    else if(object.isDict()) {
        return formatDict(object.asDict());
    }

    throw std::runtime_error("Could not convert object to string");
}

std::string RuntimeDiagnostics::formatNumber(double value) {
    double integral = value;
    double fractional = std::modf(value, &integral);

    std::stringstream ss;
    if(std::abs(fractional) < std::numeric_limits<double>::epsilon()) {
        ss << std::fixed << std::setprecision(0) << integral;
        return ss.str();
    }
    else {
        ss << std::fixed << std::setprecision(std::numeric_limits<double>::digits10 - 1) << value;
        std::string str = ss.str();
        str.erase(str.find_last_not_of('0') + 1, std::string::npos);
        if (str.back() == '.') {
            str.pop_back();
        }
        return str;
    }
}

std::string RuntimeDiagnostics::formatArray(const std::vector<Value>& arr) {
    std::string result = "[";
    
    for (size_t i = 0; i < arr.size(); i++) {
        if (i > 0) result += ", ";
        result += stringify(arr[i]);
    }
    
    result += "]";
    return result;
}

std::string RuntimeDiagnostics::formatDict(const std::unordered_map<std::string, Value>& dict) {
    std::string result = "{";
    
    bool first = true;
    for (const auto& pair : dict) {
        if (!first) result += ", ";
        result += "\"" + pair.first + "\": " + stringify(pair.second);
        first = false;
    }
    
    result += "}";
    return result;
}

void RuntimeDiagnostics::cleanupUnusedFunctions(std::vector<std::shared_ptr<BuiltinFunction>>& functions) {
    // Only remove functions that are definitely not referenced anywhere (use_count == 1)
    // This is more conservative to prevent dangling pointer issues
    functions.erase(
        std::remove_if(functions.begin(), functions.end(),
            [](const std::shared_ptr<BuiltinFunction>& func) {
                return func.use_count() == 1;  // Only referenced by this vector, nowhere else
            }),
        functions.end()
    );
}

void RuntimeDiagnostics::cleanupUnusedThunks(std::vector<std::shared_ptr<Thunk>>& thunks) {
    // Only remove thunks that are definitely not referenced anywhere (use_count == 1)
    // This is more conservative to prevent dangling pointer issues
    thunks.erase(
        std::remove_if(thunks.begin(), thunks.end(),
            [](const std::shared_ptr<Thunk>& thunk) {
                return thunk.use_count() == 1;  // Only referenced by this vector, nowhere else
            }),
        thunks.end()
    );
}

void RuntimeDiagnostics::forceCleanup(std::vector<std::shared_ptr<BuiltinFunction>>& functions, 
                                     std::vector<std::shared_ptr<Thunk>>& thunks) {
    // More aggressive cleanup when breaking array references
    functions.erase(
        std::remove_if(functions.begin(), functions.end(),
            [](const std::shared_ptr<BuiltinFunction>& func) {
                return func.use_count() <= 2; // More aggressive than == 1
            }),
        functions.end()
    );
    
    thunks.erase(
        std::remove_if(thunks.begin(), thunks.end(),
            [](const std::shared_ptr<Thunk>& thunk) {
                return thunk.use_count() <= 2; // More aggressive than == 1
            }),
        thunks.end()
    );
}