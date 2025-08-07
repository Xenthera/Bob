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

struct Object
{
    virtual ~Object(){};
};

struct Number : Object
{
    double value;
    explicit Number(double value) : value(value) {}
};

struct String : Object
{
    std::string value;
    explicit String(std::string str) : value(str) {}
    ~String(){

    }
};

struct Boolean : Object
{
    bool value;
    explicit Boolean(bool value) : value(value) {}
};

struct None : public Object
{

};

struct Function : public Object
{
    const std::string name;
    const std::vector<std::string> params;
    const std::vector<std::shared_ptr<Stmt>> body;
    const std::shared_ptr<Environment> closure;

    Function(std::string name, std::vector<std::string> params, 
             std::vector<std::shared_ptr<Stmt>> body, 
             std::shared_ptr<Environment> closure)
        : name(name), params(params), body(body), closure(closure) {}
};

struct BuiltinFunction : public Object
{
    const std::string name;
    const std::function<Value(std::vector<Value>, int, int)> func;
    
    BuiltinFunction(std::string name, std::function<Value(std::vector<Value>, int, int)> func)
        : name(name), func(func) {}
};

