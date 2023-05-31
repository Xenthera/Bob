#pragma once
#include <string>
#include <iostream>
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
        std::cout << value.size() << std::endl;
        std::cout << "String being destroyed..." << std::endl;
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