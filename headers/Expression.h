//
// Created by Bobby Lucero on 5/21/23.
//

#pragma once
#include <iostream>
#include <memory>
#include "Lexer.h"
#include "helperFunctions/ShortHands.h"
#include "TypeWrapper.h"
#include "Value.h"

struct AssignExpr;
struct BinaryExpr;
struct GroupingExpr;
struct LiteralExpr;
struct UnaryExpr;
struct VarExpr;
struct CallExpr;

// AST nodes use shared_ptr for proper memory management
struct ExprVisitor
{
    virtual Value visitAssignExpr(const std::shared_ptr<AssignExpr>& expr) = 0;
    virtual Value visitBinaryExpr(const std::shared_ptr<BinaryExpr>& expr) = 0;
    virtual Value visitGroupingExpr(const std::shared_ptr<GroupingExpr>& expr) = 0;
    virtual Value visitLiteralExpr(const std::shared_ptr<LiteralExpr>& expr) = 0;
    virtual Value visitUnaryExpr(const std::shared_ptr<UnaryExpr>& expr) = 0;
    virtual Value visitVariableExpr(const std::shared_ptr<VarExpr>& expr) = 0;
    virtual Value visitCallExpr(const std::shared_ptr<CallExpr>& expr) = 0;
};


struct Expr : public std::enable_shared_from_this<Expr> {
    virtual Value accept(ExprVisitor* visitor) = 0;
    virtual ~Expr() = default;
};

struct AssignExpr : Expr
{
    const Token name;
    std::shared_ptr<Expr> value;
    AssignExpr(Token name, std::shared_ptr<Expr> value) : name(name), value(value)
    {
    }

    Value accept(ExprVisitor* visitor) override
    {
        return visitor->visitAssignExpr(std::static_pointer_cast<AssignExpr>(shared_from_this()));
    }
};

struct BinaryExpr : Expr
{
    std::shared_ptr<Expr> left;
    const Token oper;
    std::shared_ptr<Expr> right;

    BinaryExpr(std::shared_ptr<Expr> left, Token oper, std::shared_ptr<Expr> right) 
        : left(left), oper(oper), right(right)
    {
    }
    Value accept(ExprVisitor* visitor) override{
        return visitor->visitBinaryExpr(std::static_pointer_cast<BinaryExpr>(shared_from_this()));
    }
};

struct GroupingExpr : Expr
{
    std::shared_ptr<Expr> expression;

    explicit GroupingExpr(std::shared_ptr<Expr> expression) : expression(expression)
    {
    }
    Value accept(ExprVisitor* visitor) override{
        return visitor->visitGroupingExpr(std::static_pointer_cast<GroupingExpr>(shared_from_this()));
    }
};

struct LiteralExpr : Expr
{
    std::string value;
    bool isNumber;
    bool isNull;
    bool isBoolean;

    LiteralExpr(const std::string& value, bool isNumber, bool isNull, bool isBoolean)
        : value(value), isNumber(isNumber), isNull(isNull), isBoolean(isBoolean) {}
    Value accept(ExprVisitor* visitor) override{
        return visitor->visitLiteralExpr(std::static_pointer_cast<LiteralExpr>(shared_from_this()));
    }
};

struct UnaryExpr : Expr
{
    const Token oper;
    std::shared_ptr<Expr> right;

    UnaryExpr(Token oper, std::shared_ptr<Expr> right) : oper(oper), right(right)
    {
    }
    Value accept(ExprVisitor* visitor) override{
        return visitor->visitUnaryExpr(std::static_pointer_cast<UnaryExpr>(shared_from_this()));
    }
};

struct VarExpr : Expr
{
    const Token name;
    explicit VarExpr(Token name) : name(name){};
    Value accept(ExprVisitor* visitor) override
    {
        return visitor->visitVariableExpr(std::static_pointer_cast<VarExpr>(shared_from_this()));
    }
};

struct CallExpr : Expr
{
    std::shared_ptr<Expr> callee;
    const Token paren;
    std::vector<std::shared_ptr<Expr>> arguments;

    CallExpr(std::shared_ptr<Expr> callee, Token paren, std::vector<std::shared_ptr<Expr>> arguments)
        : callee(callee), paren(paren), arguments(arguments)
    {
    }

    Value accept(ExprVisitor* visitor) override
    {
        return visitor->visitCallExpr(std::static_pointer_cast<CallExpr>(shared_from_this()));
    }
};

////

