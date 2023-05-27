#pragma once
#include <string>
struct Object
{
    static int count;
    Object();
    virtual ~Object();
};

struct Number : public Object
{
    double value;
    explicit Number(double value) : value(value) {}
};

struct String : public Object
{
    std::string value;
    explicit String(std::string str) : value(str) {}
};