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

// Forward declarations
struct FunctionExpr;
struct IncrementExpr;
struct TernaryExpr;
struct ArrayLiteralExpr;
struct ArrayIndexExpr;
struct ArrayAssignExpr;
struct DictLiteralExpr;
struct DictIndexExpr;
struct DictAssignExpr;
struct ExprVisitor;

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
    virtual Value visitCallExpr(const std::shared_ptr<CallExpr>& expr) = 0;
    virtual Value visitFunctionExpr(const std::shared_ptr<FunctionExpr>& expr) = 0;
    virtual Value visitGroupingExpr(const std::shared_ptr<GroupingExpr>& expr) = 0;
    virtual Value visitIncrementExpr(const std::shared_ptr<IncrementExpr>& expr) = 0;
    virtual Value visitLiteralExpr(const std::shared_ptr<LiteralExpr>& expr) = 0;
    virtual Value visitUnaryExpr(const std::shared_ptr<UnaryExpr>& expr) = 0;
    virtual Value visitVarExpr(const std::shared_ptr<VarExpr>& expr) = 0;
    virtual Value visitTernaryExpr(const std::shared_ptr<TernaryExpr>& expr) = 0;
    virtual Value visitArrayLiteralExpr(const std::shared_ptr<ArrayLiteralExpr>& expr) = 0;
    virtual Value visitArrayIndexExpr(const std::shared_ptr<ArrayIndexExpr>& expr) = 0;
    virtual Value visitArrayAssignExpr(const std::shared_ptr<ArrayAssignExpr>& expr) = 0;
    virtual Value visitDictLiteralExpr(const std::shared_ptr<DictLiteralExpr>& expr) = 0;

};

struct Expr : public std::enable_shared_from_this<Expr> {
    virtual Value accept(ExprVisitor* visitor) = 0;
    virtual ~Expr() = default;
};

struct AssignExpr : Expr
{
    const Token name;
    const Token op;
    std::shared_ptr<Expr> value;
    AssignExpr(Token name, Token op, std::shared_ptr<Expr> value)
        : name(name), op(op), value(value) {}
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
        : left(left), oper(oper), right(right) {}
    Value accept(ExprVisitor* visitor) override{
        return visitor->visitBinaryExpr(std::static_pointer_cast<BinaryExpr>(shared_from_this()));
    }
};

struct GroupingExpr : Expr
{
    std::shared_ptr<Expr> expression;

    explicit GroupingExpr(std::shared_ptr<Expr> expression) : expression(expression) {}
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
    Token oper;
    std::shared_ptr<Expr> right;
    UnaryExpr(Token oper, std::shared_ptr<Expr> right) : oper(oper), right(right) {}
    Value accept(ExprVisitor* visitor) override{
        return visitor->visitUnaryExpr(std::static_pointer_cast<UnaryExpr>(shared_from_this()));
    }
};

struct VarExpr : Expr
{
    Token name;
    explicit VarExpr(Token name) : name(name){};
    Value accept(ExprVisitor* visitor) override
    {
        return visitor->visitVarExpr(std::static_pointer_cast<VarExpr>(shared_from_this()));
    }
};

struct FunctionExpr : Expr {
    std::vector<Token> params;
    std::vector<std::shared_ptr<Stmt>> body;
    FunctionExpr(const std::vector<Token>& params, const std::vector<std::shared_ptr<Stmt>>& body)
        : params(params), body(body) {}
    Value accept(ExprVisitor* visitor) override
    {
        return visitor->visitFunctionExpr(std::static_pointer_cast<FunctionExpr>(shared_from_this()));
    }
};

struct CallExpr : Expr
{
    std::shared_ptr<Expr> callee;
    Token paren;
    std::vector<std::shared_ptr<Expr>> arguments;
    bool isTailCall = false;  // Flag for tail call optimization
    
    CallExpr(std::shared_ptr<Expr> callee, Token paren, std::vector<std::shared_ptr<Expr>> arguments)
        : callee(callee), paren(paren), arguments(arguments) {}
    Value accept(ExprVisitor* visitor) override
    {
        return visitor->visitCallExpr(std::static_pointer_cast<CallExpr>(shared_from_this()));
    }
};

struct IncrementExpr : Expr
{
    std::shared_ptr<Expr> operand;
    Token oper;
    bool isPrefix;  // true for ++x, false for x++
    
    IncrementExpr(std::shared_ptr<Expr> operand, Token oper, bool isPrefix)
        : operand(operand), oper(oper), isPrefix(isPrefix) {}
    Value accept(ExprVisitor* visitor) override
    {
        return visitor->visitIncrementExpr(std::static_pointer_cast<IncrementExpr>(shared_from_this()));
    }
};

struct TernaryExpr : Expr
{
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Expr> thenExpr;
    std::shared_ptr<Expr> elseExpr;
    
    TernaryExpr(std::shared_ptr<Expr> condition, std::shared_ptr<Expr> thenExpr, std::shared_ptr<Expr> elseExpr)
        : condition(condition), thenExpr(thenExpr), elseExpr(elseExpr) {}
    Value accept(ExprVisitor* visitor) override
    {
        return visitor->visitTernaryExpr(std::static_pointer_cast<TernaryExpr>(shared_from_this()));
    }
};

struct ArrayLiteralExpr : Expr
{
    std::vector<std::shared_ptr<Expr>> elements;
    
    explicit ArrayLiteralExpr(const std::vector<std::shared_ptr<Expr>>& elements)
        : elements(elements) {}
    Value accept(ExprVisitor* visitor) override
    {
        return visitor->visitArrayLiteralExpr(std::static_pointer_cast<ArrayLiteralExpr>(shared_from_this()));
    }
};

struct ArrayIndexExpr : Expr
{
    std::shared_ptr<Expr> array;
    std::shared_ptr<Expr> index;
    Token bracket;  // The closing bracket token for error reporting
    
    ArrayIndexExpr(std::shared_ptr<Expr> array, std::shared_ptr<Expr> index, Token bracket)
        : array(array), index(index), bracket(bracket) {}
    Value accept(ExprVisitor* visitor) override
    {
        return visitor->visitArrayIndexExpr(std::static_pointer_cast<ArrayIndexExpr>(shared_from_this()));
    }
};

struct ArrayAssignExpr : Expr
{
    std::shared_ptr<Expr> array;
    std::shared_ptr<Expr> index;
    std::shared_ptr<Expr> value;
    Token bracket;  // The closing bracket token for error reporting
    
    ArrayAssignExpr(std::shared_ptr<Expr> array, std::shared_ptr<Expr> index, std::shared_ptr<Expr> value, Token bracket)
        : array(array), index(index), value(value), bracket(bracket) {}
    Value accept(ExprVisitor* visitor) override
    {
        return visitor->visitArrayAssignExpr(std::static_pointer_cast<ArrayAssignExpr>(shared_from_this()));
    }
};

struct DictLiteralExpr : Expr
{
    std::vector<std::pair<std::string, std::shared_ptr<Expr>>> pairs;
    
    explicit DictLiteralExpr(const std::vector<std::pair<std::string, std::shared_ptr<Expr>>>& pairs)
        : pairs(pairs) {}
    Value accept(ExprVisitor* visitor) override
    {
        return visitor->visitDictLiteralExpr(std::static_pointer_cast<DictLiteralExpr>(shared_from_this()));
    }
};



