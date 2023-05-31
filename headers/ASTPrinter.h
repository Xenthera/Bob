#pragma once
#include "Expression.h"
#include "TypeWrapper.h"
#include "helperFunctions/ShortHands.h"
#include <string>
#include <initializer_list>


class ASTPrinter : ExprVisitor
{
    sptr(Object) visitBinaryExpr(sptr(BinaryExpr) expr) override;
    sptr(Object) visitGroupingExpr(sptr(GroupingExpr) expr) override;
    sptr(Object) visitLiteralExpr(sptr(LiteralExpr) expr) override;
    sptr(Object) visitUnaryExpr(sptr(UnaryExpr) expr) override;
    sptr(Object) visitAssignExpr(sptr(AssignExpr) expr) override;
    sptr(Object) visitVariableExpr(sptr(VarExpr) expr) override;


public:
    sptr(Object) print(sptr(Expr) expr);
private:
    sptr(Object) parenthesize(std::string name,  std::vector<sptr(Expr)> exprs);

};