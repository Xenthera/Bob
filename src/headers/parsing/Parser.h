#pragma once
#include <utility>
#include <vector>
#include "Lexer.h"
#include "Expression.h"
#include "Statement.h"
#include "TypeWrapper.h"
#include "helperFunctions/ShortHands.h"
#include "ErrorReporter.h"

class Parser
{
private:
    const std::vector<Token> tokens;
    int current = 0;
    int functionDepth = 0; // Track nesting level of functions
    ErrorReporter* errorReporter = nullptr;

public:
    explicit Parser(std::vector<Token> tokens) : tokens(std::move(tokens)){};
    std::vector<sptr(Stmt)> parse();
    void setErrorReporter(ErrorReporter* reporter) { errorReporter = reporter; }

private:
    sptr(Expr) expression();
    sptr(Expr) logical_or();
    sptr(Expr) ternary();
    sptr(Expr) logical_and();
    sptr(Expr) bitwise_or();
    sptr(Expr) bitwise_xor();
    sptr(Expr) bitwise_and();
    sptr(Expr) shift();
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

    std::shared_ptr<Stmt> whileStatement();

    std::shared_ptr<Stmt> doWhileStatement();

    std::shared_ptr<Stmt> forStatement();

    std::shared_ptr<Stmt> breakStatement();

    std::shared_ptr<Stmt> continueStatement();

    std::shared_ptr<Stmt> declaration();
    std::shared_ptr<Stmt> classDeclaration();
    std::shared_ptr<Stmt> extensionDeclaration();
    std::shared_ptr<Stmt> tryStatement();
    std::shared_ptr<Stmt> throwStatement();
    std::shared_ptr<Stmt> importStatement();
    std::shared_ptr<Stmt> fromImportStatement();

    std::shared_ptr<Stmt> varDeclaration();

    std::shared_ptr<Stmt> functionDeclaration();
    std::shared_ptr<Expr> functionExpression();

    std::shared_ptr<Stmt> assignmentStatement();
    sptr(Expr) assignment();
    sptr(Expr) assignmentExpression();  // For for loop increment clauses
    sptr(Expr) increment();  // Parse increment/decrement expressions
    sptr(Expr) postfix();    // Parse postfix operators

    std::vector<std::shared_ptr<Stmt>> block();
    
    sptr(Expr) finishCall(sptr(Expr) callee);
    sptr(Expr) finishArrayIndex(sptr(Expr) array);
    sptr(Expr) finishArrayAssign(sptr(Expr) array, sptr(Expr) index, sptr(Expr) value);
    sptr(Expr) finishDictIndex(sptr(Expr) dict);
    sptr(Expr) finishDictAssign(sptr(Expr) dict, sptr(Expr) key, sptr(Expr) value);
    sptr(Expr) arrayLiteral();
    sptr(Expr) dictLiteral();
    sptr(Expr) call();  // Handle call chains (function calls, array indexing, and dict indexing)
    
    // Helper methods for function scope tracking
    void enterFunction() { functionDepth++; }
    void exitFunction() { functionDepth--; }
    bool isInFunction() const { return functionDepth > 0; }
    
    // Helper method for tail call detection
    bool isTailCall(const std::shared_ptr<Expr>& expr);
};