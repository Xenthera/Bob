//
// Created by Bobby Lucero on 5/21/23.
//


#include "../headers/bob.h"
#include "../headers/Expression.h"
#include "../headers/Lexer.h"
#include "../headers/ASTPrinter.h"
#include "../headers/TypeWrapper.h"
int main(){
    Object::count = 0;
    Bob bobLang;

    //bobLang.runFile("source.bob");
    bobLang.runPrompt();

    //ASTPrinter printer;
//
//    std::shared_ptr<Expr<std::shared_ptr<Object>>> expression = std::make_shared<BinaryExpr<std::shared_ptr<Object>> >(
//            std::make_shared<UnaryExpr<std::shared_ptr<Object>>>(
//                    Token{MINUS, "-", 1},
//                    std::make_shared<LiteralExpr<std::shared_ptr<Object>>>("123", true)
//                    ),
//            Token{MINUS, "-", 1},
//            std::make_shared<GroupingExpr<std::shared_ptr<Object>>>(
//                    std::make_shared<LiteralExpr<std::shared_ptr<Object>>>("45.67", true)
//                    )
//            );
//
////    Expr<std::string>* e = new BinaryExpr<std::string>(
////                new UnaryExpr<std::string>(Token{MINUS, "-", 0}, new LiteralExpr<std::string>("123")),
////                Token{STAR, "*", 0},
////                new UnaryExpr<std::string>(Token{PLUS, "+", 0}, new LiteralExpr<std::string>("535"))
////            );
//
//
//    std::cout << std::dynamic_pointer_cast<String>(printer.print(expression.get()))->value;
//
//    std::cout << std::endl;

    //bobLang.runPrompt();

//    std::shared_ptr<Object> object = std::make_shared<String>(String{"Hi"});
//
//    if(auto num = std::dynamic_pointer_cast<Number>(object))
//    {
//        std::cout << num->value << std::endl;
//    }else if(auto str = std::dynamic_pointer_cast<String>(object))
//    {
//        std::cout << str->value << std::endl;
//    }

    return 0;
}
