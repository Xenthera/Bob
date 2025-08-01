//
// Created by Bobby Lucero on 5/26/23.
//
#include "../headers/Parser.h"
#include <stdexcept>


//              Precedence
// to all the morons on facebook who don't know what pemdas is, fuck you
///////////////////////////////////////////

sptr(Expr) Parser::expression()
{
    return assignment();
}

sptr(Expr) Parser::logical_or()
{
    sptr(Expr) expr = logical_and();

    while(match({OR}))
    {
        Token op = previous();
        sptr(Expr) right = logical_and();
        expr = msptr(BinaryExpr)(expr, op, right);
    }

    return expr;
}

sptr(Expr) Parser::logical_and()
{
    sptr(Expr) expr = equality();

    while(match({AND}))
    {
        Token op = previous();
        sptr(Expr) right = equality();
        expr = msptr(BinaryExpr)(expr, op, right);
    }

    return expr;
}

// bitwise_or now calls comparison (not bitwise_xor)
sptr(Expr) Parser::bitwise_or()
{
    sptr(Expr) expr = bitwise_xor();

    while(match({BIN_OR}))
    {
        Token op = previous();
        sptr(Expr) right = bitwise_xor();
        expr = msptr(BinaryExpr)(expr, op, right);
    }

    return expr;
}

sptr(Expr) Parser::bitwise_xor()
{
    sptr(Expr) expr = bitwise_and();

    while(match({BIN_XOR}))
    {
        Token op = previous();
        sptr(Expr) right = bitwise_and();
        expr = msptr(BinaryExpr)(expr, op, right);
    }

    return expr;
}

sptr(Expr) Parser::bitwise_and()
{
    sptr(Expr) expr = shift();

    while(match({BIN_AND}))
    {
        Token op = previous();
        sptr(Expr) right = shift();
        expr = msptr(BinaryExpr)(expr, op, right);
    }

    return expr;
}

sptr(Expr) Parser::shift()
{
    sptr(Expr) expr = term();

    while(match({BIN_SLEFT, BIN_SRIGHT}))
    {
        Token op = previous();
        sptr(Expr) right = term();
        expr = msptr(BinaryExpr)(expr, op, right);
    }

    return expr;
}

sptr(Expr) Parser::assignment()
{
    sptr(Expr) expr = logical_or();

    if(match({EQUAL, PLUS_EQUAL, MINUS_EQUAL, STAR_EQUAL, SLASH_EQUAL, PERCENT_EQUAL,
              BIN_AND_EQUAL, BIN_OR_EQUAL, BIN_XOR_EQUAL, BIN_SLEFT_EQUAL, BIN_SRIGHT_EQUAL}))
    {
        Token op = previous();
        sptr(Expr) value = assignment();
        if(std::dynamic_pointer_cast<VarExpr>(expr))
        {
            Token name = std::dynamic_pointer_cast<VarExpr>(expr)->name;
            return msptr(AssignExpr)(name, op, value);
        }
        
        if (errorReporter) {
            errorReporter->reportError(op.line, op.column, "Parse Error",
                "Invalid assignment target", "");
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
    sptr(Expr) expr = bitwise_or();

    while(match({GREATER, GREATER_EQUAL, LESS, LESS_EQUAL}))
    {
        Token op = previous();
        sptr(Expr) right = bitwise_or();
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
    if(match({FALSE})) return msptr(LiteralExpr)("false", false, false, true);
    if(match({TRUE})) return msptr(LiteralExpr)("true", false, false, true);
    if(match({NONE})) return msptr(LiteralExpr)("none", false, true, false);

    if(match({NUMBER})) return msptr(LiteralExpr)(previous().lexeme, true, false, false);
    if(match({STRING})) return msptr(LiteralExpr)(previous().lexeme, false, false, false);

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

    if(match({FUNCTION})) {
        return functionExpression();
    }

    if (errorReporter) {
        errorReporter->reportError(peek().line, peek().column, "Parse Error", 
            "Expression expected", "");
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

    sptr(Expr) initializer = msptr(LiteralExpr)("none", false, true, false);
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
    
    // Enter function scope
    enterFunction();
    
    std::vector<sptr(Stmt)> body = block();
    
    // Exit function scope
    exitFunction();
    
    return msptr(FunctionStmt)(name, parameters, body);
}

std::shared_ptr<Expr> Parser::functionExpression() {
    consume(OPEN_PAREN, "Expect '(' after 'func'.");
    std::vector<Token> parameters;
    if (!check(CLOSE_PAREN)) {
        do {
            if (parameters.size() >= 255) {
                if (errorReporter) {
                                errorReporter->reportError(peek().line, 0, "Parse Error", 
                "Cannot have more than 255 parameters", "");
                }
                throw std::runtime_error("Cannot have more than 255 parameters.");
            }
            parameters.push_back(consume(IDENTIFIER, "Expect parameter name."));
        } while (match({COMMA}));
    }
    consume(CLOSE_PAREN, "Expect ')' after parameters.");
    consume(OPEN_BRACE, "Expect '{' before function body.");
    
    // Enter function scope
    enterFunction();
    
    std::vector<std::shared_ptr<Stmt>> body = block();
    
    // Exit function scope
    exitFunction();
    
    return msptr(FunctionExpr)(parameters, body);
}

sptr(Stmt) Parser::statement()
{
    if(match({RETURN})) return returnStatement();
    if(match({IF})) return ifStatement();
    if(match({OPEN_BRACE})) return msptr(BlockStmt)(block());
    return expressionStatement();
}



sptr(Stmt) Parser::ifStatement()
{
    consume(OPEN_PAREN, "Expected '(' after 'if'.");
    sptr(Expr) condition = expression();
    consume(CLOSE_PAREN, "Expected ')' after if condition.");
    
    sptr(Stmt) thenBranch = statement();
    sptr(Stmt) elseBranch = nullptr;
    
    if (match({ELSE})) {
        elseBranch = statement();
    }
    
    return msptr(IfStmt)(condition, thenBranch, elseBranch);
}

sptr(Stmt) Parser::returnStatement()
{
    Token keyword = previous();
    
    // Check if we're inside a function
    if (!isInFunction()) {
        if (errorReporter) {
            errorReporter->reportError(keyword.line, 0, "Parse Error", 
                "Cannot return from outside a function", "");
        }
        throw std::runtime_error("Cannot return from outside a function");
    }
    
    sptr(Expr) value = msptr(LiteralExpr)("none", false, true, false);
    
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

    if (errorReporter) {
        // Use the precise column information from the token
        int errorColumn = peek().column;
        
        // For missing closing parenthesis, point to where it should be
        if (type == CLOSE_PAREN) {
            // The closing parenthesis should be right after the previous token
            errorColumn = previous().column + previous().lexeme.length();
            
            // For string tokens, add 2 to account for the opening and closing quotes
            if (previous().type == STRING) {
                errorColumn += 2;
            }
        }
        
        errorReporter->reportError(peek().line, errorColumn, "Parse Error", 
            "Unexpected symbol '" + peek().lexeme + "': " + message, "");
    }
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


