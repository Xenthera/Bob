#pragma once
#include <utility>
#include <vector>
#include "Lexer.h"
#include "Expression.h"
#include "TypeWrapper.h"

class Parser
{
private:
    const std::vector<Token> tokens;
    int current = 0;

public:
    explicit Parser(std::vector<Token> tokens) : tokens(std::move(tokens)){};
    std::shared_ptr<Expr<std::shared_ptr<Object>>> parse();

private:
    std::shared_ptr<Expr<std::shared_ptr<Object>>> expression();
    std::shared_ptr<Expr<std::shared_ptr<Object>>> equality();
    std::shared_ptr<Expr<std::shared_ptr<Object>>> comparison();
    std::shared_ptr<Expr<std::shared_ptr<Object>>> term();
    std::shared_ptr<Expr<std::shared_ptr<Object>>> factor();
    std::shared_ptr<Expr<std::shared_ptr<Object>>> unary();
    std::shared_ptr<Expr<std::shared_ptr<Object>>> primary();

    bool match(std::vector<TokenType> types);

    bool check(TokenType type);
    bool isAtEnd();
    Token advance();
    Token peek();
    Token previous();
    Token consume(TokenType type, std::string message);

    void sync();

};