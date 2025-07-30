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
    if(match({BANG, MINUS, BIN_NOT}))
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
        if (check(OPEN_PAREN)) {
            return finishCall(msptr(VarExpr)(previous()));
        }
        return msptr(VarExpr)(previous());
    }

    if(match({OPEN_PAREN}))
    {
        sptr(Expr) expr = expression();
        consume(CLOSE_PAREN, "Expected ')' after expression on line " + std::to_string(peek().line));
        if (check(OPEN_PAREN)) {
            return finishCall(msptr(GroupingExpr)(expr));
        }
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

sptr(Stmt) Parser::declaration()
{
    try{
        if(match({VAR})) return varDeclaration();
        if(match({FUNCTION})) return functionDeclaration();
        return statement();
    }
    catch(std::runtime_error& e)
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

sptr(Stmt) Parser::functionDeclaration()
{
    Token name = consume(IDENTIFIER, "Expected function name.");
    consume(OPEN_PAREN, "Expected '(' after function name.");
    
    std::vector<Token> parameters;
    if (!check(CLOSE_PAREN)) {
        do {
            parameters.push_back(consume(IDENTIFIER, "Expected parameter name."));
        } while (match({COMMA}));
    }
    
    consume(CLOSE_PAREN, "Expected ')' after parameters.");
    consume(OPEN_BRACE, "Expected '{' before function body.");
    
    std::vector<sptr(Stmt)> body = block();
    return msptr(FunctionStmt)(name, parameters, body);
}

sptr(Stmt) Parser::statement()
{
    if(match({RETURN})) return returnStatement();
    if(match({OPEN_BRACE})) return msptr(BlockStmt)(block());
    return expressionStatement();
}



sptr(Stmt) Parser::returnStatement()
{
    Token keyword = previous();
    sptr(Expr) value = msptr(LiteralExpr)("none", false, true);
    
    if (!check(SEMICOLON)) {
        value = expression();
    }
    
    consume(SEMICOLON, "Expected ';' after return value.");
    return msptr(ReturnStmt)(keyword, value);
}

sptr(Stmt) Parser::expressionStatement()
{
    sptr(Expr) expr = expression();
    consume(SEMICOLON, "Expected ';' after expression.");
    return msptr(ExpressionStmt)(expr);
}

std::vector<sptr(Stmt)> Parser::block()
{
    std::vector<sptr(Stmt)> statements;

    while(!check(CLOSE_BRACE) && !isAtEnd())
    {
        statements.push_back(declaration());
    }

    consume(CLOSE_BRACE, "Expected '}' after block.");
    return statements;
}

sptr(Expr) Parser::finishCall(sptr(Expr) callee) {
    std::vector<sptr(Expr)> arguments;
    
    // Consume the opening parenthesis
    consume(OPEN_PAREN, "Expected '(' after function name.");
    
    // Parse arguments if there are any
    if (!check(CLOSE_PAREN)) {
        do {
            arguments.push_back(expression());
        } while (match({COMMA}));
    }

    Token paren = consume(CLOSE_PAREN, "Expected ')' after arguments.");
    return msptr(CallExpr)(callee, paren, arguments);
}

bool Parser::match(const std::vector<TokenType>& types) {
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

Token Parser::consume(TokenType type, const std::string& message) {
    if(check(type)) return advance();

    throw std::runtime_error("Unexpected symbol '" + peek().lexeme +"': "+ message);
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


