#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "Value.h"
#include "Lexer.h"

class Environment {
public:
    Environment() : parent(nullptr) {}
    Environment(std::shared_ptr<Environment> parent_env) : parent(parent_env) {}
    
    // Optimized define with inline
    inline void define(const std::string& name, const Value& value) {
        variables[name] = value;
    }
    
    // Optimized assign with inline
    inline void assign(const Token& name, const Value& value) {
        auto it = variables.find(name.lexeme);
        if (it != variables.end()) {
            it->second = value;
        } else if (parent != nullptr) {
            parent->assign(name, value);
        } else {
            throw std::runtime_error("Undefined variable '" + name.lexeme + "'.");
        }
    }
    
    // Optimized get with inline and move semantics
    inline Value get(const Token& name) {
        auto it = variables.find(name.lexeme);
        if (it != variables.end()) {
            return it->second; // Return by value (will use move if possible)
        }
        if (parent != nullptr) {
            return parent->get(name);
        }
        throw std::runtime_error("Undefined variable '" + name.lexeme + "'.");
    }
    
    std::shared_ptr<Environment> getParent() const { return parent; }
    inline void clear() { variables.clear(); }

private:
    std::unordered_map<std::string, Value> variables;
    std::shared_ptr<Environment> parent;
};