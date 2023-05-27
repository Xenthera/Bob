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
    virtual sptr(Object) visitBinaryExpr(sptr(BinaryExpr) expr) = 0;
    virtual sptr(Object) visitGroupingExpr(sptr(GroupingExpr) expr) = 0;
    virtual sptr(Object) visitLiteralExpr(sptr(LiteralExpr) expr) = 0;
    virtual sptr(Object) visitUnaryExpr(sptr(UnaryExpr) expr) = 0;
};


struct Expr{
    virtual sptr(Object) accept(Visitor* visitor) = 0;
    virtual ~Expr(){}
};


struct BinaryExpr : Expr, public std::enable_shared_from_this<BinaryExpr>
{
    const std::shared_ptr<Expr> left;
    const Token oper;
    const std::shared_ptr<Expr> right;

    BinaryExpr(sptr(Expr) left, Token oper, sptr(Expr) right) : left(left), oper(oper), right(right)
    {
    }
    sptr(Object) accept(Visitor* visitor) override{
        return visitor->visitBinaryExpr(shared_from_this() );
    }
};

struct GroupingExpr : Expr, public std::enable_shared_from_this<GroupingExpr>
{
    const std::shared_ptr<Expr> expression;

    GroupingExpr(sptr(Expr) expression) : expression(expression)
    {
    }
    sptr(Object) accept(Visitor* visitor) override{
        return visitor->visitGroupingExpr(shared_from_this());
    }
};

struct LiteralExpr : Expr, public std::enable_shared_from_this<LiteralExpr>
{
    const std::string value;
    const bool isNumber;
    const bool isNull;
    LiteralExpr(std::string value, bool isNumber, bool isNull) : value(value), isNumber(isNumber), isNull(isNull)
    {
    }
    sptr(Object) accept(Visitor* visitor) override{
        return visitor->visitLiteralExpr(shared_from_this());
    }
};

struct UnaryExpr : Expr, public std::enable_shared_from_this<UnaryExpr>
{
    const Token oper;
    const std::shared_ptr<Expr> right;

    UnaryExpr(Token oper, sptr(Expr) right) : oper(oper), right(right)
    {
    }
    sptr(Object) accept(Visitor* visitor) override{
        return visitor->visitUnaryExpr(shared_from_this());
    }
};

////

