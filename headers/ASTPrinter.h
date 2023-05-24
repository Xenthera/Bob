#pragma once
#include "Expression.h"
#include <string>
#include <initializer_list>

class ASTPrinter : public Visitor<std::string>
{
    std::string visitBinaryExpr(BinaryExpr<std::string>* expression) override;
//    std::string visitGroupingExpr(GroupingExpr<std::string> expression);
    std::string visitLiteralExpr(LiteralExpr<std::string>* expression) override;
    std::string visitUnaryExpr(UnaryExpr<std::string>* expression) override;
public:
    int test = 10;
    std::string print(Expr<std::string>* expr);
private:
    std::string parenthesize(std::string name,  std::vector<std::shared_ptr<Expr<std::string> > > exprs);

};