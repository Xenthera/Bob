
#include "Parser.h"
#include <stdexcept>


// Operator Precedence Rules
// Following standard mathematical order of operations
///////////////////////////////////////////

sptr(Expr) Parser::expression()
{
    return assignment();
}

sptr(Expr) Parser::logical_or()
{
    sptr(Expr) expr = ternary();

    while(match({OR}))
    {
        Token op = previous();
        sptr(Expr) right = ternary();
        expr = msptr(BinaryExpr)(expr, op, right);
    }

    return expr;
}

sptr(Expr) Parser::ternary()
{
    sptr(Expr) expr = logical_and();

    if (match({QUESTION})) {
        sptr(Expr) thenExpr = expression();
        consume(COLON, "Expected ':' after ternary condition");
        sptr(Expr) elseExpr = expression();
        expr = msptr(TernaryExpr)(expr, thenExpr, elseExpr);
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
    // Assignments are now statements, not expressions
    // This function should only handle expressions that are not assignments
    return increment();
}

sptr(Expr) Parser::assignmentExpression()
{
    // This allows assignments as expressions (for for loop increment clauses)
    sptr(Expr) expr = increment();

    if(match({EQUAL, PLUS_EQUAL, MINUS_EQUAL, STAR_EQUAL, SLASH_EQUAL, PERCENT_EQUAL,
              BIN_AND_EQUAL, BIN_OR_EQUAL, BIN_XOR_EQUAL, BIN_SLEFT_EQUAL, BIN_SRIGHT_EQUAL}))
    {
        Token op = previous();
        sptr(Expr) value = assignmentExpression();
        if(std::dynamic_pointer_cast<VarExpr>(expr))
        {
            Token name = std::dynamic_pointer_cast<VarExpr>(expr)->name;
            return msptr(AssignExpr)(name, op, value);
        }
        else if(std::dynamic_pointer_cast<ArrayIndexExpr>(expr))
        {
            auto arrayExpr = std::dynamic_pointer_cast<ArrayIndexExpr>(expr);
            return msptr(ArrayAssignExpr)(arrayExpr->array, arrayExpr->index, value, arrayExpr->bracket);
        }
        else if(std::dynamic_pointer_cast<PropertyExpr>(expr))
        {
            auto propertyExpr = std::dynamic_pointer_cast<PropertyExpr>(expr);
            return msptr(PropertyAssignExpr)(propertyExpr->object, propertyExpr->name, value);
        }
        else if(std::dynamic_pointer_cast<ArrayIndexExpr>(expr))
        {
            auto arrayExpr = std::dynamic_pointer_cast<ArrayIndexExpr>(expr);
            return msptr(ArrayAssignExpr)(arrayExpr->array, arrayExpr->index, value, arrayExpr->bracket);
        }

        
        if (errorReporter) {
            errorReporter->reportError(op.line, op.column, "Parse Error",
                "Invalid assignment target", "");
        }
        throw std::runtime_error("Invalid assignment target.");
    }

    return expr;
}

sptr(Expr) Parser::increment()
{
    return logical_or();
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
    if(match({BANG, MINUS, BIN_NOT, PLUS_PLUS, MINUS_MINUS}))
    {
        Token op = previous();
        sptr(Expr) right = unary();
        
        // Handle prefix increment/decrement
        if (op.type == PLUS_PLUS || op.type == MINUS_MINUS) {
            // Ensure the operand is a variable or array indexing
            if (!std::dynamic_pointer_cast<VarExpr>(right) && 
                !std::dynamic_pointer_cast<ArrayIndexExpr>(right)) {
                if (errorReporter) {
                    errorReporter->reportError(op.line, op.column, "Parse Error", 
                        "Prefix increment/decrement can only be applied to variables or array elements", "");
                }
                throw std::runtime_error("Prefix increment/decrement can only be applied to variables or array elements.");
            }
            return msptr(IncrementExpr)(right, op, true);  // true = prefix
        }
        
        return msptr(UnaryExpr)(op, right);
    }

    return postfix();
}

sptr(Expr) Parser::postfix()
{
    sptr(Expr) expr = primary();

    while (true) {
        if (match({OPEN_PAREN})) {
            expr = finishCall(expr);
            continue;
        }
        if (match({OPEN_BRACKET})) {
            expr = finishArrayIndex(expr);
            continue;
        }
        if (match({DOT})) {
            Token name = consume(IDENTIFIER, "Expected property name after '.'.");
            expr = msptr(PropertyExpr)(expr, name);
            continue;
        }
        if (match({PLUS_PLUS, MINUS_MINUS})) {
            Token oper = previous();
            if (!std::dynamic_pointer_cast<VarExpr>(expr) &&
                !std::dynamic_pointer_cast<ArrayIndexExpr>(expr)) {
                if (errorReporter) {
                    errorReporter->reportError(oper.line, oper.column, "Parse Error",
                        "Postfix increment/decrement can only be applied to variables or array elements", "");
                }
                throw std::runtime_error("Postfix increment/decrement can only be applied to variables or array elements.");
            }
            expr = msptr(IncrementExpr)(expr, oper, false);
            continue;
        }
        break;
    }

    return expr;
}

sptr(Expr) Parser::primary()
{
    if(match({FALSE})) return msptr(LiteralExpr)("false", false, false, true);
    if(match({TRUE})) return msptr(LiteralExpr)("true", false, false, true);
    if(match({NONE})) return msptr(LiteralExpr)("none", false, true, false);

    if(match({NUMBER})) return msptr(LiteralExpr)(previous().lexeme, true, false, false);
    if(match({STRING})) return msptr(LiteralExpr)(previous().lexeme, false, false, false);

    if(match( {IDENTIFIER, THIS, SUPER})) {
        Token ident = previous();
        if (ident.type == THIS) {
            return msptr(VarExpr)(Token{IDENTIFIER, "this", ident.line, ident.column});
        }
        if (ident.type == SUPER) {
            return msptr(VarExpr)(Token{IDENTIFIER, "super", ident.line, ident.column});
        }
        return msptr(VarExpr)(ident);
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

    if(match({OPEN_BRACKET})) {
        return arrayLiteral();
    }

    if(match({OPEN_BRACE})) {
        return dictLiteral();
    }

    if (errorReporter) {
        errorReporter->reportError(peek().line, peek().column, "Parse Error", 
            "Expression expected", "");
    }
    throw std::runtime_error("Expression expected at: " + std::to_string(peek().line));
}

sptr(Expr) Parser::arrayLiteral()
{
    std::vector<sptr(Expr)> elements;
    
    if (!check(CLOSE_BRACKET)) {
        do {
            elements.push_back(expression());
        } while (match({COMMA}));
    }
    
    consume(CLOSE_BRACKET, "Expected ']' after array elements.");
    return msptr(ArrayLiteralExpr)(elements);
}

sptr(Expr) Parser::dictLiteral()
{
    std::vector<std::pair<std::string, sptr(Expr)>> pairs;
    
    if (!check(CLOSE_BRACE)) {
        do {
            // Parse key (must be a string literal)
            if (!match({STRING})) {
                if (errorReporter) {
                    errorReporter->reportError(peek().line, peek().column, "Parse Error", 
                        "Dictionary key must be a string literal", "");
                }
                throw std::runtime_error("Dictionary key must be a string literal");
            }
            std::string key = previous().lexeme;
            
            // Parse colon
            consume(COLON, "Expected ':' after dictionary key");
            
            // Parse value
            sptr(Expr) value = expression();
            
            pairs.emplace_back(key, value);
        } while (match({COMMA}));
    }
    
    consume(CLOSE_BRACE, "Expected '}' after dictionary pairs.");
    return msptr(DictLiteralExpr)(pairs);
}

sptr(Expr) Parser::call()
{
    Token ident = previous();
    sptr(Expr) expr;
    if (ident.type == THIS) {
        expr = msptr(VarExpr)(Token{IDENTIFIER, "this", ident.line, ident.column});
    } else {
        expr = msptr(VarExpr)(ident);
    }
    
    while (true) {
        if (match({OPEN_PAREN})) {
            expr = finishCall(expr);
        } else if (match({OPEN_BRACKET})) {
            expr = finishArrayIndex(expr);
        } else if (match({DOT})) {
            Token name = consume(IDENTIFIER, "Expected property name after '.'.");
            expr = msptr(PropertyExpr)(expr, name);
        } else {
            break;
        }
    }
    
    return expr;
}


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
        if(match({CLASS})) return classDeclaration();
        if(match({EXTENSION})) return extensionDeclaration();
        return statement();
    }
    catch(std::runtime_error& e)
    {
        sync();
        throw std::runtime_error(e.what());
    }
}

