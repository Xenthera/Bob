#pragma once
#include "OptimizedValue.h"
#include "OptimizedEnvironment.h"
#include "CompiledExpr.h"
#include "CompiledBlock.h"
#include <memory>

// Fast interpreter that uses compiled expressions and optimized values
class FastInterpreter {
public:
    FastInterpreter() : environment(std::make_unique<OptimizedEnvironment>()) {}
    
    // Execute compiled expressions
    OptimizedValue evaluate(const CompiledExpr& expr) {
        return expr.execute();
    }
    
    // Execute compiled blocks
    void execute(const CompiledBlock& block) {
        block.execute();
    }
    
    // Fast variable operations
    void defineVariable(const std::string& name, const OptimizedValue& value) {
        environment->define(name, value);
    }
    
    OptimizedValue getVariable(const std::string& name) const {
        return environment->get(name);
    }
    
    void setVariable(const std::string& name, const OptimizedValue& value) {
        environment->assign(name, value);
    }
    
    // Compile and execute (one-shot)
    OptimizedValue compileAndExecute(const std::shared_ptr<Expr>& expr) {
        auto compiled = ExprCompiler::compile(expr);
        return compiled->execute();
    }
    
private:
    std::unique_ptr<OptimizedEnvironment> environment;
};
