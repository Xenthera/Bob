#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "Value.h"
#include "Lexer.h"

// Forward declaration
class ErrorReporter;

class Environment {
public:
    Environment() : parent(nullptr), errorReporter(nullptr) {}
    Environment(std::shared_ptr<Environment> parent_env) : parent(parent_env), errorReporter(nullptr) {}
    
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

private:
    std::unordered_map<std::string, Value> variables;
    std::shared_ptr<Environment> parent;
    ErrorReporter* errorReporter;
};