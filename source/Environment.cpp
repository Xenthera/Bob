//
// Created by Bobby Lucero on 5/30/23.
//

#include "../headers/Environment.h"
#include "../headers/Lexer.h"


sptr(Object) Environment::get(Token name)
{
    if(variables.count(name.lexeme))
    {
        return variables[name.lexeme];
    }

    if(enclosing != nullptr)
    {
        return enclosing->get(name);
    }

    throw std::runtime_error("Undefined variable '" + name.lexeme + "'.");
}

void Environment::define(std::string name, sptr(Object) value) {
    // Allow redefinition - just overwrite the existing value
    variables[name] = value;
}

void Environment::assign(Token name, std::shared_ptr<Object> value) {
    if(variables.count(name.lexeme) > 0)
    {
        variables[name.lexeme] = value;
        return;
    }

    if(enclosing != nullptr)
    {
        enclosing->assign(name, value);
        return;
    }

    throw std::runtime_error("Undefined variable '" + name.lexeme + "'.");
}
