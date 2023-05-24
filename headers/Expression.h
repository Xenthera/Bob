//
// Created by Bobby Lucero on 5/21/23.
//

#pragma once
#include "Lexer.h"
#include <iostream>

template <typename T>
struct Visitor;

template <typename T>
struct Expr{
    virtual T accept(Visitor<T>* visitor) = 0;
    virtual ~Expr(){}
};

template <typename T>
struct BinaryExpr : Expr<T>
{
    const std::shared_ptr<Expr<T> > left;
    const Token oper;
    const std::shared_ptr<Expr<T> > right;

    BinaryExpr(std::shared_ptr<Expr<T> > left, Token oper, std::shared_ptr<Expr<T> > right) : left(left), oper(oper), right(right)
    {
    }
    T accept(Visitor<T>* visitor) override{
        return visitor->visitBinaryExpr(this);
    }
};
template <typename T>
struct GroupingExpr : Expr<T>
{
    const std::shared_ptr<Expr<T> > expression;

    GroupingExpr(std::shared_ptr<Expr<T> > expression) : expression(expression)
    {
    }
    T accept(Visitor<T>* visitor) override{
        return visitor->visitGroupingExpr(this);
    }
};
template <typename T>
struct LiteralExpr : Expr<T>
{
    const std::string value;
    const bool isNumber;
    LiteralExpr(std::string value, bool isNumber) : value(value), isNumber(isNumber)
    {
    }
    T accept(Visitor<T>* visitor) override{
        return visitor->visitLiteralExpr(this);
    }
};
template <typename T>
struct UnaryExpr : Expr<T>
{
    const Token oper;
    const std::shared_ptr<Expr<T> > right;

    UnaryExpr(Token oper, std::shared_ptr<Expr<T> > right) : oper(oper), right(right)
    {
    }
    T accept(Visitor<T>* visitor) override{
        return visitor->visitUnaryExpr(this);
    }
};

////
template <typename T>
struct Visitor
{
    virtual T visitBinaryExpr(BinaryExpr<T>* expression) = 0;
    virtual T visitGroupingExpr(GroupingExpr<T>* expression) = 0;
    virtual T visitLiteralExpr(LiteralExpr<T>* expression) = 0;
    virtual T visitUnaryExpr(UnaryExpr<T>* expression) = 0;
};
