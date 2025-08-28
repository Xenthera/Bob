#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include "Value.h"

// Forward declarations
struct Stmt;
struct Environment;

struct Function
{
    const std::string name;
    const std::vector<std::string> params;
    const std::vector<std::shared_ptr<Stmt>> body;
    const std::shared_ptr<Environment> closure;
    const std::string ownerClass; // empty for non-methods
    const std::string sourceModule; // empty for non-module functions

    Function(std::string name, std::vector<std::string> params, 
             std::vector<std::shared_ptr<Stmt>> body, 
             std::shared_ptr<Environment> closure,
             std::string ownerClass = "", std::string sourceModule = "")
        : name(name), params(params), body(body), closure(closure), ownerClass(ownerClass), sourceModule(sourceModule) {}
};

struct BuiltinFunction
{
    const std::string name;
    const std::function<Value(std::vector<Value>, int, int)> func;
    
    BuiltinFunction(std::string name, std::function<Value(std::vector<Value>, int, int)> func)
        : name(name), func(func) {}
};

