#pragma once
#include "Expression.h"
#include "Statement.h"
#include <memory>
#include <vector>

// Optimizer that detects and optimizes common performance patterns
class Optimizer {
public:
    // Optimize a statement list for performance
    static std::vector<std::shared_ptr<Stmt>> optimize(const std::vector<std::shared_ptr<Stmt>>& statements);
    
private:
    // Detect and optimize simple loops
    static std::shared_ptr<Stmt> optimizeForLoop(const std::shared_ptr<ForStmt>& stmt);
    
    // Detect and optimize simple arithmetic expressions
    static std::shared_ptr<Expr> optimizeArithmetic(const std::shared_ptr<BinaryExpr>& expr);
};
