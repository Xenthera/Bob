#pragma once
#include "Value.h"
#include <functional>
#include <memory>

// Compiled expression that bypasses virtual function overhead
class CompiledExpr {
public:
    using Executor = std::function<Value()>;
    
    CompiledExpr(Executor exec) : executor(std::move(exec)) {}
    
    Value execute() const { return executor(); }
    
private:
    Executor executor;
};

// Compiler that converts AST expressions to compiled form
class ExprCompiler {
public:
    static std::unique_ptr<CompiledExpr> compile(const std::shared_ptr<Expr>& expr);
    
private:
    // Compile specific expression types
    static std::unique_ptr<CompiledExpr> compileLiteral(const std::shared_ptr<LiteralExpr>& expr);
    static std::unique_ptr<CompiledExpr> compileBinary(const std::shared_ptr<BinaryExpr>& expr);
    static std::unique_ptr<CompiledExpr> compileVar(const std::shared_ptr<VarExpr>& expr);
};
