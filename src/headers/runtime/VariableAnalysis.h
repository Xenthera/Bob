#pragma once

#include "Expression.h"
#include "Statement.h"
#include <unordered_set>

// Utility functions for analyzing variable usage in AST nodes
namespace VariableAnalysis {
    // Collect all variable names used in a list of statements
    std::unordered_set<std::string> collectUsedVariables(const std::vector<std::shared_ptr<Stmt>>& statements);
    
    // Collect variable names from a single statement
    void collectUsedVariablesFromStmt(const std::shared_ptr<Stmt>& stmt, std::unordered_set<std::string>& usedVars);
    
    // Collect variable names from a single expression
    void collectUsedVariablesFromExpr(const std::shared_ptr<Expr>& expr, std::unordered_set<std::string>& usedVars);
}
