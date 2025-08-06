#pragma once


#include "helperFunctions/ShortHands.h"
#include "TypeWrapper.h"
#include "Expression.h"

struct ExpressionStmt;
struct VarStmt;
struct BlockStmt;
struct FunctionStmt;
struct ReturnStmt;
struct IfStmt;
struct WhileStmt;
struct DoWhileStmt;
struct ForStmt;
struct BreakStmt;
struct ContinueStmt;
struct AssignStmt;

struct ExecutionContext {
    bool isFunctionBody = false;
    bool hasReturn = false;
    bool hasBreak = false;
    bool hasContinue = false;
    Value returnValue;
};

struct StmtVisitor
{
    virtual void visitBlockStmt(const std::shared_ptr<BlockStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitExpressionStmt(const std::shared_ptr<ExpressionStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitVarStmt(const std::shared_ptr<VarStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitFunctionStmt(const std::shared_ptr<FunctionStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitReturnStmt(const std::shared_ptr<ReturnStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitIfStmt(const std::shared_ptr<IfStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitWhileStmt(const std::shared_ptr<WhileStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitDoWhileStmt(const std::shared_ptr<DoWhileStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitForStmt(const std::shared_ptr<ForStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitBreakStmt(const std::shared_ptr<BreakStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitContinueStmt(const std::shared_ptr<ContinueStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitAssignStmt(const std::shared_ptr<AssignStmt>& stmt, ExecutionContext* context = nullptr) = 0;
};

struct Stmt : public std::enable_shared_from_this<Stmt>
{
    std::shared_ptr<Expr> expression;
    virtual void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) = 0;
    virtual ~Stmt(){};
};

struct BlockStmt : Stmt
{
    std::vector<std::shared_ptr<Stmt> > statements;
    explicit BlockStmt(std::vector<std::shared_ptr<Stmt> > statements) : statements(statements)
    {
    }
    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override
    {
        visitor->visitBlockStmt(std::static_pointer_cast<BlockStmt>(shared_from_this()), context);
    }
};

struct ExpressionStmt : Stmt
{
    std::shared_ptr<Expr> expression;
    explicit ExpressionStmt(std::shared_ptr<Expr> expression) : expression(expression)
    {
    }

    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override
    {
        visitor->visitExpressionStmt(std::static_pointer_cast<ExpressionStmt>(shared_from_this()), context);
    }
};



struct VarStmt : Stmt
{
    Token name;
    std::shared_ptr<Expr> initializer;
    VarStmt(Token name, std::shared_ptr<Expr> initializer) : name(name), initializer(initializer)
    {
    }

    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override
    {
        visitor->visitVarStmt(std::static_pointer_cast<VarStmt>(shared_from_this()), context);
    }
};

struct FunctionStmt : Stmt
{
    const Token name;
    const std::vector<Token> params;
    std::vector<std::shared_ptr<Stmt> > body;

    FunctionStmt(Token name, std::vector<Token> params, std::vector<std::shared_ptr<Stmt> > body) 
        : name(name), params(params), body(body) {}

    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override
    {
        visitor->visitFunctionStmt(std::static_pointer_cast<FunctionStmt>(shared_from_this()), context);
    }
};

struct ReturnStmt : Stmt
{
    const Token keyword;
    std::shared_ptr<Expr> value;

    ReturnStmt(Token keyword, std::shared_ptr<Expr> value) : keyword(keyword), value(value) {}

    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override
    {
        visitor->visitReturnStmt(std::static_pointer_cast<ReturnStmt>(shared_from_this()), context);
    }
};

struct IfStmt : Stmt
{
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> thenBranch;
    std::shared_ptr<Stmt> elseBranch;

    IfStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch) 
        : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}

    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override
    {
        visitor->visitIfStmt(std::static_pointer_cast<IfStmt>(shared_from_this()), context);
    }
};

struct WhileStmt : Stmt
{
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> body;

    WhileStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> body) 
        : condition(condition), body(body) {}

    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override
    {
        visitor->visitWhileStmt(std::static_pointer_cast<WhileStmt>(shared_from_this()), context);
    }
};

struct DoWhileStmt : Stmt
{
    std::shared_ptr<Stmt> body;
    std::shared_ptr<Expr> condition;

    DoWhileStmt(std::shared_ptr<Stmt> body, std::shared_ptr<Expr> condition) 
        : body(body), condition(condition) {}

    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override
    {
        visitor->visitDoWhileStmt(std::static_pointer_cast<DoWhileStmt>(shared_from_this()), context);
    }
};

struct ForStmt : Stmt
{
    std::shared_ptr<Stmt> initializer;
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Expr> increment;
    std::shared_ptr<Stmt> body;

    ForStmt(std::shared_ptr<Stmt> initializer, std::shared_ptr<Expr> condition, 
            std::shared_ptr<Expr> increment, std::shared_ptr<Stmt> body) 
        : initializer(initializer), condition(condition), increment(increment), body(body) {}

    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override
    {
        visitor->visitForStmt(std::static_pointer_cast<ForStmt>(shared_from_this()), context);
    }
};

struct BreakStmt : Stmt
{
    const Token keyword;

    BreakStmt(Token keyword) : keyword(keyword) {}

    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override
    {
        visitor->visitBreakStmt(std::static_pointer_cast<BreakStmt>(shared_from_this()), context);
    }
};

struct ContinueStmt : Stmt
{
    const Token keyword;

    ContinueStmt(Token keyword) : keyword(keyword) {}

    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override
    {
        visitor->visitContinueStmt(std::static_pointer_cast<ContinueStmt>(shared_from_this()), context);
    }
};

struct AssignStmt : Stmt
{
    const Token name;
    const Token op;
    std::shared_ptr<Expr> value;

    AssignStmt(Token name, Token op, std::shared_ptr<Expr> value) 
        : name(name), op(op), value(value) {}

    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override
    {
        visitor->visitAssignStmt(std::static_pointer_cast<AssignStmt>(shared_from_this()), context);
    }
};