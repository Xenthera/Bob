//
// Created by Bobby Lucero on 5/21/23.
//


#include "../headers/bob.h"
#include "../headers/Expression.h"
#include "../headers/Lexer.h"
#include "../headers/ASTPrinter.h"
int main(){

    Bob bobLang;

    //bobLang.runFile("source.bob");

    ASTPrinter printer;

    std::shared_ptr<Expr<std::string> > expression = std::make_shared<BinaryExpr<std::string> >(
            std::make_shared<UnaryExpr<std::string> >(
                    Token{MINUS, "-", 1},
                    std::make_shared<LiteralExpr<std::string>>("123", true)
                    ),
            Token{STAR, "*", 1},
            std::make_shared<GroupingExpr<std::string> >(
                    std::make_shared<LiteralExpr<std::string> >("45.67", true)
                    )
            );

//    Expr<std::string>* e = new BinaryExpr<std::string>(
//                new UnaryExpr<std::string>(Token{MINUS, "-", 0}, new LiteralExpr<std::string>("123")),
//                Token{STAR, "*", 0},
//                new UnaryExpr<std::string>(Token{PLUS, "+", 0}, new LiteralExpr<std::string>("535"))
//            );
    LiteralExpr<std::string>* le = new LiteralExpr<std::string>("123", true);

    std::cout << printer.print(expression.get());

    std::cout << std::endl;

    //bobLang.runPrompt();

    return 0;
}