sptr(Stmt) Parser::classDeclaration() {
    Token name = consume(IDENTIFIER, "Expected class name.");
    bool hasParent = false;
    Token parentName{};
    if (match({EXTENDS})) {
        hasParent = true;
        parentName = consume(IDENTIFIER, "Expected parent class name after 'extends'.");
    }
    consume(OPEN_BRACE, "Expected '{' after class declaration.");

    std::vector<ClassField> fields;
    std::vector<std::shared_ptr<FunctionStmt>> methods;

    while (!check(CLOSE_BRACE) && !isAtEnd()) {
        if (match({VAR})) {
            Token fieldName = consume(IDENTIFIER, "Expected field name.");
            std::shared_ptr<Expr> init = nullptr;
            if (match({EQUAL})) {
                init = expression();
            }
            consume(SEMICOLON, "Expected ';' after field declaration.");
            fields.emplace_back(fieldName, init);
        } else if (match({FUNCTION})) {
            Token methodName = consume(IDENTIFIER, "Expected method name.");
            consume(OPEN_PAREN, "Expected '(' after method name.");
            std::vector<Token> parameters;
            if (!check(CLOSE_PAREN)) {
                do {
                    parameters.push_back(consume(IDENTIFIER, "Expected parameter name."));
                } while (match({COMMA}));
            }
            consume(CLOSE_PAREN, "Expected ')' after parameters.");
            consume(OPEN_BRACE, "Expected '{' before method body.");
            enterFunction();
            std::vector<std::shared_ptr<Stmt>> body = block();
            exitFunction();
            methods.push_back(msptr(FunctionStmt)(methodName, parameters, body));
        } else {
            if (errorReporter) {
                errorReporter->reportError(peek().line, peek().column, "Parse Error", "Expected 'var' or 'func' in class body", "");
            }
            throw std::runtime_error("Invalid class member");
        }
    }

    consume(CLOSE_BRACE, "Expected '}' after class body.");
    return msptr(ClassStmt)(name, hasParent, parentName, fields, methods);
}

