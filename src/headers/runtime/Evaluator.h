#pragma once

#include "Expression.h"
#include "Value.h"

class Interpreter; // Forward declaration for the back-pointer

/**
 * @class Evaluator
 * @brief Handles the logic for visiting and evaluating Expression AST nodes.
 *
 * Implements the Visitor pattern for all Expression types. This class
 * contains the core evaluation logic for expressions, returning a Value.
 */
class Evaluator : public ExprVisitor {
private:
    Interpreter* interpreter; // Back-pointer to access interpreter services like isTruthy, etc.

public:
    explicit Evaluator(Interpreter* interpreter);
    virtual ~Evaluator() = default;

    // Expression Visitors
    Value visitBinaryExpr(const std::shared_ptr<BinaryExpr>& expression) override;
    Value visitCallExpr(const std::shared_ptr<CallExpr>& expression) override;
    Value visitFunctionExpr(const std::shared_ptr<FunctionExpr>& expression) override;
    Value visitGroupingExpr(const std::shared_ptr<GroupingExpr>& expression) override;
    Value visitLiteralExpr(const std::shared_ptr<LiteralExpr>& expression) override;
    Value visitUnaryExpr(const std::shared_ptr<UnaryExpr>& expression) override;
    Value visitVarExpr(const std::shared_ptr<VarExpr>& expression) override;
    Value visitIncrementExpr(const std::shared_ptr<IncrementExpr>& expression) override;
    Value visitAssignExpr(const std::shared_ptr<AssignExpr>& expression) override;
    Value visitTernaryExpr(const std::shared_ptr<TernaryExpr>& expression) override;
    Value visitArrayLiteralExpr(const std::shared_ptr<ArrayLiteralExpr>& expression) override;
    Value visitArrayIndexExpr(const std::shared_ptr<ArrayIndexExpr>& expression) override;
    Value visitPropertyExpr(const std::shared_ptr<PropertyExpr>& expression) override;
    Value visitArrayAssignExpr(const std::shared_ptr<ArrayAssignExpr>& expression) override;
    Value visitPropertyAssignExpr(const std::shared_ptr<PropertyAssignExpr>& expression) override;
    Value visitDictLiteralExpr(const std::shared_ptr<DictLiteralExpr>& expression) override;

private:
    // Helper methods for builtin properties
    Value getArrayProperty(const Value& array, const std::string& propertyName);
    Value getDictProperty(const Value& dict, const std::string& propertyName);
    

};
