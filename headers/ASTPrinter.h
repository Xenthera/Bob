#pragma once
#include "Expression.h"
#include "TypeWrapper.h"
#include <string>
#include <initializer_list>

class ASTPrinter : public Visitor<std::shared_ptr<Object>>
{
    std::shared_ptr<Object> visitBinaryExpr(BinaryExpr<std::shared_ptr<Object>>* expression) override;
    std::shared_ptr<Object> visitGroupingExpr(GroupingExpr<std::shared_ptr<Object>>* expression) override;
    std::shared_ptr<Object> visitLiteralExpr(LiteralExpr<std::shared_ptr<Object>>* expression) override;
    std::shared_ptr<Object> visitUnaryExpr(UnaryExpr<std::shared_ptr<Object>>* expression) override;
public:
    std::shared_ptr<Object> print(Expr<std::shared_ptr<Object>>* expr);
private:
    std::shared_ptr<Object> parenthesize(std::string name,  std::vector<std::shared_ptr<Expr<std::shared_ptr<Object>>>> exprs);

};