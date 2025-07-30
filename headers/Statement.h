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
    virtual void visitBlockStmt(sptr(BlockStmt) stmt) = 0;
    virtual void visitExpressionStmt(sptr(ExpressionStmt) stmt) = 0;
    virtual void visitVarStmt(sptr(VarStmt) stmt) = 0;
    virtual void visitFunctionStmt(sptr(FunctionStmt) stmt) = 0;
    virtual void visitReturnStmt(sptr(ReturnStmt) stmt) = 0;
    virtual void visitIfStmt(sptr(IfStmt) stmt) = 0;
};

struct Stmt
{
    const sptr(Expr) expression;
    virtual void accept(StmtVisitor* visitor) = 0;
    virtual ~Stmt(){};
};

struct BlockStmt : Stmt, public std::enable_shared_from_this<BlockStmt>
{
    const std::vector<sptr(Stmt)> statements;
    explicit BlockStmt(std::vector<sptr(Stmt)> statements) : statements(statements)
    {
    }
    void accept(StmtVisitor* visitor) override
    {
        visitor->visitBlockStmt(shared_from_this());
    }
};

struct ExpressionStmt : Stmt, public std::enable_shared_from_this<ExpressionStmt>
{
    const sptr(Expr) expression;
    explicit ExpressionStmt(sptr(Expr) expression) : expression(expression)
    {
    }

    void accept(StmtVisitor* visitor) override
    {
        visitor->visitExpressionStmt(shared_from_this());
    }
};



struct VarStmt : Stmt, public std::enable_shared_from_this<VarStmt>
{
    Token name;
    const sptr(Expr) initializer;
    VarStmt(Token name, sptr(Expr) initializer) : name(name), initializer(initializer)
    {
    }

    void accept(StmtVisitor* visitor) override
    {
        visitor->visitVarStmt(shared_from_this());
    }
};

struct FunctionStmt : Stmt, public std::enable_shared_from_this<FunctionStmt>
{
    const Token name;
    const std::vector<Token> params;
    const std::vector<sptr(Stmt)> body;

    FunctionStmt(Token name, std::vector<Token> params, std::vector<sptr(Stmt)> body) 
        : name(name), params(params), body(body) {}

    void accept(StmtVisitor* visitor) override
    {
        visitor->visitFunctionStmt(shared_from_this());
    }
};

struct ReturnStmt : Stmt, public std::enable_shared_from_this<ReturnStmt>
{
    const Token keyword;
    const sptr(Expr) value;

    ReturnStmt(Token keyword, sptr(Expr) value) : keyword(keyword), value(value) {}

    void accept(StmtVisitor* visitor) override
    {
        visitor->visitReturnStmt(shared_from_this());
    }
};

struct IfStmt : Stmt, public std::enable_shared_from_this<IfStmt>
{
    const sptr(Expr) condition;
    const sptr(Stmt) thenBranch;
    const sptr(Stmt) elseBranch;

    IfStmt(sptr(Expr) condition, sptr(Stmt) thenBranch, sptr(Stmt) elseBranch) 
        : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}

    void accept(StmtVisitor* visitor) override
    {
        visitor->visitIfStmt(shared_from_this());
    }
};