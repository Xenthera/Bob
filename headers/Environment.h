#pragma once

#include <unordered_map>
#include "TypeWrapper.h"
#include "helperFunctions/ShortHands.h"
#include "Lexer.h"

class Environment
{
public:

    void define(std::string name, sptr(Object) value);

    void assign(Token name, sptr(Object) value);

    std::shared_ptr<Object> get(Token name);

private:
    std::unordered_map<std::string, sptr(Object)> variables;

};