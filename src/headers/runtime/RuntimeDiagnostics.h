#pragma once

#include "Value.h"
#include <string>
#include <memory>

// Forward declarations from Value.h
struct Function;
struct BuiltinFunction;
struct Thunk;

/**
 * RuntimeDiagnostics - Utility functions for runtime operations
 * 
 * This class handles value conversion, equality checking, string representation,
 * and other diagnostic utilities that don't belong in core evaluation logic.
 */
class RuntimeDiagnostics {
public:
    RuntimeDiagnostics() = default;
    
    // Value utility functions
    bool isTruthy(Value object);
    bool isEqual(Value a, Value b);
    std::string stringify(Value object);
    
    // Memory management utilities
    void cleanupUnusedFunctions(std::vector<std::shared_ptr<BuiltinFunction>>& functions);
    void cleanupUnusedThunks(std::vector<std::shared_ptr<Thunk>>& thunks);
    void forceCleanup(std::vector<std::shared_ptr<BuiltinFunction>>& functions, 
                     std::vector<std::shared_ptr<Thunk>>& thunks);
    
private:
    // Helper methods for stringify
    std::string formatNumber(double value);
    std::string formatArray(const std::vector<Value>& arr);
    std::string formatDict(const std::unordered_map<std::string, Value>& dict);
};