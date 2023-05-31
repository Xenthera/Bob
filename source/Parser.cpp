//
// Created by Bobby Lucero on 5/26/23.
//
#include "../headers/Parser.h"


//              Precedence
// to all the morons on facebook who don't know what pemdas is, fuck you
///////////////////////////////////////////

sptr(Expr) Parser::expression()
{
    return assignment();
}

sptr(Expr) Parser::assignment()
{

    sptr(Expr) expr = equality();

    if(match({EQUAL}))
    {
        Token equals = previous();

        sptr(Expr) value = assignment();

        if(std::dynamic_pointer_cast<VarExpr>(expr))
        {

            Token name = std::dynamic_pointer_cast<VarExpr>(expr)->name;

            return msptr(AssignExpr)(name, value);
        }

        throw std::runtime_error("Invalid assignment target.");
    }

    return expr;
}

sptr(Expr) Parser::equality()
{
    sptr(Expr) expr = comparison();

    while(match({BANG_EQUAL, DOUBLE_EQUAL}))
    {
        Token op = previous();
        sptr(Expr) right = comparison();
        expr = msptr(BinaryExpr)(expr, op, right);
    }

    return expr;
}

sptr(Expr) Parser::comparison()
{
    sptr(Expr) expr = term();

    while(match({GREATER, GREATER_EQUAL, LESS, LESS_EQUAL}))
    {
        Token op = previous();
        sptr(Expr) right = term();
        expr = msptr(BinaryExpr)(expr, op, right);
    }

    return expr;
}

sptr(Expr) Parser::term()
{
    sptr(Expr) expr = factor();

    while(match({MINUS, PLUS}))
    {
        Token op = previous();
        sptr(Expr) right = factor();
        expr = msptr(BinaryExpr)(expr, op, right);
    }

    return expr;
}

sptr(Expr) Parser::factor()
{
    sptr(Expr) expr = unary();

    while(match({SLASH, STAR, PERCENT}))
    {
        Token op = previous();
        sptr(Expr) right = unary();
        expr = msptr(BinaryExpr)(expr, op, right);
    }

    return expr;
}

sptr(Expr) Parser::unary()
{
    if(match({BANG, MINUS}))
    {
        Token op = previous();
        sptr(Expr) right = unary();
        return msptr(UnaryExpr)(op, right);
    }

    return primary();
}

sptr(Expr) Parser::primary()
{
    if(match({FALSE})) return msptr(LiteralExpr)("false", false, false);
    if(match({TRUE})) return msptr(LiteralExpr)("true", false, false);
    if(match({NONE})) return msptr(LiteralExpr)("none", false, true);

    if(match({NUMBER})) return msptr(LiteralExpr)(previous().lexeme, true, false);
    if(match({STRING})) return msptr(LiteralExpr)(previous().lexeme, false, false);

    if(match( {IDENTIFIER})) {
        return msptr(VarExpr)(previous());
    }

    if(match({OPEN_PAREN}))
    {
        sptr(Expr) expr = expression();
        consume(CLOSE_PAREN, "Expected ')' after expression on line " + std::to_string(peek().line));
        return msptr(GroupingExpr)(expr);
    }

    throw std::runtime_error("Expression expected at: " + std::to_string(peek().line));
}

///////////////////////////////////////////


std::vector<sptr(Stmt)> Parser::parse() {

        std::vector<sptr(Stmt)> statements;
        while(!isAtEnd())
        {
            statements.push_back(declaration());
        }

        return statements;

}

sptr(Stmt) Parser::statement()
{
    if(match({PRINT})) return printStatement();
    return expressionStatement();
}

sptr(Stmt) Parser::printStatement()
{
    sptr(Expr) value = expression();
    consume(SEMICOLON, "Expected ';' after value.");
    return msptr(PrintStmt)(value);
}

sptr(Stmt) Parser::expressionStatement()
{
    sptr(Expr) expr = expression();
    consume(SEMICOLON, "Expected ';' after expression.");
    return msptr(ExpressionStmt)(expr);
}

sptr(Stmt) Parser::declaration()
{
    try{
        if(match({VAR})) return varDeclaration();
        return statement();
    }
    catch(std::runtime_error e)
    {
        sync();
        throw std::runtime_error(e.what());
    }
}

sptr(Stmt) Parser::varDeclaration()
{
    Token name = consume(IDENTIFIER, "Expected variable name.");

    sptr(Expr) initializer = msptr(LiteralExpr)("none", false, true);
    if(match({EQUAL}))
    {
        initializer = expression();
    }
    consume(SEMICOLON, "Expected ';' after variable declaration.");
    return msptr(VarStmt)(name, initializer);
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


