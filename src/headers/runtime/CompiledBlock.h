#pragma once
#include "OptimizedValue.h"
#include <vector>
#include <functional>
#include <memory>

// Compiled block that executes multiple statements efficiently
class CompiledBlock {
public:
    using StatementExecutor = std::function<void()>;
    
    void addStatement(StatementExecutor exec) {
        statements.push_back(std::move(exec));
    }
    
    void execute() {
        for (const auto& stmt : statements) {
            stmt();
        }
    }
    
private:
    std::vector<StatementExecutor> statements;
};

// Compiler for statement blocks
class BlockCompiler {
public:
    static std::unique_ptr<CompiledBlock> compile(const std::vector<std::shared_ptr<Stmt>>& statements);
    
private:
    // Compile specific statement types
    static StatementExecutor compileForLoop(const std::shared_ptr<ForStmt>& stmt);
    static StatementExecutor compileExpression(const std::shared_ptr<ExpressionStmt>& stmt);
    static StatementExecutor compileVarDecl(const std::shared_ptr<VarStmt>& stmt);
};
