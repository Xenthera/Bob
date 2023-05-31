#pragma once


#include "helperFunctions/ShortHands.h"
#include "TypeWrapper.h"
#include "Expression.h"

struct ExpressionStmt;
struct PrintStmt;
struct VarStmt;

struct StmtVisitor
{
    virtual void visitExpressionStmt(sptr(ExpressionStmt) stmt) = 0;
    virtual void visitPrintStmt(sptr(PrintStmt) stmt) = 0;
    virtual void visitVarStmt(sptr(VarStmt) stmt) = 0;
};

struct Stmt
{
    const sptr(Expr) expression;
    virtual void accept(StmtVisitor* visitor) = 0;
    virtual ~Stmt(){};
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

struct PrintStmt : Stmt, public std::enable_shared_from_this<PrintStmt>
{
    const sptr(Expr) expression;
    explicit PrintStmt(sptr(Expr) expression) : expression(expression)
    {
    }

    void accept(StmtVisitor* visitor) override
    {
        visitor->visitPrintStmt(shared_from_this());
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