//
// Created by Bobby Lucero on 5/26/23.
//
#include "../headers/Parser.h"


//              Precedence
// to all the morons on facebook who don't know what pemdas is, fuck you
///////////////////////////////////////////

std::shared_ptr<Expr<std::shared_ptr<Object>>> Parser::expression()
{
    return equality();
}

std::shared_ptr<Expr<std::shared_ptr<Object>>> Parser::equality()
{
    std::shared_ptr<Expr<std::shared_ptr<Object>>> expr = comparison();

    while(match({BANG_EQUAL, DOUBLE_EQUAL}))
    {
        Token op = previous();
        std::shared_ptr<Expr<std::shared_ptr<Object>>> right = comparison();
        expr = std::make_shared<BinaryExpr<std::shared_ptr<Object>>>(expr, op, right);
    }

    return expr;
}

std::shared_ptr<Expr<std::shared_ptr<Object>>> Parser::comparison()
{
    std::shared_ptr<Expr<std::shared_ptr<Object>>> expr = term();

    while(match({GREATER, GREATER_EQUAL, LESS, LESS_EQUAL}))
    {
        Token op = previous();
        std::shared_ptr<Expr<std::shared_ptr<Object>>> right = term();
        expr = std::make_shared<BinaryExpr<std::shared_ptr<Object>>>(expr, op, right);
    }

    return expr;
}

std::shared_ptr<Expr<std::shared_ptr<Object>>> Parser::term()
{
    std::shared_ptr<Expr<std::shared_ptr<Object>>> expr = factor();

    while(match({MINUS, PLUS}))
    {
        std::cout << "Found comparison" << std::endl;
        Token op = previous();
        std::shared_ptr<Expr<std::shared_ptr<Object>>> right = factor();
        expr = std::make_shared<BinaryExpr<std::shared_ptr<Object>>>(expr, op, right);
    }

    return expr;
}

std::shared_ptr<Expr<std::shared_ptr<Object>>> Parser::factor()
{
    std::shared_ptr<Expr<std::shared_ptr<Object>>> expr = unary();

    while(match({SLASH, STAR}))
    {
        Token op = previous();
        std::shared_ptr<Expr<std::shared_ptr<Object>>> right = unary();
        expr = std::make_shared<BinaryExpr<std::shared_ptr<Object>>>(expr, op, right);
    }

    return expr;
}

std::shared_ptr<Expr<std::shared_ptr<Object>>> Parser::unary()
{
    if(match({BANG, MINUS}))
    {
        Token op = previous();
        std::shared_ptr<Expr<std::shared_ptr<Object>>> right = unary();
        return std::make_shared<UnaryExpr<std::shared_ptr<Object>>>(op, right);
    }

    return primary();
}

std::shared_ptr<Expr<std::shared_ptr<Object>>> Parser::primary()
{
    if(match({FALSE})) return std::make_shared<LiteralExpr<std::shared_ptr<Object>>>("true", false);
    if(match({TRUE})) return std::make_shared<LiteralExpr<std::shared_ptr<Object>>>("true", false);
    if(match({NONE})) return std::make_shared<LiteralExpr<std::shared_ptr<Object>>>("none", false);

    if(match({NUMBER})) return std::make_shared<LiteralExpr<std::shared_ptr<Object>>>(previous().lexeme, true);
    if(match({STRING})) return std::make_shared<LiteralExpr<std::shared_ptr<Object>>>(previous().lexeme, false);

    if(match({OPEN_PAREN}))
    {
        std::shared_ptr<Expr<std::shared_ptr<Object>>> expr = expression();
        consume(CLOSE_PAREN, "Expected ')' after expression on line " + std::to_string(peek().line));
        return std::make_shared<GroupingExpr<std::shared_ptr<Object>>>(expr);
    }

    throw std::runtime_error("Expression expected at: " + std::to_string(peek().line));
}

///////////////////////////////////////////


std::shared_ptr<Expr<std::shared_ptr<Object>>> Parser::parse() {

        return expression();

}



bool Parser::match(std::vector<TokenType> types) {
    for(TokenType t : types)
    {
        if(check(t))
        {
            advance();
            return true;
        }
    }

    return false;
}

bool Parser::check(TokenType type) {
    if(isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::isAtEnd() {
   return peek().type == END_OF_FILE;
}

Token Parser::advance() {
    if(!isAtEnd()) current++;
    return previous();
}

Token Parser::peek() {
    return tokens[current];
}

Token Parser::previous() {
    return tokens[current - 1];
}

Token Parser::consume(TokenType type, std::string message) {
    if(check(type)) return advance();

    throw std::runtime_error(peek().lexeme +": "+ message);
}

void Parser::sync()
{
    advance();
    while(!isAtEnd())
    {
        if(previous().type == SEMICOLON) return;

        switch (peek().type) {
            case CLASS:
            case FUNCTION:
            case VAR:
            case FOR:
            case IF:
            case WHILE:
            case RETURN:
                return;
        }

        advance();
    }
}


