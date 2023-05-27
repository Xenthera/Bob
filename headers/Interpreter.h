#pragma once
#include "Expression.h"
#include "helperFunctions/ShortHands.h"
#include "TypeWrapper.h"

class Interpreter : Visitor
{

public:
    sptr(Object) visitBinaryExpr(sptr(BinaryExpr) expression) override;
    sptr(Object) visitGroupingExpr(sptr(GroupingExpr) expression) override;
    sptr(Object) visitLiteralExpr(sptr(LiteralExpr) expression) override;
    sptr(Object) visitUnaryExpr(sptr(UnaryExpr) expression) override;

    void interpret(sptr(Expr) expr);

private:
    sptr(Object) evaluate(sptr(Expr) expr);
    bool isTruthy(sptr(Object) object);
    bool isEqual(sptr(Object) a, sptr(Object) b);

    std::string stringify(sptr(Object) object);
};
