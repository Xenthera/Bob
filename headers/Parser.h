#pragma once
#include <utility>
#include <vector>
#include "Lexer.h"
#include "Expression.h"
#include "Statement.h"
#include "TypeWrapper.h"
#include "helperFunctions/ShortHands.h"

class Parser
{
private:
    const std::vector<Token> tokens;
    int current = 0;

public:
    explicit Parser(std::vector<Token> tokens) : tokens(std::move(tokens)){};
    std::vector<sptr(Stmt)> parse();

private:
    sptr(Expr) expression();
    sptr(Expr) equality();
    sptr(Expr) comparison();
    sptr(Expr) term();
    sptr(Expr) factor();
    sptr(Expr) unary();
    sptr(Expr) primary();

    bool match(std::vector<TokenType> types);

    bool check(TokenType type);
    bool isAtEnd();
    Token advance();
    Token peek();
    Token previous();
    Token consume(TokenType type, std::string message);
    sptr(Stmt) statement();

    void sync();

    std::shared_ptr<Stmt> printStatement();

    std::shared_ptr<Stmt> expressionStatement();

    std::shared_ptr<Stmt> declaration();

    std::shared_ptr<Stmt> varDeclaration();

    std::shared_ptr<Expr> assignment();

    std::vector<std::shared_ptr<Stmt>> block();
};