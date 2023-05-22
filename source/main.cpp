//
// Created by Bobby Lucero on 5/21/23.
//
#include "../headers/bob.h"
#include "../headers/Expression.h"
#include "../headers/Lexer.h"
int main(){

    Bob bobLang;

    //bobLang.runFile("source.bob");


    Expr a;
    Expr b;
    Token t = {PLUS, "+", 1};
    Token t2 = {MINUS, "-", 1};
    BinaryExpr e = BinaryExpr(a, t, b);

    std::shared_ptr<Expr> any = std::make_shared<BinaryExpr>(a, t, b);
    if(std::shared_ptr<BinaryExpr> binexpr = std::dynamic_pointer_cast<BinaryExpr>(any))
    {
        std::cout << binexpr->oper.lexeme;
    }

    any = std::make_shared<BinaryExpr>(a, t2, b);
    if(std::shared_ptr<BinaryExpr> binexpr = std::dynamic_pointer_cast<BinaryExpr>(any))
    {
        std::cout << binexpr->oper.lexeme;
    }

    std::cout << std::endl;

    bobLang.runPrompt();

    return 0;
}
