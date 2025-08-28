#include "Environment.h"
#include "ErrorReporter.h"

void Environment::assign(const Token& name, const Value& value) {
    // Disallow reassignment of module bindings (immutability of module variable)
    auto itv = variables.find(name.lexeme);
    if (itv != variables.end() && itv->second.isModule()) {
        if (errorReporter) {
            errorReporter->reportError(name.line, name.column, "Import Error",
                "Cannot reassign module binding '" + name.lexeme + "'", "");
        }
        throw std::runtime_error("Cannot reassign module binding '" + name.lexeme + "'");
    }
    auto it = variables.find(name.lexeme);
    if (it != variables.end()) {
        it->second = value;
        return;
    }
    
    if (parent != nullptr) {
        parent->assign(name, value);
        return;
    }
    
    // Report only if not within a try; otherwise let try/catch handle
    if (errorReporter) {
        // We cannot check tryDepth here directly; rely on Executor to suppress double-reporting
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

 

void Environment::pruneForClosureCapture(const std::unordered_set<std::string>& usedVariables) {
    for (auto &entry : variables) {
        const std::string& varName = entry.first;
        Value &v = entry.second;
        
        // Only prune heavy containers that aren't used by the function
        // If usedVariables is empty, don't prune anything (be conservative)
        if (!usedVariables.empty() && !usedVariables.count(varName)) {
            if (v.isArray()) {
                entry.second = Value(std::vector<Value>{});
            } else if (v.isDict()) {
                entry.second = Value(std::unordered_map<std::string, Value>{});
            }
        }
    }
    if (parent) {
        parent->pruneForClosureCapture(usedVariables);
    }
}