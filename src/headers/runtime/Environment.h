#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <unordered_set>
#include "Value.h"
#include "Lexer.h"

// Forward declaration
class ErrorReporter;

struct Environment {
public:
    Environment() : parent(nullptr), errorReporter(nullptr) {}
    Environment(std::shared_ptr<Environment> parent_env) : parent(parent_env), errorReporter(nullptr) {}
    
    // Copy constructor for closure snapshots - preserves shared reference to parent chain
    Environment(const Environment& other) : parent(other.parent), errorReporter(other.errorReporter) {
        // Copy all variables normally - arrays will be handled by forceCleanup
        variables = other.variables;
        
        // Preserve shared reference to parent environment chain instead of deep copying
        // This ensures closures maintain access to the global environment with built-in functions
    }
    
    // Set error reporter for enhanced error reporting
    void setErrorReporter(ErrorReporter* reporter) {
        errorReporter = reporter;
    }
    ErrorReporter* getErrorReporter() const { return errorReporter; }
    
    // Optimized define with inline
    inline void define(const std::string& name, const Value& value) {
        variables[name] = value;
    }
    
    // Enhanced assign with error reporting
    void assign(const Token& name, const Value& value);
    
    // Enhanced get with error reporting
    Value get(const Token& name);
    
    // Prune heavy containers in a snapshot to avoid capture cycles
    void pruneForClosureCapture(const std::unordered_set<std::string>& usedVariables);
    
    std::shared_ptr<Environment> getParent() const { return parent; }
    // Export all variables (shallow copy) for module namespace
    std::unordered_map<std::string, Value> getAll() const { return variables; }
    

private:
    std::unordered_map<std::string, Value> variables;
    std::shared_ptr<Environment> parent;
    ErrorReporter* errorReporter;
};

