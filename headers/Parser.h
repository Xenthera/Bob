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

    bool match(const std::vector<TokenType>& types);

    bool check(TokenType type);
    bool isAtEnd();
    Token advance();
    Token peek();
    Token previous();
    Token consume(TokenType type, const std::string& message);
    sptr(Stmt) statement();

    void sync();



    std::shared_ptr<Stmt> expressionStatement();

    std::shared_ptr<Stmt> returnStatement();

    std::shared_ptr<Stmt> ifStatement();

    std::shared_ptr<Stmt> declaration();

    std::shared_ptr<Stmt> varDeclaration();

    std::shared_ptr<Stmt> functionDeclaration();

    sptr(Expr) assignment();

    std::vector<std::shared_ptr<Stmt>> block();
    
    sptr(Expr) finishCall(sptr(Expr) callee);
};