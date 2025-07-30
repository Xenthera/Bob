#pragma once
#include "Expression.h"
#include "Statement.h"
#include "helperFunctions/ShortHands.h"
#include "TypeWrapper.h"
#include "Environment.h"
#include "StdLib.h"

class Return : public std::exception {
public:
    sptr(Object) value;
    
    Return(sptr(Object) value) : value(value) {}
    
    const char* what() const noexcept override {
        return "Return";
    }
};

class Interpreter : ExprVisitor, StmtVisitor
{

public:
    sptr(Object) visitBinaryExpr(sptr(BinaryExpr) expression) override;
    sptr(Object) visitGroupingExpr(sptr(GroupingExpr) expression) override;
    sptr(Object) visitLiteralExpr(sptr(LiteralExpr) expression) override;
    sptr(Object) visitUnaryExpr(sptr(UnaryExpr) expression) override;
    sptr(Object) visitVariableExpr(sptr(VarExpr) expression) override;
    sptr(Object) visitAssignExpr(sptr(AssignExpr) expression) override;
    sptr(Object) visitCallExpr(sptr(CallExpr) expression) override;

    void visitBlockStmt(sptr(BlockStmt) statement) override;
    void visitExpressionStmt(sptr(ExpressionStmt) statement) override;

    void visitVarStmt(sptr(VarStmt) statement) override;
    void visitFunctionStmt(sptr(FunctionStmt) statement) override;
    void visitReturnStmt(sptr(ReturnStmt) statement) override;

    void interpret(std::vector<sptr(Stmt)> statements);

    explicit Interpreter(bool IsInteractive) : IsInteractive(IsInteractive){
        environment = msptr(Environment)();
        
        // Add standard library functions
        addStdLibFunctions();
    }
    virtual ~Interpreter() = default;

private:

    sptr(Environment) environment;
    bool IsInteractive;

    sptr(Object) evaluate(sptr(Expr) expr);
    bool isTruthy(sptr(Object) object);
    bool isEqual(sptr(Object) a, sptr(Object) b);
    bool isWholeNumer(double num);
    void execute(sptr(Stmt) statement);
    void executeBlock(std::vector<sptr(Stmt)> statements, sptr(Environment) env);
    void addStdLibFunctions();
    
public:
    std::string stringify(sptr(Object) object);
};
