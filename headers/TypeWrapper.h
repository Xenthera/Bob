#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <functional>
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
    const std::vector<std::shared_ptr<void>> body;  // Will cast to Stmt* when needed
    const std::shared_ptr<void> closure;  // Will cast to Environment* when needed

    Function(std::string name, std::vector<std::string> params, 
             std::vector<std::shared_ptr<void>> body, 
             std::shared_ptr<void> closure)
        : name(name), params(params), body(body), closure(closure) {}
};

struct BuiltinFunction : public Object
{
    const std::string name;
    const std::function<std::shared_ptr<Object>(std::vector<std::shared_ptr<Object> >)> func;
    
    BuiltinFunction(std::string name, std::function<std::shared_ptr<Object>(std::vector<std::shared_ptr<Object> >)> func)
        : name(name), func(func) {}
};

