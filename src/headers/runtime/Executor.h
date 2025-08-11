#pragma once

#include "Statement.h"

class Evaluator; // Forward declaration
class Interpreter; // Forward declaration

/**
 * @class Executor
 * @brief Handles the execution of statements and control flow.
 *
 * Implements the StmtVisitor pattern. It is responsible for executing statements,
 * managing environments, and handling control flow constructs like loops and
 * conditionals. It uses the Evaluator to evaluate expressions when needed.
 */
class Executor : public StmtVisitor {
private:
    Interpreter* interpreter; // Back-pointer to access interpreter services
    Evaluator* evaluator;     // For evaluating expressions

public:
    Executor(Interpreter* interpreter, Evaluator* evaluator);
    virtual ~Executor();

    void interpret(const std::vector<std::shared_ptr<Stmt>>& statements);
    void executeBlock(const std::vector<std::shared_ptr<Stmt>>& statements, std::shared_ptr<Environment> env, ExecutionContext* context);

    // Statement Visitors
    void visitBlockStmt(const std::shared_ptr<BlockStmt>& statement, ExecutionContext* context = nullptr) override;
    void visitExpressionStmt(const std::shared_ptr<ExpressionStmt>& statement, ExecutionContext* context = nullptr) override;
    void visitVarStmt(const std::shared_ptr<VarStmt>& statement, ExecutionContext* context = nullptr) override;
    void visitFunctionStmt(const std::shared_ptr<FunctionStmt>& statement, ExecutionContext* context = nullptr) override;
    void visitReturnStmt(const std::shared_ptr<ReturnStmt>& statement, ExecutionContext* context = nullptr) override;
    void visitIfStmt(const std::shared_ptr<IfStmt>& statement, ExecutionContext* context = nullptr) override;
    void visitWhileStmt(const std::shared_ptr<WhileStmt>& statement, ExecutionContext* context = nullptr) override;
    void visitDoWhileStmt(const std::shared_ptr<DoWhileStmt>& statement, ExecutionContext* context = nullptr) override;
    void visitForStmt(const std::shared_ptr<ForStmt>& statement, ExecutionContext* context = nullptr) override;
    void visitBreakStmt(const std::shared_ptr<BreakStmt>& statement, ExecutionContext* context = nullptr) override;
    void visitContinueStmt(const std::shared_ptr<ContinueStmt>& statement, ExecutionContext* context = nullptr) override;
    void visitAssignStmt(const std::shared_ptr<AssignStmt>& statement, ExecutionContext* context = nullptr) override;
    void visitClassStmt(const std::shared_ptr<ClassStmt>& statement, ExecutionContext* context = nullptr) override;
    void visitExtensionStmt(const std::shared_ptr<ExtensionStmt>& statement, ExecutionContext* context = nullptr) override;
    void visitTryStmt(const std::shared_ptr<TryStmt>& statement, ExecutionContext* context = nullptr) override;
    void visitThrowStmt(const std::shared_ptr<ThrowStmt>& statement, ExecutionContext* context = nullptr) override;

private:
    void execute(const std::shared_ptr<Stmt>& statement, ExecutionContext* context);
};