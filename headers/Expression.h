//
// Created by Bobby Lucero on 5/21/23.
//

#pragma once
#include <iostream>
#include "Lexer.h"
#include "helperFunctions/ShortHands.h"
#include "TypeWrapper.h"

struct BinaryExpr;
struct GroupingExpr;
struct LiteralExpr;
struct UnaryExpr;

struct Visitor
{
    virtual sptr(Object) visitBinaryExpr(BinaryExpr* expression) = 0;
    virtual sptr(Object) visitGroupingExpr(GroupingExpr* expression) = 0;
    virtual sptr(Object) visitLiteralExpr(LiteralExpr* expression) = 0;
    virtual sptr(Object) visitUnaryExpr(UnaryExpr* expression) = 0;
};


struct Expr{
    virtual sptr(Object) accept(Visitor* visitor) = 0;
    virtual ~Expr(){}
};


struct BinaryExpr : Expr
{
    const std::shared_ptr<Expr> left;
    const Token oper;
    const std::shared_ptr<Expr> right;

    BinaryExpr(sptr(Expr) left, Token oper, sptr(Expr) right) : left(left), oper(oper), right(right)
    {
    }
    sptr(Object) accept(Visitor* visitor) override{
        return visitor->visitBinaryExpr(this);
    }
};

struct GroupingExpr : Expr
{
    const std::shared_ptr<Expr> expression;

    GroupingExpr(sptr(Expr) expression) : expression(expression)
    {
    }
    sptr(Object) accept(Visitor* visitor) override{
        return visitor->visitGroupingExpr(this);
    }
};

struct LiteralExpr : Expr
{
    const std::string value;
    const bool isNumber;
    LiteralExpr(std::string value, bool isNumber) : value(value), isNumber(isNumber)
    {
    }
    sptr(Object) accept(Visitor* visitor) override{
        return visitor->visitLiteralExpr(this);
    }
};

struct UnaryExpr : Expr
{
    const Token oper;
    const std::shared_ptr<Expr> right;

    UnaryExpr(Token oper, sptr(Expr) right) : oper(oper), right(right)
    {
    }
    sptr(Object) accept(Visitor* visitor) override{
        return visitor->visitUnaryExpr(this);
    }
};

////

