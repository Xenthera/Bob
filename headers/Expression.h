//
// Created by Bobby Lucero on 5/21/23.
//

#pragma once
#include "Lexer.h"
#include <iostream>
struct Expr{
    virtual ~Expr()
    {

    }
};

struct BinaryExpr : Expr
{
    const Expr left;
    const Token oper;
    const Expr right;

    BinaryExpr(Expr left, Token oper, Expr right) : left(left), oper(oper), right(right)
    {
    }
};
struct GroupingExpr : Expr
{
    const Expr expression;

    GroupingExpr(Expr expression) : expression(expression)
    {
    }
};
struct LiteralExpr : Expr
{
    const std::string value;

    LiteralExpr(std::string value) : value(value)
    {
    }
};
struct UnaryExpr : Expr
{
    const Token oper;
    const Expr right;

    UnaryExpr(Token oper, Expr right) : oper(oper), right(right)
    {
    }
};
