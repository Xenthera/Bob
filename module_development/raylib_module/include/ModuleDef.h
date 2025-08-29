// ModuleDef.h
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "Value.h"

// Module is already defined in Value.h, so we don't redefine it here

// Interface for C++ modules
class ModuleDef {
public:
    virtual ~ModuleDef() = default;
    
    // Get module name
    virtual std::string getName() const = 0;
    
    // Register functions and constants with the module (like builtin modules)
    virtual void registerModule(class Interpreter& interpreter) = 0;
};

// Simple implementation for testing
class SimpleModuleDef : public ModuleDef {
private:
    std::string name;
    
public:
    SimpleModuleDef(const std::string& moduleName) : name(moduleName) {}
    
    std::string getName() const override {
        return name;
    }
    
    void registerModule(class Interpreter& interpreter) override {
        // This will be implemented by the actual module
    }
};


