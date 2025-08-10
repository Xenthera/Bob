#include "RuntimeDiagnostics.h"
#include "Value.h"
#include "TypeWrapper.h"  // For Function and BuiltinFunction definitions
#include <sstream>
#if defined(__linux__)
#include <malloc.h>
#elif defined(__APPLE__)
#include <malloc/malloc.h>
#endif
#include <iomanip>
#include <limits>
#include <cmath>
#include <algorithm>


bool RuntimeDiagnostics::isTruthy(Value object) { return object.isTruthy(); }

bool RuntimeDiagnostics::isEqual(Value a, Value b) {
    if (a.isNumber() && b.isBoolean()) {
        return b.asBoolean() ? (a.asNumber() != 0.0) : (a.asNumber() == 0.0);
    }
    if (a.isBoolean() && b.isNumber()) {
        return a.asBoolean() ? (b.asNumber() != 0.0) : (b.asNumber() == 0.0);
    }
    return a.equals(b);
}

std::string RuntimeDiagnostics::stringify(Value object) { return object.toString(); }

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

std::string RuntimeDiagnostics::formatArray(const std::vector<Value>& arr) { return Value(arr).toString(); }

std::string RuntimeDiagnostics::formatDict(const std::unordered_map<std::string, Value>& dict) { return Value(dict).toString(); }

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

void RuntimeDiagnostics::cleanupUnusedFunctions(std::vector<std::shared_ptr<Function>>& functions) {
    // Only remove functions that are definitely not referenced anywhere (use_count == 1)
    // This is more conservative to prevent dangling pointer issues
    functions.erase(
        std::remove_if(functions.begin(), functions.end(),
            [](const std::shared_ptr<Function>& func) {
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

 

void RuntimeDiagnostics::forceCleanup(std::vector<std::shared_ptr<BuiltinFunction>>& builtinFunctions,
                                     std::vector<std::shared_ptr<Function>>& functions,
                                     std::vector<std::shared_ptr<Thunk>>& thunks) {
    try {
        // Remove functions only when they are exclusively held by the interpreter vector
        functions.erase(
            std::remove_if(functions.begin(), functions.end(),
                [](const std::shared_ptr<Function>& func) {
                    return func.use_count() == 1;
                }),
            functions.end()
        );
        
        // Also cleanup builtin functions and thunks
        builtinFunctions.erase(
            std::remove_if(builtinFunctions.begin(), builtinFunctions.end(),
                [](const std::shared_ptr<BuiltinFunction>& func) {
                    return func.use_count() <= 1; // Only referenced by Interpreter
                }),
            builtinFunctions.end()
        );
        
        thunks.erase(
            std::remove_if(thunks.begin(), thunks.end(),
                [](const std::shared_ptr<Thunk>& thunk) {
                    return thunk.use_count() <= 1; // Only referenced by Interpreter
                }),
            thunks.end()
        );
    } catch (const std::exception& e) {
        std::cerr << "Exception in forceCleanup: " << e.what() << std::endl;
        throw; // Re-throw to let the caller handle it
    }
}