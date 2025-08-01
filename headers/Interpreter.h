#pragma once
#include "Expression.h"
#include "Statement.h"
#include "helperFunctions/ShortHands.h"
#include "TypeWrapper.h"
#include "Environment.h"
#include "Value.h"
#include "StdLib.h"
#include "ErrorReporter.h"

#include <vector>
#include <memory>
#include <unordered_map>
#include <stack>

class Interpreter : public ExprVisitor, public StmtVisitor {

public:
    Value visitBinaryExpr(const std::shared_ptr<BinaryExpr>& expression) override;
    Value visitCallExpr(const std::shared_ptr<CallExpr>& expression) override;
    Value visitFunctionExpr(const std::shared_ptr<FunctionExpr>& expression) override;
    Value visitGroupingExpr(const std::shared_ptr<GroupingExpr>& expression) override;
    Value visitLiteralExpr(const std::shared_ptr<LiteralExpr>& expression) override;
    Value visitUnaryExpr(const std::shared_ptr<UnaryExpr>& expression) override;
    Value visitVarExpr(const std::shared_ptr<VarExpr>& expression) override;
    Value visitAssignExpr(const std::shared_ptr<AssignExpr>& expression) override;

    void visitBlockStmt(const std::shared_ptr<BlockStmt>& statement) override;
    void visitExpressionStmt(const std::shared_ptr<ExpressionStmt>& statement) override;
    void visitVarStmt(const std::shared_ptr<VarStmt>& statement) override;
    void visitFunctionStmt(const std::shared_ptr<FunctionStmt>& statement) override;
    void visitReturnStmt(const std::shared_ptr<ReturnStmt>& statement) override;
    void visitIfStmt(const std::shared_ptr<IfStmt>& statement) override;

    void interpret(std::vector<std::shared_ptr<Stmt> > statements);

    explicit Interpreter(bool IsInteractive) : IsInteractive(IsInteractive), errorReporter(nullptr){
        environment = std::make_shared<Environment>();
    }
    virtual ~Interpreter() = default;

private:
    std::shared_ptr<Environment> environment;
    bool IsInteractive;
    std::vector<std::shared_ptr<BuiltinFunction> > builtinFunctions;
    std::vector<std::shared_ptr<Function> > functions;
    ErrorReporter* errorReporter;
    
    Value evaluate(const std::shared_ptr<Expr>& expr);
    bool isEqual(Value a, Value b);
    bool isWholeNumer(double num);
    void execute(const std::shared_ptr<Stmt>& statement);
    void executeBlock(std::vector<std::shared_ptr<Stmt> > statements, std::shared_ptr<Environment> env);
    void addStdLibFunctions();
    
public:
    bool isTruthy(Value object);
    std::string stringify(Value object);
    void addBuiltinFunction(std::shared_ptr<BuiltinFunction> func);

    // Error reporting
    void setErrorReporter(ErrorReporter* reporter) { 
        errorReporter = reporter; 
        if (environment) {
            environment->setErrorReporter(reporter);
        }
        
        // Add standard library functions after error reporter is set
        addStdLibFunctions();
    }
};
