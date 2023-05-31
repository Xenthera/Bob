#pragma once
#include "Expression.h"
#include "Statement.h"
#include "helperFunctions/ShortHands.h"
#include "TypeWrapper.h"
#include "Environment.h"

class Interpreter : ExprVisitor, StmtVisitor
{

public:
    sptr(Object) visitBinaryExpr(sptr(BinaryExpr) expression) override;
    sptr(Object) visitGroupingExpr(sptr(GroupingExpr) expression) override;
    sptr(Object) visitLiteralExpr(sptr(LiteralExpr) expression) override;
    sptr(Object) visitUnaryExpr(sptr(UnaryExpr) expression) override;
    sptr(Object) visitVariableExpr(sptr(VarExpr) expression) override;
    sptr(Object) visitAssignExpr(sptr(AssignExpr) expression) override;

    void visitExpressionStmt(sptr(ExpressionStmt) statement) override;
    void visitPrintStmt(sptr(PrintStmt) statement) override;
    void visitVarStmt(sptr(VarStmt) statement) override;

    void interpret(std::vector<sptr(Stmt)> statements);

private:

    Environment environment;

    sptr(Object) evaluate(sptr(Expr) expr);
    bool isTruthy(sptr(Object) object);
    bool isEqual(sptr(Object) a, sptr(Object) b);

    std::string stringify(sptr(Object) object);

    bool isWholeNumer(double num);

    void execute(std::shared_ptr<Stmt> statement);
};
