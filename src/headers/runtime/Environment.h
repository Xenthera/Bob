#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "Value.h"
#include "Lexer.h"

// Forward declaration
class ErrorReporter;

struct Environment {
public:
    Environment() : parent(nullptr), errorReporter(nullptr) {}
    Environment(std::shared_ptr<Environment> parent_env) : parent(parent_env), errorReporter(nullptr) {}
    
    // Copy constructor for closure snapshots - creates a deep copy of the environment chain
    Environment(const Environment& other) : parent(nullptr), errorReporter(other.errorReporter) {
        // Copy all variables normally - arrays will be handled by forceCleanup
        variables = other.variables;
        
        // Create a deep copy of the parent environment chain
        if (other.parent) {
            parent = std::make_shared<Environment>(*other.parent);
        }
    }
    
    // Set error reporter for enhanced error reporting
    void setErrorReporter(ErrorReporter* reporter) {
        errorReporter = reporter;
    }
    
    // Optimized define with inline
    inline void define(const std::string& name, const Value& value) {
        variables[name] = value;
    }
    
    // Enhanced assign with error reporting
    void assign(const Token& name, const Value& value);
    
    // Enhanced get with error reporting
    Value get(const Token& name);
    
    // Get by string name with error reporting
    Value get(const std::string& name);
    
    std::shared_ptr<Environment> getParent() const { return parent; }
    inline void clear() { variables.clear(); }
    
    // Set parent environment for TCO environment reuse
    inline void setParent(std::shared_ptr<Environment> newParent) {
        parent = newParent;
    }

private:
    std::unordered_map<std::string, Value> variables;
    std::shared_ptr<Environment> parent;
    ErrorReporter* errorReporter;
};

