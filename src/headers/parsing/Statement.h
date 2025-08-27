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
struct ForeachStmt;
struct BreakStmt;
struct ContinueStmt;
struct AssignStmt;
struct ClassStmt;
struct ExtensionStmt;

#include "ExecutionContext.h"

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
    virtual void visitForeachStmt(const std::shared_ptr<ForeachStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitBreakStmt(const std::shared_ptr<BreakStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitContinueStmt(const std::shared_ptr<ContinueStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitAssignStmt(const std::shared_ptr<AssignStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitClassStmt(const std::shared_ptr<ClassStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitExtensionStmt(const std::shared_ptr<ExtensionStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitTryStmt(const std::shared_ptr<struct TryStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitThrowStmt(const std::shared_ptr<struct ThrowStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitImportStmt(const std::shared_ptr<struct ImportStmt>& stmt, ExecutionContext* context = nullptr) = 0;
    virtual void visitFromImportStmt(const std::shared_ptr<struct FromImportStmt>& stmt, ExecutionContext* context = nullptr) = 0;
};

struct Stmt : public std::enable_shared_from_this<Stmt>
{
    std::shared_ptr<Expr> expression;
    virtual void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) = 0;
    virtual ~Stmt(){};
};

struct ClassField {
    Token name;
    std::shared_ptr<Expr> initializer; // may be null
    ClassField(Token name, std::shared_ptr<Expr> init) : name(name), initializer(init) {}
};

struct ClassStmt : Stmt {
    const Token name;
    bool hasParent;
    Token parentName; // valid only if hasParent
    std::vector<ClassField> fields;
    std::vector<std::shared_ptr<FunctionStmt>> methods;

    ClassStmt(Token name, bool hasParent, Token parentName, std::vector<ClassField> fields, std::vector<std::shared_ptr<FunctionStmt>> methods)
        : name(name), hasParent(hasParent), parentName(parentName), fields(std::move(fields)), methods(std::move(methods)) {}

    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override {
        visitor->visitClassStmt(std::static_pointer_cast<ClassStmt>(shared_from_this()), context);
    }
};

struct ExtensionStmt : Stmt {
    const Token target;
    std::vector<std::shared_ptr<FunctionStmt>> methods;

    ExtensionStmt(Token target, std::vector<std::shared_ptr<FunctionStmt>> methods)
        : target(target), methods(std::move(methods)) {}

    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override {
        visitor->visitExtensionStmt(std::static_pointer_cast<ExtensionStmt>(shared_from_this()), context);
    }
};

struct BlockStmt : Stmt
{
    std::vector<std::shared_ptr<Stmt>> statements;
    explicit BlockStmt(std::vector<std::shared_ptr<Stmt>> statements) : statements(statements)
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
    std::vector<std::shared_ptr<Stmt>> body;

    FunctionStmt(Token name, std::vector<Token> params, std::vector<std::shared_ptr<Stmt>> body) 
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

struct ForeachStmt : Stmt
{
    Token varName;  // Variable name for the current element
    std::shared_ptr<Expr> collection;  // Array or dict to iterate over
    std::shared_ptr<Stmt> body;

    ForeachStmt(Token varName, std::shared_ptr<Expr> collection, std::shared_ptr<Stmt> body)
        : varName(varName), collection(collection), body(body) {}

    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override
    {
        visitor->visitForeachStmt(std::static_pointer_cast<ForeachStmt>(shared_from_this()), context);
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

struct TryStmt : Stmt {
    std::shared_ptr<Stmt> tryBlock;
    Token catchVar; // IDENTIFIER or empty token if no catch
    std::shared_ptr<Stmt> catchBlock; // may be null
    std::shared_ptr<Stmt> finallyBlock; // may be null

    TryStmt(std::shared_ptr<Stmt> t, Token cvar, std::shared_ptr<Stmt> cblk, std::shared_ptr<Stmt> fblk)
        : tryBlock(t), catchVar(cvar), catchBlock(cblk), finallyBlock(fblk) {}

    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override {
        visitor->visitTryStmt(std::static_pointer_cast<TryStmt>(shared_from_this()), context);
    }
};

struct ThrowStmt : Stmt {
    const Token keyword;
    std::shared_ptr<Expr> value;
    ThrowStmt(Token kw, std::shared_ptr<Expr> v) : keyword(kw), value(v) {}
    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override {
        visitor->visitThrowStmt(std::static_pointer_cast<ThrowStmt>(shared_from_this()), context);
    }
};

// import module [as alias]
struct ImportStmt : Stmt {
    Token importToken; // IMPORT
    Token moduleName;  // IDENTIFIER
    bool hasAlias = false;
    Token alias;       // IDENTIFIER if hasAlias
    ImportStmt(Token kw, Token mod, bool ha, Token al)
        : importToken(kw), moduleName(mod), hasAlias(ha), alias(al) {}
    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override {
        visitor->visitImportStmt(std::static_pointer_cast<ImportStmt>(shared_from_this()), context);
    }
};

// from module import name [as alias], name2 ...
struct FromImportStmt : Stmt {
    Token fromToken;   // FROM
    Token moduleName;  // IDENTIFIER or STRING
    struct ImportItem { Token name; bool hasAlias; Token alias; };
    std::vector<ImportItem> items;
    bool importAll = false; // true for: from module import *;
    FromImportStmt(Token kw, Token mod, std::vector<ImportItem> it)
        : fromToken(kw), moduleName(mod), items(std::move(it)), importAll(false) {}
    FromImportStmt(Token kw, Token mod, bool all)
        : fromToken(kw), moduleName(mod), importAll(all) {}
    void accept(StmtVisitor* visitor, ExecutionContext* context = nullptr) override {
        visitor->visitFromImportStmt(std::static_pointer_cast<FromImportStmt>(shared_from_this()), context);
    }
};