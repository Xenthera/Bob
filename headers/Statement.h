#pragma once


#include "helperFunctions/ShortHands.h"
#include "TypeWrapper.h"
#include "Expression.h"

struct ExpressionStmt;
struct VarStmt;
struct BlockStmt;
struct FunctionStmt;
struct ReturnStmt;
struct IfStmt;

struct StmtVisitor
{
    virtual void visitBlockStmt(const std::shared_ptr<BlockStmt>& stmt) = 0;
    virtual void visitExpressionStmt(const std::shared_ptr<ExpressionStmt>& stmt) = 0;
    virtual void visitVarStmt(const std::shared_ptr<VarStmt>& stmt) = 0;
    virtual void visitFunctionStmt(const std::shared_ptr<FunctionStmt>& stmt) = 0;
    virtual void visitReturnStmt(const std::shared_ptr<ReturnStmt>& stmt) = 0;
    virtual void visitIfStmt(const std::shared_ptr<IfStmt>& stmt) = 0;
};

struct Stmt : public std::enable_shared_from_this<Stmt>
{
    std::shared_ptr<Expr> expression;
    virtual void accept(StmtVisitor* visitor) = 0;
    virtual ~Stmt(){};
};

struct BlockStmt : Stmt
{
    std::vector<std::shared_ptr<Stmt>> statements;
    explicit BlockStmt(std::vector<std::shared_ptr<Stmt>> statements) : statements(statements)
    {
    }
    void accept(StmtVisitor* visitor) override
    {
        visitor->visitBlockStmt(std::static_pointer_cast<BlockStmt>(shared_from_this()));
    }
};

struct ExpressionStmt : Stmt
{
    std::shared_ptr<Expr> expression;
    explicit ExpressionStmt(std::shared_ptr<Expr> expression) : expression(expression)
    {
    }

    void accept(StmtVisitor* visitor) override
    {
        visitor->visitExpressionStmt(std::static_pointer_cast<ExpressionStmt>(shared_from_this()));
    }
};



struct VarStmt : Stmt
{
    Token name;
    std::shared_ptr<Expr> initializer;
    VarStmt(Token name, std::shared_ptr<Expr> initializer) : name(name), initializer(initializer)
    {
    }

    void accept(StmtVisitor* visitor) override
    {
        visitor->visitVarStmt(std::static_pointer_cast<VarStmt>(shared_from_this()));
    }
};

struct FunctionStmt : Stmt
{
    const Token name;
    const std::vector<Token> params;
    std::vector<std::shared_ptr<Stmt>> body;

    FunctionStmt(Token name, std::vector<Token> params, std::vector<std::shared_ptr<Stmt>> body) 
        : name(name), params(params), body(body) {}

    void accept(StmtVisitor* visitor) override
    {
        visitor->visitFunctionStmt(std::static_pointer_cast<FunctionStmt>(shared_from_this()));
    }
};

struct ReturnStmt : Stmt
{
    const Token keyword;
    std::shared_ptr<Expr> value;

    ReturnStmt(Token keyword, std::shared_ptr<Expr> value) : keyword(keyword), value(value) {}

    void accept(StmtVisitor* visitor) override
    {
        visitor->visitReturnStmt(std::static_pointer_cast<ReturnStmt>(shared_from_this()));
    }
};

struct IfStmt : Stmt
{
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> thenBranch;
    std::shared_ptr<Stmt> elseBranch;

    IfStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch) 
        : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}

    void accept(StmtVisitor* visitor) override
    {
        visitor->visitIfStmt(std::static_pointer_cast<IfStmt>(shared_from_this()));
    }
};