#pragma once
#include "Expression.h"
#include "TypeWrapper.h"
#include "helperFunctions/ShortHands.h"
#include <string>
#include <initializer_list>


class ASTPrinter : public Visitor
{
    sptr(Object) visitBinaryExpr(BinaryExpr* expression) override;
    sptr(Object) visitGroupingExpr(GroupingExpr* expression) override;
    sptr(Object) visitLiteralExpr(LiteralExpr* expression) override;
    sptr(Object) visitUnaryExpr(UnaryExpr* expression) override;
public:
    sptr(Object) print(Expr* expr);
private:
    sptr(Object) parenthesize(std::string name,  std::vector<sptr(Expr)> exprs);

};