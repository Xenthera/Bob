//
// Created by Bobby Lucero on 5/23/23.
//
#include "../headers/ASTPrinter.h"


sptr(Object) ASTPrinter::visitBinaryExpr(BinaryExpr* expression){
    std::cout << expression->left << std::endl;
    return parenthesize(expression->oper.lexeme, std::vector<sptr(Expr)>{expression->left, expression->right});
}

sptr(Object) ASTPrinter::visitGroupingExpr(GroupingExpr* expression){
    return parenthesize("group", std::vector<sptr(Expr)>{expression->expression});
}
sptr(Object) ASTPrinter::visitLiteralExpr(LiteralExpr* expression){
    return msptr(String)(expression->value);
}
sptr(Object) ASTPrinter::visitUnaryExpr(UnaryExpr* expression){
    return parenthesize(expression->oper.lexeme, std::vector<sptr(Expr)>{expression->right});
}

sptr(Object) ASTPrinter::print(Expr *expr) {
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
