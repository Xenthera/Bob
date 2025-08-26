#pragma once
#include "OptimizedValue.h"
#include <unordered_map>
#include <vector>
#include <string>

// Optimized environment with fast variable access
class OptimizedEnvironment {
public:
    // Fast variable lookup using direct indexing
    struct Variable {
        OptimizedValue value;
        bool isConst;
        
        Variable(const OptimizedValue& val, bool constant = false) 
            : value(val), isConst(constant) {}
    };
    
    // Define a variable (fast)
    void define(const std::string& name, const OptimizedValue& value, bool isConst = false) {
        variables[name] = Variable(value, isConst);
    }
    
    // Get a variable (fast)
    OptimizedValue get(const std::string& name) const {
        auto it = variables.find(name);
        if (it != variables.end()) {
            return it->second.value;
        }
        throw std::runtime_error("Undefined variable: " + name);
    }
    
    // Assign to a variable (fast)
    void assign(const std::string& name, const OptimizedValue& value) {
        auto it = variables.find(name);
        if (it != variables.end()) {
            if (it->second.isConst) {
                throw std::runtime_error("Cannot assign to const variable: " + name);
            }
            it->second.value = value;
        } else {
            throw std::runtime_error("Undefined variable: " + name);
        }
    }
    
private:
    std::unordered_map<std::string, Variable> variables;
};
