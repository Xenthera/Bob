//
// Created by Bobby Lucero on 5/23/23.
//
#include "../headers/ASTPrinter.h"


sptr(Object) ASTPrinter::visitBinaryExpr(sptr(BinaryExpr) expression){
    std::cout << expression->left << std::endl;
    return parenthesize(expression->oper.lexeme, std::vector<sptr(Expr)>{expression->left, expression->right});
}

sptr(Object) ASTPrinter::visitGroupingExpr(sptr(GroupingExpr) expression){
    return parenthesize("group", std::vector<sptr(Expr)>{expression->expression});
}
sptr(Object) ASTPrinter::visitLiteralExpr(sptr(LiteralExpr) expression){
    return msptr(String)(expression->value);
}
sptr(Object) ASTPrinter::visitUnaryExpr(sptr(UnaryExpr) expression){
    return parenthesize(expression->oper.lexeme, std::vector<sptr(Expr)>{expression->right});
}

sptr(Object) ASTPrinter::print(sptr(Expr) expr) {
    return expr->accept(this);
}

sptr(Object) ASTPrinter::parenthesize(std::string name, std::vector<sptr(Expr)> exprs) {
    std::string builder;

    builder += "(" + name;

    for(const sptr(Expr)& expr : exprs)
    {
        builder += " ";
        builder += std::dynamic_pointer_cast<String>(expr->accept(this))->value;
    }

    builder += ")";

    return msptr(String)(builder);

}

sptr(Object) ASTPrinter::visitAssignExpr(std::shared_ptr<AssignExpr> expr) {
    return std::shared_ptr<String>();
}

sptr(Object) ASTPrinter::visitVariableExpr(std::shared_ptr<VarExpr> expr) {
    return std::shared_ptr<String>();
}