sptr(Stmt) Parser::extensionDeclaration() {
    Token target = consume(IDENTIFIER, "Expected extension target (class/builtin/any).");
    consume(OPEN_BRACE, "Expected '{' after extension target.");
    std::vector<std::shared_ptr<FunctionStmt>> methods;
    while (!check(CLOSE_BRACE) && !isAtEnd()) {
        if (match({FUNCTION})) {
            Token methodName = consume(IDENTIFIER, "Expected method name.");
            consume(OPEN_PAREN, "Expected '(' after method name.");
            std::vector<Token> parameters;
            if (!check(CLOSE_PAREN)) {
                do {
                    parameters.push_back(consume(IDENTIFIER, "Expected parameter name."));
                } while (match({COMMA}));
            }
            consume(CLOSE_PAREN, "Expected ')' after parameters.");
            consume(OPEN_BRACE, "Expected '{' before method body.");
            enterFunction();
            std::vector<std::shared_ptr<Stmt>> body = block();
            exitFunction();
            methods.push_back(msptr(FunctionStmt)(methodName, parameters, body));
        } else {
            if (errorReporter) {
                errorReporter->reportError(peek().line, peek().column, "Parse Error", "Expected 'func' in extension body", "");
            }
            throw std::runtime_error("Invalid extension member");
        }
    }
    consume(CLOSE_BRACE, "Expected '}' after extension body.");
    return msptr(ExtensionStmt)(target, methods);
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
            static const size_t MAX_FUNCTION_PARAMETERS = 255;
            if (parameters.size() >= MAX_FUNCTION_PARAMETERS) {
                if (errorReporter) {
                                errorReporter->reportError(peek().line, 0, "Parse Error", 
                                "Cannot have more than " + std::to_string(MAX_FUNCTION_PARAMETERS) + " parameters", "");
            }
            throw std::runtime_error("Cannot have more than " + std::to_string(MAX_FUNCTION_PARAMETERS) + " parameters.");
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
    if(match({DO})) return doWhileStatement();
    if(match({WHILE})) return whileStatement();
    if(match({FOR})) return forStatement();
    if(match({BREAK})) return breakStatement();
    if(match({CONTINUE})) return continueStatement();
    if(match({OPEN_BRACE})) return msptr(BlockStmt)(block());
    
    // Check for assignment statement - simplified approach
    if(check(IDENTIFIER) || check(THIS)) {
        // Try to parse as assignment expression first
        int currentPos = current;
        try {
            sptr(Expr) expr = assignmentExpression();
            
            // If we successfully parsed an assignment expression, it's an assignment statement
            if(std::dynamic_pointer_cast<AssignExpr>(expr) || 
               std::dynamic_pointer_cast<ArrayAssignExpr>(expr) ||
               std::dynamic_pointer_cast<PropertyAssignExpr>(expr)) {
                consume(SEMICOLON, "Expected ';' after assignment.");
                return msptr(ExpressionStmt)(expr);
            }
            
            // If it's not an assignment, reset and parse as expression statement
            current = currentPos;
        } catch (...) {
            // If assignment parsing failed, reset and parse as expression statement
            current = currentPos;
        }
    }
    
    return expressionStatement();
}

sptr(Stmt) Parser::assignmentStatement()
{
    Token name = consume(IDENTIFIER, "Expected variable name for assignment.");
    
    // Consume any assignment operator
    Token op;
    if(match({EQUAL, PLUS_EQUAL, MINUS_EQUAL, STAR_EQUAL, SLASH_EQUAL, PERCENT_EQUAL,
              BIN_AND_EQUAL, BIN_OR_EQUAL, BIN_XOR_EQUAL, BIN_SLEFT_EQUAL, BIN_SRIGHT_EQUAL})) {
        op = previous();
    } else {
        throw std::runtime_error("Expected assignment operator.");
    }
    
    sptr(Expr) value = expression();
    consume(SEMICOLON, "Expected ';' after assignment.");
    return msptr(AssignStmt)(name, op, value);
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

sptr(Stmt) Parser::whileStatement()
{
    consume(OPEN_PAREN, "Expected '(' after 'while'.");
    sptr(Expr) condition = expression();
    consume(CLOSE_PAREN, "Expected ')' after while condition.");
    
    sptr(Stmt) body = statement();
    
    return msptr(WhileStmt)(condition, body);
}

sptr(Stmt) Parser::doWhileStatement()
{
    sptr(Stmt) body = statement();
    
    consume(WHILE, "Expected 'while' after do-while body.");
    consume(OPEN_PAREN, "Expected '(' after 'while'.");
    sptr(Expr) condition = expression();
    consume(CLOSE_PAREN, "Expected ')' after while condition.");
    consume(SEMICOLON, "Expected ';' after do-while condition.");
    
    return msptr(DoWhileStmt)(body, condition);
}

sptr(Stmt) Parser::forStatement()
{
    consume(OPEN_PAREN, "Expected '(' after 'for'.");
    
    sptr(Stmt) initializer;
    if (match({SEMICOLON})) {
        initializer = nullptr;
    } else if (match({VAR})) {
        initializer = varDeclaration();
    } else {
        // Allow assignment expressions in for loop initializer
        sptr(Expr) expr = assignmentExpression();
        consume(SEMICOLON, "Expected ';' after for loop initializer.");
        initializer = msptr(ExpressionStmt)(expr);
    }
    
    sptr(Expr) condition = nullptr;
    if (!check(SEMICOLON)) {
        condition = expression();
    }
    consume(SEMICOLON, "Expected ';' after for loop condition.");
    
    sptr(Expr) increment = nullptr;
    if (!check(CLOSE_PAREN)) {
        increment = assignmentExpression();
    }
    consume(CLOSE_PAREN, "Expected ')' after for clauses.");
    
    sptr(Stmt) body = statement();
    
    // Return the for statement directly instead of desugaring
    return msptr(ForStmt)(initializer, condition, increment, body);
}

sptr(Stmt) Parser::breakStatement()
{
    Token keyword = previous();
    consume(SEMICOLON, "Expected ';' after 'break'.");
    return msptr(BreakStmt)(keyword);
}

sptr(Stmt) Parser::continueStatement()
{
    Token keyword = previous();
    consume(SEMICOLON, "Expected ';' after 'continue'.");
    return msptr(ContinueStmt)(keyword);
}

// Helper function to detect if an expression is a tail call
bool Parser::isTailCall(const std::shared_ptr<Expr>& expr) {
    // Check if this is a direct function call (no operations on the result)
    if (auto callExpr = std::dynamic_pointer_cast<CallExpr>(expr)) {
        return true;  // Direct function call in return statement
    }
    return false;
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
        
        // Check if this is a tail call and mark it
        if (isTailCall(value)) {
            if (auto callExpr = std::dynamic_pointer_cast<CallExpr>(value)) {
                callExpr->isTailCall = true;
            }
        }
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
    
    // Parse arguments if there are any
    if (!check(CLOSE_PAREN)) {
        do {
            arguments.push_back(expression());
        } while (match({COMMA}));
    }

    Token paren = consume(CLOSE_PAREN, "Expected ')' after arguments.");
    return msptr(CallExpr)(callee, paren, arguments);
}

sptr(Expr) Parser::finishArrayIndex(sptr(Expr) array) {
    sptr(Expr) index = expression();
    Token bracket = consume(CLOSE_BRACKET, "Expected ']' after index.");
    return msptr(ArrayIndexExpr)(array, index, bracket);
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


