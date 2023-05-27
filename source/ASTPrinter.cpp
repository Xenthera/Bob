//
// Created by Bobby Lucero on 5/23/23.
//
#include "../headers/ASTPrinter.h"


std::shared_ptr<Object> ASTPrinter::visitBinaryExpr(BinaryExpr<std::shared_ptr<Object>>* expression){
    std::cout << expression->left << std::endl;
    return parenthesize(expression->oper.lexeme, std::vector<std::shared_ptr<Expr<std::shared_ptr<Object>>>>{expression->left, expression->right});
}

std::shared_ptr<Object> ASTPrinter::visitGroupingExpr(GroupingExpr<std::shared_ptr<Object>>* expression){
    return parenthesize("group", std::vector<std::shared_ptr<Expr<std::shared_ptr<Object>>>>{expression->expression});
}
std::shared_ptr<Object> ASTPrinter::visitLiteralExpr(LiteralExpr<std::shared_ptr<Object>>* expression){
    return std::make_shared<String>(expression->value);
}
std::shared_ptr<Object> ASTPrinter::visitUnaryExpr(UnaryExpr<std::shared_ptr<Object>>* expression){
    return parenthesize(expression->oper.lexeme, std::vector<std::shared_ptr<Expr<std::shared_ptr<Object>>>>{expression->right});
}

std::shared_ptr<Object> ASTPrinter::print(Expr<std::shared_ptr<Object>> *expr) {
    return expr->accept(this);
}

std::shared_ptr<Object> ASTPrinter::parenthesize(std::string name, std::vector<std::shared_ptr<Expr<std::shared_ptr<Object>>>> exprs) {
    std::string builder;

    builder += "(" + name;

    for(const std::shared_ptr<Expr<std::shared_ptr<Object>>>& expr : exprs)
    {


        builder += " ";
        builder += std::dynamic_pointer_cast<String>(expr->accept(this))->value;
    }

    builder += ")";

    return std::make_shared<String>(builder);

}
