//
// Created by Bobby Lucero on 5/23/23.
//
#include "../headers/ASTPrinter.h"


std::string ASTPrinter::visitBinaryExpr(BinaryExpr<std::string>* expression){
    std::cout << expression->left << std::endl;
    return parenthesize(expression->oper.lexeme, std::vector<std::shared_ptr<Expr<std::string>>>{expression->left, expression->right});
}

std::string ASTPrinter::visitGroupingExpr(GroupingExpr<std::string>* expression){
    return parenthesize("group", std::vector<std::shared_ptr<Expr<std::string>>>{expression->expression});
}
std::string ASTPrinter::visitLiteralExpr(LiteralExpr<std::string>* expression){
    return expression->value;
}
std::string ASTPrinter::visitUnaryExpr(UnaryExpr<std::string>* expression){
    return parenthesize(expression->oper.lexeme, std::vector<std::shared_ptr<Expr<std::string>>>{expression->right});
}

std::string ASTPrinter::print(Expr<std::string> *expr) {
    return expr->accept(this);
}

std::string ASTPrinter::parenthesize(std::string name, std::vector<std::shared_ptr<Expr<std::string>>> exprs) {
    std::string builder;

    builder += "(" + name;

    for(std::shared_ptr<Expr<std::string>> expr : exprs)
    {
        std::cout << expr << std::endl;

        builder += " ";
        builder += expr->accept(this);
    }

    builder += ")";

    return builder;

}
