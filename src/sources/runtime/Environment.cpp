#include "Environment.h"
#include "ErrorReporter.h"

void Environment::assign(const Token& name, const Value& value) {
    auto it = variables.find(name.lexeme);
    if (it != variables.end()) {
        it->second = value;
        return;
    }
    
    if (parent != nullptr) {
        parent->assign(name, value);
        return;
    }
    
    if (errorReporter) {
        errorReporter->reportError(name.line, name.column, "Runtime Error", 
            "Undefined variable '" + name.lexeme + "'", "");
    }
    throw std::runtime_error("Undefined variable '" + name.lexeme + "'");
}

Value Environment::get(const Token& name) {
    auto it = variables.find(name.lexeme);
    if (it != variables.end()) {
        return it->second;
    }
    
    if (parent != nullptr) {
        return parent->get(name);
    }
    
    if (errorReporter) {
        errorReporter->reportError(name.line, name.column, "Runtime Error", 
            "Undefined variable '" + name.lexeme + "'", "");
    }
    throw std::runtime_error("Undefined variable '" + name.lexeme + "'");
}

Value Environment::get(const std::string& name) {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return it->second;
    }
    
    if (parent != nullptr) {
        return parent->get(name);
    }
    
    throw std::runtime_error("Undefined variable '" + name + "'");
} 

void Environment::pruneForClosureCapture() {
    for (auto &entry : variables) {
        Value &v = entry.second;
        if (v.isArray()) {
            // Replace with a new empty array to avoid mutating original shared storage
            entry.second = Value(std::vector<Value>{});
        } else if (v.isDict()) {
            // Replace with a new empty dict to avoid mutating original shared storage
            entry.second = Value(std::unordered_map<std::string, Value>{});
        }
    }
    if (parent) {
        parent->pruneForClosureCapture();
    }
}