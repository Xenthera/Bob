#include "VariableAnalysis.h"

namespace VariableAnalysis {

std::unordered_set<std::string> collectUsedVariables(const std::vector<std::shared_ptr<Stmt>>& statements) {
    std::unordered_set<std::string> usedVars;
    
    for (const auto& stmt : statements) {
        collectUsedVariablesFromStmt(stmt, usedVars);
    }
    
    return usedVars;
}

void collectUsedVariablesFromStmt(const std::shared_ptr<Stmt>& stmt, std::unordered_set<std::string>& usedVars) {
    if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStmt>(stmt)) {
        collectUsedVariablesFromExpr(exprStmt->expression, usedVars);
    } else if (auto varStmt = std::dynamic_pointer_cast<VarStmt>(stmt)) {
        collectUsedVariablesFromExpr(varStmt->initializer, usedVars);
    } else if (auto returnStmt = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
        collectUsedVariablesFromExpr(returnStmt->value, usedVars);
    } else if (auto ifStmt = std::dynamic_pointer_cast<IfStmt>(stmt)) {
        collectUsedVariablesFromExpr(ifStmt->condition, usedVars);
        collectUsedVariablesFromStmt(ifStmt->thenBranch, usedVars);
        if (ifStmt->elseBranch) {
            collectUsedVariablesFromStmt(ifStmt->elseBranch, usedVars);
        }
    } else if (auto whileStmt = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
        collectUsedVariablesFromExpr(whileStmt->condition, usedVars);
        collectUsedVariablesFromStmt(whileStmt->body, usedVars);
    } else if (auto forStmt = std::dynamic_pointer_cast<ForStmt>(stmt)) {
        if (forStmt->initializer) {
            collectUsedVariablesFromStmt(forStmt->initializer, usedVars);
        }
        if (forStmt->condition) {
            collectUsedVariablesFromExpr(forStmt->condition, usedVars);
        }
        if (forStmt->increment) {
            collectUsedVariablesFromExpr(forStmt->increment, usedVars);
        }
        collectUsedVariablesFromStmt(forStmt->body, usedVars);
    } else if (auto foreachStmt = std::dynamic_pointer_cast<ForeachStmt>(stmt)) {
        collectUsedVariablesFromExpr(foreachStmt->collection, usedVars);
        collectUsedVariablesFromStmt(foreachStmt->body, usedVars);
    } else if (auto blockStmt = std::dynamic_pointer_cast<BlockStmt>(stmt)) {
        for (const auto& blockStmt : blockStmt->statements) {
            collectUsedVariablesFromStmt(blockStmt, usedVars);
        }
    } else if (auto tryStmt = std::dynamic_pointer_cast<TryStmt>(stmt)) {
        collectUsedVariablesFromStmt(tryStmt->tryBlock, usedVars);
        if (tryStmt->catchBlock) {
            collectUsedVariablesFromStmt(tryStmt->catchBlock, usedVars);
        }
        if (tryStmt->finallyBlock) {
            collectUsedVariablesFromStmt(tryStmt->finallyBlock, usedVars);
        }
    } else if (auto throwStmt = std::dynamic_pointer_cast<ThrowStmt>(stmt)) {
        collectUsedVariablesFromExpr(throwStmt->value, usedVars);
    }
    // Note: Function declarations, class declarations, etc. don't need variable collection
    // as they define new scopes and don't reference outer variables
}

void collectUsedVariablesFromExpr(const std::shared_ptr<Expr>& expr, std::unordered_set<std::string>& usedVars) {
    if (!expr) return;
    
    if (auto varExpr = std::dynamic_pointer_cast<VarExpr>(expr)) {
        usedVars.insert(varExpr->name.lexeme);
    } else if (auto binaryExpr = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        collectUsedVariablesFromExpr(binaryExpr->left, usedVars);
        collectUsedVariablesFromExpr(binaryExpr->right, usedVars);
    } else if (auto unaryExpr = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        collectUsedVariablesFromExpr(unaryExpr->right, usedVars);
    } else if (auto callExpr = std::dynamic_pointer_cast<CallExpr>(expr)) {
        collectUsedVariablesFromExpr(callExpr->callee, usedVars);
        for (const auto& arg : callExpr->arguments) {
            collectUsedVariablesFromExpr(arg, usedVars);
        }
    } else if (auto arrayIndexExpr = std::dynamic_pointer_cast<ArrayIndexExpr>(expr)) {
        collectUsedVariablesFromExpr(arrayIndexExpr->array, usedVars);
        collectUsedVariablesFromExpr(arrayIndexExpr->index, usedVars);
    } else if (auto assignExpr = std::dynamic_pointer_cast<AssignExpr>(expr)) {
        collectUsedVariablesFromExpr(assignExpr->value, usedVars);
    } else if (auto arrayAssignExpr = std::dynamic_pointer_cast<ArrayAssignExpr>(expr)) {
        collectUsedVariablesFromExpr(arrayAssignExpr->array, usedVars);
        collectUsedVariablesFromExpr(arrayAssignExpr->index, usedVars);
        collectUsedVariablesFromExpr(arrayAssignExpr->value, usedVars);
    } else if (auto propertyAssignExpr = std::dynamic_pointer_cast<PropertyAssignExpr>(expr)) {
        collectUsedVariablesFromExpr(propertyAssignExpr->object, usedVars);
        collectUsedVariablesFromExpr(propertyAssignExpr->value, usedVars);
    } else if (auto incrementExpr = std::dynamic_pointer_cast<IncrementExpr>(expr)) {
        collectUsedVariablesFromExpr(incrementExpr->operand, usedVars);
    } else if (auto groupingExpr = std::dynamic_pointer_cast<GroupingExpr>(expr)) {
        collectUsedVariablesFromExpr(groupingExpr->expression, usedVars);
    }
    // LiteralExpr doesn't reference variables, so no need to handle it
}

} // namespace VariableAnalysis
