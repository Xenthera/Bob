#pragma once
#include <vector>
#include "Lexer.h"
#include "Expression.h"
#include "TypeWrapper.h"

class Parser
{
private:
    const std::vector<Token> tokens;
    int current = 0;

public:
    Parser(std::vector<Token> tokens) : tokens(tokens){};

private:
    std::shared_ptr<Expr<Object>> expression();
};