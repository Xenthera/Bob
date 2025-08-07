
#include <utility>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <limits>
#include <unordered_map>
#include <iostream>
#include <chrono>
#include <stdexcept>
#include <algorithm>
#include "../headers/Interpreter.h"
#include "../headers/helperFunctions/HelperFunctions.h"
#include "../headers/BobStdLib.h"

struct ReturnContext {
    Value returnValue;
    bool hasReturn;
    ReturnContext() : returnValue(NONE_VALUE), hasReturn(false) {}
};

// Trampoline-based tail call optimization - no exceptions needed




Value Interpreter::visitLiteralExpr(const std::shared_ptr<LiteralExpr>& expr) {
    if (expr->isNull) return NONE_VALUE;
    if (expr->isNumber) {
        double num;
        if (expr->value[1] == 'b') {
            num = binaryStringToLong(expr->value);
        } else {
            num = std::stod(expr->value);
        }
        return Value(num);
    }
    if (expr->isBoolean) {
        if (expr->value == "true") return TRUE_VALUE;
        if (expr->value == "false") return FALSE_VALUE;
    }
    return Value(expr->value);
}

Value Interpreter::visitGroupingExpr(const std::shared_ptr<GroupingExpr>& expression) {
    return evaluate(expression->expression);
}

Value Interpreter::visitUnaryExpr(const std::shared_ptr<UnaryExpr>& expression)
{
    Value right = evaluate(expression->right);

    switch (expression->oper.type) {
        case MINUS:
            if (!right.isNumber()) {
                if (errorReporter) {
                    errorReporter->reportError(expression->oper.line, expression->oper.column, "Runtime Error", 
                        "Operand must be a number when using: " + expression->oper.lexeme, expression->oper.lexeme);
                }
                throw std::runtime_error("Operand must be a number when using: " + expression->oper.lexeme);
            }
            return Value(-right.asNumber());

        case BANG:
            return Value(!isTruthy(right));

        case BIN_NOT:
            if (!right.isNumber()) {
                if (errorReporter) {
                    errorReporter->reportError(expression->oper.line, expression->oper.column, "Runtime Error", 
                        "Operand must be a number when using: " + expression->oper.lexeme, expression->oper.lexeme);
                }
                throw std::runtime_error("Operand must be a number when using: " + expression->oper.lexeme);
            }
            return Value(static_cast<double>(~(static_cast<long>(right.asNumber()))));

        default:
            if (errorReporter) {
                errorReporter->reportError(expression->oper.line, expression->oper.column, "Runtime Error", 
                    "Invalid unary operator: " + expression->oper.lexeme, expression->oper.lexeme);
            }
            throw std::runtime_error("Invalid unary operator: " + expression->oper.lexeme);
    }
}

Value Interpreter::visitBinaryExpr(const std::shared_ptr<BinaryExpr>& expression) {
    Value left = evaluate(expression->left);
    Value right = evaluate(expression->right);

    // Handle logical operators (AND, OR) - these work with any types
    if (expression->oper.type == AND) {
        return isTruthy(left) ? right : left;
    }
    if (expression->oper.type == OR) {
        return isTruthy(left) ? left : right;
    }

    // Handle equality operators - these work with any types
    if (expression->oper.type == DOUBLE_EQUAL || expression->oper.type == BANG_EQUAL) {
        bool equal = isEqual(left, right);
        return Value(expression->oper.type == DOUBLE_EQUAL ? equal : !equal);
    }

    // Handle comparison operators - only work with numbers
    if (expression->oper.type == GREATER || expression->oper.type == GREATER_EQUAL || 
        expression->oper.type == LESS || expression->oper.type == LESS_EQUAL) {
        
        if (left.isNumber() && right.isNumber()) {
            double leftNum = left.asNumber();
            double rightNum = right.asNumber();
            
            switch (expression->oper.type) {
                case GREATER: return Value(leftNum > rightNum);
                case GREATER_EQUAL: return Value(leftNum >= rightNum);
                case LESS: return Value(leftNum < rightNum);
                case LESS_EQUAL: return Value(leftNum <= rightNum);
            }
        }
        
        // Error for non-number comparisons
        std::string opName;
        switch (expression->oper.type) {
            case GREATER: opName = ">"; break;
            case GREATER_EQUAL: opName = ">="; break;
            case LESS: opName = "<"; break;
            case LESS_EQUAL: opName = "<="; break;
        }
        
        if (errorReporter) {
            errorReporter->reportError(expression->oper.line, expression->oper.column, "Runtime Error", 
                ErrorUtils::makeOperatorError(opName, left.getType(), right.getType()), opName);
        }
        throw std::runtime_error(ErrorUtils::makeOperatorError(opName, left.getType(), right.getType()));
    }

    // Handle all other operators using Value's operator overloads
    try {
        switch (expression->oper.type) {
            case PLUS: return left + right;
            case MINUS: return left - right;
            case STAR: return left * right;
            case SLASH: return left / right;
            case PERCENT: return left % right;
            case BIN_AND: return left & right;
            case BIN_OR: return left | right;
            case BIN_XOR: return left ^ right;
            case BIN_SLEFT: return left << right;
            case BIN_SRIGHT: return left >> right;
            default:
                if (errorReporter) {
                    errorReporter->reportError(expression->oper.line, expression->oper.column, "Runtime Error", 
                        "Unknown operator: " + expression->oper.lexeme, expression->oper.lexeme);
                }
                throw std::runtime_error("Unknown operator: " + expression->oper.lexeme);
        }
    } catch (const std::runtime_error& e) {
        // The Value operators provide good error messages, just add context
        if (errorReporter) {
            errorReporter->reportError(expression->oper.line, expression->oper.column, "Runtime Error", 
                e.what(), expression->oper.lexeme);
        }
        throw;
    }
}

Value Interpreter::visitVarExpr(const std::shared_ptr<VarExpr>& expression)
{
    return environment->get(expression->name);
}

Value Interpreter::visitIncrementExpr(const std::shared_ptr<IncrementExpr>& expression) {
    // Get the current value of the operand
    Value currentValue = evaluate(expression->operand);
    
    if (!currentValue.isNumber()) {
        if (errorReporter) {
            errorReporter->reportError(expression->oper.line, expression->oper.column, 
                "Runtime Error", "Increment/decrement can only be applied to numbers.", "");
        }
        throw std::runtime_error("Increment/decrement can only be applied to numbers.");
    }
    
    double currentNum = currentValue.asNumber();
    double newValue;
    
    // Determine the operation based on the operator
    if (expression->oper.type == PLUS_PLUS) {
        newValue = currentNum + 1.0;
    } else if (expression->oper.type == MINUS_MINUS) {
        newValue = currentNum - 1.0;
    } else {
        if (errorReporter) {
            errorReporter->reportError(expression->oper.line, expression->oper.column, 
                "Runtime Error", "Invalid increment/decrement operator.", "");
        }
        throw std::runtime_error("Invalid increment/decrement operator.");
    }
    
    // Update the variable or array element
    if (auto varExpr = std::dynamic_pointer_cast<VarExpr>(expression->operand)) {
        environment->assign(varExpr->name, Value(newValue));
    } else if (auto arrayExpr = std::dynamic_pointer_cast<ArrayIndexExpr>(expression->operand)) {
        // Handle array indexing increment/decrement
        Value array = evaluate(arrayExpr->array);
        Value index = evaluate(arrayExpr->index);
        
        if (!array.isArray()) {
            if (errorReporter) {
                errorReporter->reportError(expression->oper.line, expression->oper.column, 
                    "Runtime Error", "Can only index arrays", "");
            }
            throw std::runtime_error("Can only index arrays");
        }
        
        if (!index.isNumber()) {
            if (errorReporter) {
                errorReporter->reportError(expression->oper.line, expression->oper.column, 
                    "Runtime Error", "Array index must be a number", "");
            }
            throw std::runtime_error("Array index must be a number");
        }
        
        int idx = static_cast<int>(index.asNumber());
        std::vector<Value>& arr = array.asArray();
        
        if (idx < 0 || idx >= static_cast<int>(arr.size())) {
            if (errorReporter) {
                errorReporter->reportError(arrayExpr->bracket.line, arrayExpr->bracket.column, 
                    "Runtime Error", "Array index out of bounds", "");
            }
            throw std::runtime_error("Array index out of bounds");
        }
        
        // Update the array element
        arr[idx] = Value(newValue);
    } else {
        if (errorReporter) {
            errorReporter->reportError(expression->oper.line, expression->oper.column, 
                "Runtime Error", "Increment/decrement can only be applied to variables or array elements.", "");
        }
        throw std::runtime_error("Increment/decrement can only be applied to variables or array elements.");
    }
    
    // Return the appropriate value based on prefix/postfix
    if (expression->isPrefix) {
            return Value(newValue);  // Prefix: return new value
} else {
    return currentValue;     // Postfix: return old value
}
}

void Interpreter::addStdLibFunctions() {
    // Add standard library functions to the environment
            BobStdLib::addToEnvironment(environment, *this, errorReporter);
}

void Interpreter::addBuiltinFunction(std::shared_ptr<BuiltinFunction> func) {
    builtinFunctions.push_back(func);
}



Value Interpreter::visitAssignExpr(const std::shared_ptr<AssignExpr>& expression) {
    Value value = evaluate(expression->value);
    
    switch (expression->op.type) {
        case PLUS_EQUAL:
        case MINUS_EQUAL:
        case STAR_EQUAL:
        case SLASH_EQUAL:
        case PERCENT_EQUAL:
        case BIN_AND_EQUAL:
        case BIN_OR_EQUAL:
        case BIN_XOR_EQUAL:
        case BIN_SLEFT_EQUAL:
        case BIN_SRIGHT_EQUAL: {
            Value currentValue = environment->get(expression->name);
            
            // Check if the operation is supported before attempting it
            std::string opName;
            switch (expression->op.type) {
                case PLUS_EQUAL: opName = "+="; break;
                case MINUS_EQUAL: opName = "-="; break;
                case STAR_EQUAL: opName = "*="; break;
                case SLASH_EQUAL: opName = "/="; break;
                case PERCENT_EQUAL: opName = "%="; break;
                case BIN_AND_EQUAL: opName = "&="; break;
                case BIN_OR_EQUAL: opName = "|="; break;
                case BIN_XOR_EQUAL: opName = "^="; break;
                case BIN_SLEFT_EQUAL: opName = "<<="; break;
                case BIN_SRIGHT_EQUAL: opName = ">>="; break;
                default: opName = expression->op.lexeme; break;
            }
            
            // Check if the operation is supported for these types
            bool operationSupported = false;
            switch (expression->op.type) {
                case PLUS_EQUAL:
                    operationSupported = (currentValue.isNumber() && value.isNumber()) ||
                                       (currentValue.isString() && value.isString()) ||
                                       (currentValue.isString() && value.isNumber()) ||
                                       (currentValue.isNumber() && value.isString()) ||
                                       (currentValue.isString() && value.isNone()) ||
                                       (currentValue.isNone() && value.isString()) ||
                                       (currentValue.isString() && !value.isString() && !value.isNumber()) ||
                                       (!currentValue.isString() && !currentValue.isNumber() && value.isString());
                    break;
                case MINUS_EQUAL:
                case PERCENT_EQUAL:
                case BIN_AND_EQUAL:
                case BIN_OR_EQUAL:
                case BIN_XOR_EQUAL:
                case BIN_SLEFT_EQUAL:
                case BIN_SRIGHT_EQUAL:
                    operationSupported = currentValue.isNumber() && value.isNumber();
                    break;
                case STAR_EQUAL:
                    operationSupported = (currentValue.isNumber() && value.isNumber()) ||
                                       (currentValue.isString() && value.isNumber()) ||
                                       (currentValue.isNumber() && value.isString());
                    break;
                case SLASH_EQUAL:
                    operationSupported = currentValue.isNumber() && value.isNumber();
                    break;
                default:
                    operationSupported = false;
                    break;
            }
            
            if (!operationSupported) {
                if (errorReporter) {
                    errorReporter->reportError(expression->op.line, expression->op.column, "Runtime Error", 
                        ErrorUtils::makeOperatorError(opName, currentValue.getType(), value.getType()), 
                        expression->op.lexeme);
                }
                throw std::runtime_error(ErrorUtils::makeOperatorError(opName, currentValue.getType(), value.getType()));
            }
            
            // Perform the operation
            switch (expression->op.type) {
                case PLUS_EQUAL:
                    value = currentValue + value;
                    break;
                case MINUS_EQUAL:
                    value = currentValue - value;
                    break;
                case STAR_EQUAL:
                    value = currentValue * value;
                    break;
                case SLASH_EQUAL:
                    value = currentValue / value;
                    break;
                case PERCENT_EQUAL:
                    value = currentValue % value;
                    break;
                case BIN_AND_EQUAL:
                    value = currentValue & value;
                    break;
                case BIN_OR_EQUAL:
                    value = currentValue | value;
                    break;
                case BIN_XOR_EQUAL:
                    value = currentValue ^ value;
                    break;
                case BIN_SLEFT_EQUAL:
                    value = currentValue << value;
                    break;
                case BIN_SRIGHT_EQUAL:
                    value = currentValue >> value;
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
    environment->assign(expression->name, value);
    return value;
}

Value Interpreter::visitTernaryExpr(const std::shared_ptr<TernaryExpr>& expression) {
    Value condition = evaluate(expression->condition);
    
    if (isTruthy(condition)) {
        return evaluate(expression->thenExpr);
    } else {
        return evaluate(expression->elseExpr);
    }
}

Value Interpreter::visitCallExpr(const std::shared_ptr<CallExpr>& expression) {
    Value callee = evaluate(expression->callee);
    
    std::vector<Value> arguments;
    for (const std::shared_ptr<Expr>& argument : expression->arguments) {
        arguments.push_back(evaluate(argument));
    }
    
    if (callee.isBuiltinFunction()) {
        // Builtin functions now work directly with Value and receive line and column
        return callee.asBuiltinFunction()->func(arguments, expression->paren.line, expression->paren.column);
    }
    
    if (callee.isFunction()) {
        Function* function = callee.asFunction();
        if (arguments.size() != function->params.size()) {
            std::string errorMsg = "Expected " + std::to_string(function->params.size()) +
                                   " arguments but got " + std::to_string(arguments.size()) + ".";
            if (errorReporter) {
                errorReporter->reportError(expression->paren.line, expression->paren.column, "Runtime Error",
                    errorMsg, "");
            }
            throw std::runtime_error(errorMsg);
        }
        
        // Check if this is a tail call
        if (expression->isTailCall) {
            // Create a thunk for tail call optimization using smart pointer
            auto thunk = std::make_shared<Thunk>([this, function, arguments]() -> Value {
                // Use RAII to manage environment
                ScopedEnv _env(environment);
                environment = std::make_shared<Environment>(function->closure);
                environment->setErrorReporter(errorReporter);
                
                for (size_t i = 0; i < function->params.size(); i++) {
                    environment->define(function->params[i], arguments[i]);
                }
                
                ExecutionContext context;
                context.isFunctionBody = true;
                
                // Use RAII to manage thunk execution flag
                ScopedThunkFlag _inThunk(inThunkExecution);
                
                // Execute function body
                for (const auto& stmt : function->body) {
                    execute(stmt, &context);
                    if (context.hasReturn) {
                        return context.returnValue;
                    }
                }
                
                return context.returnValue;
            });
            
            // Store the thunk to keep it alive and return as Value
            thunks.push_back(thunk);
            
            // Automatic cleanup check
            thunkCreationCount++;
            if (thunkCreationCount >= CLEANUP_THRESHOLD) {
                cleanupUnusedThunks();
                thunkCreationCount = 0;
            }
            
            return Value(thunk.get());
        } else {
            // Normal function call - create new environment
            ScopedEnv _env(environment);
            environment = std::make_shared<Environment>(function->closure);
            environment->setErrorReporter(errorReporter);
            
            for (size_t i = 0; i < function->params.size(); i++) {
                environment->define(function->params[i], arguments[i]);
            }
            
            ExecutionContext context;
            context.isFunctionBody = true;
            
            // Execute function body
            for (const auto& stmt : function->body) {
                execute(stmt, &context);
                if (context.hasReturn) {
                    return context.returnValue;
                }
            }
            
            return context.returnValue;
        }
    }
    
    // Provide better error message with type information
    std::string errorMsg = "Can only call functions, got " + callee.getType();
    if (errorReporter) {
        errorReporter->reportError(expression->paren.line, expression->paren.column, "Runtime Error",
            errorMsg, "");
    }
    throw std::runtime_error(errorMsg);
}

Value Interpreter::visitArrayLiteralExpr(const std::shared_ptr<ArrayLiteralExpr>& expr) {
    std::vector<Value> elements;
    
    for (const auto& element : expr->elements) {
        elements.push_back(evaluate(element));
    }
    
    return Value(elements);
}

Value Interpreter::visitArrayIndexExpr(const std::shared_ptr<ArrayIndexExpr>& expr) {
    Value array = evaluate(expr->array);
    Value index = evaluate(expr->index);
    
    if (!array.isArray()) {
        if (errorReporter) {
            errorReporter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                "Can only index arrays", "");
        }
        throw std::runtime_error("Can only index arrays");
    }
    
    if (!index.isNumber()) {
        if (errorReporter) {
            errorReporter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                "Array index must be a number", "");
        }
        throw std::runtime_error("Array index must be a number");
    }
    
    int idx = static_cast<int>(index.asNumber());
    const std::vector<Value>& arr = array.asArray();
    

    
    if (idx < 0 || idx >= arr.size()) {
        if (errorReporter) {
            errorReporter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                "Array index out of bounds", "");
        }
        throw std::runtime_error("Array index out of bounds");
    }
    
    return arr[idx];
}

Value Interpreter::visitArrayAssignExpr(const std::shared_ptr<ArrayAssignExpr>& expr) {
    Value array = evaluate(expr->array);
    Value index = evaluate(expr->index);
    Value value = evaluate(expr->value);
    
    if (!array.isArray()) {
        if (errorReporter) {
            errorReporter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                "Can only assign to arrays", "");
        }
        throw std::runtime_error("Can only assign to arrays");
    }
    
    if (!index.isNumber()) {
        if (errorReporter) {
            errorReporter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                "Array index must be a number", "");
        }
        throw std::runtime_error("Array index must be a number");
    }
    
    int idx = static_cast<int>(index.asNumber());
    std::vector<Value>& arr = array.asArray();
    
    if (idx < 0 || idx >= arr.size()) {
        if (errorReporter) {
            errorReporter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                "Array index out of bounds", "");
        }
        throw std::runtime_error("Array index out of bounds");
    }
    
    arr[idx] = value;
    return value;
}

Value Interpreter::visitFunctionExpr(const std::shared_ptr<FunctionExpr>& expression) {
    // Convert Token parameters to string parameters
    std::vector<std::string> paramNames;
    for (const Token& param : expression->params) {
        paramNames.push_back(param.lexeme);
    }
    
    // Create a snapshot of the current environment for proper closure behavior
    auto closureEnv = std::make_shared<Environment>(*environment);
    closureEnv->setErrorReporter(errorReporter);
    
    auto function = msptr(Function)("anonymous", paramNames, expression->body, closureEnv);
    functions.push_back(function); // Keep the shared_ptr alive
    
    // Automatic cleanup check
    functionCreationCount++;
    if (functionCreationCount >= CLEANUP_THRESHOLD) {
        cleanupUnusedFunctions();
        functionCreationCount = 0;
    }
    
    return Value(function.get());
}

void Interpreter::visitBlockStmt(const std::shared_ptr<BlockStmt>& statement, ExecutionContext* context) {
    auto newEnv = std::make_shared<Environment>(environment);
    newEnv->setErrorReporter(errorReporter);
    executeBlock(statement->statements, newEnv, context);
}

void Interpreter::visitExpressionStmt(const std::shared_ptr<ExpressionStmt>& statement, ExecutionContext* context) {
    Value value = evaluate(statement->expression);

    if(IsInteractive)
        std::cout << "\u001b[38;5;8m[" << stringify(value) << "]\u001b[38;5;15m" << std::endl;
}



void Interpreter::visitVarStmt(const std::shared_ptr<VarStmt>& statement, ExecutionContext* context)
{
    Value value = NONE_VALUE;
    if(statement->initializer != nullptr)
    {
        value = evaluate(statement->initializer);
    }

    //std::cout << "Visit var stmt: " << statement->name.lexeme << " set to: " << stringify(value) << std::endl;

    environment->define(statement->name.lexeme, value);
}

void Interpreter::visitFunctionStmt(const std::shared_ptr<FunctionStmt>& statement, ExecutionContext* context)
{
    // Convert Token parameters to string parameters
    std::vector<std::string> paramNames;
    for (const Token& param : statement->params) {
        paramNames.push_back(param.lexeme);
    }
    
    // For named functions, use the current environment (not a snapshot)
    // This allows mutual recursion and forward references
    auto function = msptr(Function)(statement->name.lexeme, 
                                   paramNames, 
                                   statement->body, 
                                   environment);
    functions.push_back(function); // Keep the shared_ptr alive
    environment->define(statement->name.lexeme, Value(function.get()));
    
    // Automatic cleanup check
    functionCreationCount++;
    if (functionCreationCount >= CLEANUP_THRESHOLD) {
        cleanupUnusedFunctions();
        functionCreationCount = 0;
    }
}

void Interpreter::visitReturnStmt(const std::shared_ptr<ReturnStmt>& statement, ExecutionContext* context)
{
    Value value = NONE_VALUE;
    if (statement->value != nullptr) {
        // For tail calls, the trampoline handling is done in visitCallExpr
        // We just need to evaluate normally
        value = evaluate(statement->value);
    }
    
    if (context && context->isFunctionBody) {
        context->hasReturn = true;
        context->returnValue = value;
    }
    // If no context or not in function body, this is a top-level return (ignored)
}

void Interpreter::visitIfStmt(const std::shared_ptr<IfStmt>& statement, ExecutionContext* context)
{
    if (isTruthy(evaluate(statement->condition))) {
        execute(statement->thenBranch, context);
    } else if (statement->elseBranch != nullptr) {
        execute(statement->elseBranch, context);
    }
}

void Interpreter::visitWhileStmt(const std::shared_ptr<WhileStmt>& statement, ExecutionContext* context)
{
    ExecutionContext loopContext;
    if (context) {
        loopContext.isFunctionBody = context->isFunctionBody;
    }
    
    while (isTruthy(evaluate(statement->condition))) {
        execute(statement->body, &loopContext);
        
        // Check for return from function
        if (loopContext.hasReturn) {
            if (context) {
                context->hasReturn = true;
                context->returnValue = loopContext.returnValue;
            }
            break;
        }
        
        // Check for break
        if (loopContext.hasBreak) {
            loopContext.hasBreak = false;
            break;
        }
        
        // Check for continue (just continue to next iteration)
        if (loopContext.hasContinue) {
            loopContext.hasContinue = false;
            continue;
        }
    }
}

void Interpreter::visitDoWhileStmt(const std::shared_ptr<DoWhileStmt>& statement, ExecutionContext* context)
{
    ExecutionContext loopContext;
    if (context) {
        loopContext.isFunctionBody = context->isFunctionBody;
    }
    
    do {
        execute(statement->body, &loopContext);
        
        // Check for return from function
        if (loopContext.hasReturn) {
            if (context) {
                context->hasReturn = true;
                context->returnValue = loopContext.returnValue;
            }
            break;
        }
        
        // Check for break
        if (loopContext.hasBreak) {
            loopContext.hasBreak = false;
            break;
        }
        
        // Check for continue (just continue to next iteration)
        if (loopContext.hasContinue) {
            loopContext.hasContinue = false;
            continue;
        }
    } while (isTruthy(evaluate(statement->condition)));
}

void Interpreter::visitForStmt(const std::shared_ptr<ForStmt>& statement, ExecutionContext* context)
{
    // For loops are desugared into while loops in the parser
    // This method should never be called, but we implement it for completeness
    // The actual execution happens through the desugared while loop
    if (statement->initializer != nullptr) {
        execute(statement->initializer, context);
    }
    
    ExecutionContext loopContext;
    if (context) {
        loopContext.isFunctionBody = context->isFunctionBody;
    }
    
    while (statement->condition == nullptr || isTruthy(evaluate(statement->condition))) {
        execute(statement->body, &loopContext);
        
        // Check for return from function
        if (loopContext.hasReturn) {
            if (context) {
                context->hasReturn = true;
                context->returnValue = loopContext.returnValue;
            }
            break;
        }
        
        // Check for break
        if (loopContext.hasBreak) {
            loopContext.hasBreak = false;
            break;
        }
        
        // Check for continue (execute increment then continue to next iteration)
        if (loopContext.hasContinue) {
            loopContext.hasContinue = false;
            if (statement->increment != nullptr) {
                evaluate(statement->increment);
            }
            continue;
        }
        
        if (statement->increment != nullptr) {
            evaluate(statement->increment);
        }
    }
}

void Interpreter::visitBreakStmt(const std::shared_ptr<BreakStmt>& statement, ExecutionContext* context)
{
    if (context) {
        context->hasBreak = true;
    }
}

void Interpreter::visitContinueStmt(const std::shared_ptr<ContinueStmt>& statement, ExecutionContext* context)
{
    if (context) {
        context->hasContinue = true;
    }
}

void Interpreter::visitAssignStmt(const std::shared_ptr<AssignStmt>& statement, ExecutionContext* context)
{
    Value value = evaluate(statement->value);
    
    // Handle different assignment operators
    if (statement->op.type == EQUAL) {
        // Check if the variable previously held an array
        Value oldValue = environment->get(statement->name.lexeme);
        if (oldValue.isArray()) {
            forceCleanup(); // Clean up when breaking array references
        }
        
        // Simple assignment
        environment->assign(statement->name, value);
    } else {
        // Compound assignment - get current value first
        Value currentValue = environment->get(statement->name.lexeme);
        
        // Check if the operation is supported before attempting it
        std::string opName;
        switch (statement->op.type) {
            case PLUS_EQUAL: opName = "+="; break;
            case MINUS_EQUAL: opName = "-="; break;
            case STAR_EQUAL: opName = "*="; break;
            case SLASH_EQUAL: opName = "/="; break;
            case PERCENT_EQUAL: opName = "%="; break;
            case BIN_AND_EQUAL: opName = "&="; break;
            case BIN_OR_EQUAL: opName = "|="; break;
            case BIN_XOR_EQUAL: opName = "^="; break;
            case BIN_SLEFT_EQUAL: opName = "<<="; break;
            case BIN_SRIGHT_EQUAL: opName = ">>="; break;
            default: opName = statement->op.lexeme; break;
        }
        
        // Check if the operation is supported for these types
        bool operationSupported = false;
        switch (statement->op.type) {
            case PLUS_EQUAL:
                operationSupported = (currentValue.isNumber() && value.isNumber()) ||
                                   (currentValue.isString() && value.isString()) ||
                                   (currentValue.isString() && value.isNumber()) ||
                                   (currentValue.isNumber() && value.isString()) ||
                                   (currentValue.isString() && value.isNone()) ||
                                   (currentValue.isNone() && value.isString()) ||
                                   (currentValue.isString() && !value.isString() && !value.isNumber()) ||
                                   (!currentValue.isString() && !currentValue.isNumber() && value.isString());
                break;
            case MINUS_EQUAL:
            case PERCENT_EQUAL:
            case BIN_AND_EQUAL:
            case BIN_OR_EQUAL:
            case BIN_XOR_EQUAL:
            case BIN_SLEFT_EQUAL:
            case BIN_SRIGHT_EQUAL:
                operationSupported = currentValue.isNumber() && value.isNumber();
                break;
            case STAR_EQUAL:
                operationSupported = (currentValue.isNumber() && value.isNumber()) ||
                                   (currentValue.isString() && value.isNumber()) ||
                                   (currentValue.isNumber() && value.isString());
                break;
            case SLASH_EQUAL:
                operationSupported = currentValue.isNumber() && value.isNumber();
                break;
            default:
                operationSupported = false;
                break;
        }
        
        if (!operationSupported) {
            if (errorReporter) {
                errorReporter->reportError(statement->op.line, statement->op.column, "Runtime Error", 
                    ErrorUtils::makeOperatorError(opName, currentValue.getType(), value.getType()), 
                    statement->op.lexeme);
            }
            throw std::runtime_error(ErrorUtils::makeOperatorError(opName, currentValue.getType(), value.getType()));
        }
        
        // Apply the compound operation
        Value result;
        if (statement->op.type == PLUS_EQUAL) {
            result = currentValue + value;
        } else if (statement->op.type == MINUS_EQUAL) {
            result = currentValue - value;
        } else if (statement->op.type == STAR_EQUAL) {
            result = currentValue * value;
        } else if (statement->op.type == SLASH_EQUAL) {
            result = currentValue / value;
        } else if (statement->op.type == PERCENT_EQUAL) {
            result = currentValue % value;
        } else if (statement->op.type == BIN_AND_EQUAL) {
            result = currentValue & value;
        } else if (statement->op.type == BIN_OR_EQUAL) {
            result = currentValue | value;
        } else if (statement->op.type == BIN_XOR_EQUAL) {
            result = currentValue ^ value;
        } else if (statement->op.type == BIN_SLEFT_EQUAL) {
            result = currentValue << value;
        } else if (statement->op.type == BIN_SRIGHT_EQUAL) {
            result = currentValue >> value;
        } else {
            throw std::runtime_error("Unknown assignment operator: " + statement->op.lexeme);
        }
        
        environment->assign(statement->name, result);
    }
}

void Interpreter::interpret(std::vector<std::shared_ptr<Stmt>> statements) {
    for(const std::shared_ptr<Stmt>& s : statements)
    {
        execute(s, nullptr); // No context needed for top-level execution
    }
}

void Interpreter::execute(const std::shared_ptr<Stmt>& statement, ExecutionContext* context)
{
    statement->accept(this, context);
}

void Interpreter::executeBlock(std::vector<std::shared_ptr<Stmt>> statements, std::shared_ptr<Environment> env, ExecutionContext* context)
{
    std::shared_ptr<Environment> previous = this->environment;
    this->environment = env;

    for(const std::shared_ptr<Stmt>& s : statements)
    {
        execute(s, context);
        if (context && (context->hasReturn || context->hasBreak || context->hasContinue)) {
            this->environment = previous;
            return;
        }
    }

    this->environment = previous;
}

Value Interpreter::evaluate(const std::shared_ptr<Expr>& expr) {
    Value result = expr->accept(this);
    if (inThunkExecution) {
        return result; // Don't use trampoline when inside a thunk
    }
    return runTrampoline(result);
}

Value Interpreter::evaluateWithoutTrampoline(const std::shared_ptr<Expr>& expr) {
    return expr->accept(this);
}

Value Interpreter::runTrampoline(Value initialResult) {
    Value current = initialResult;
    
    while (current.isThunk()) {
        // Execute the thunk to get the next result
        current = current.asThunk()->execute();
    }
    
    return current;
}

bool Interpreter::isTruthy(Value object) {

    if(object.isBoolean())
    {
        return object.asBoolean();
    }

    if(object.isNone())
    {
        return false;
    }

    if(object.isNumber())
    {
        return object.asNumber() != 0;
    }

    if(object.isString())
    {
        return object.asString().length() > 0;
    }

    return true;
}

bool Interpreter::isEqual(Value a, Value b) {
    // Handle none comparisons first
    if (a.isNone() || b.isNone()) {
        return a.isNone() && b.isNone();
    }
    
    // Handle same type comparisons
    if (a.isNumber() && b.isNumber()) {
        return a.asNumber() == b.asNumber();
    }
    
    if (a.isBoolean() && b.isBoolean()) {
        return a.asBoolean() == b.asBoolean();
    }
    
    if (a.isString() && b.isString()) {
        return a.asString() == b.asString();
    }
    
    if (a.isArray() && b.isArray()) {
        const std::vector<Value>& arrA = a.asArray();
        const std::vector<Value>& arrB = b.asArray();
        
        if (arrA.size() != arrB.size()) {
            return false;
        }
        
        for (size_t i = 0; i < arrA.size(); i++) {
            if (!isEqual(arrA[i], arrB[i])) {
                return false;
            }
        }
        return true;
    }
    
    if (a.isFunction() && b.isFunction()) {
        // Functions are equal only if they are the same object
        return a.asFunction() == b.asFunction();
    }
    
    if (a.isBuiltinFunction() && b.isBuiltinFunction()) {
        // Builtin functions are equal only if they are the same object
        return a.asBuiltinFunction() == b.asBuiltinFunction();
    }
    
    // Cross-type comparisons that make sense
    if (a.isNumber() && b.isBoolean()) {
        // Numbers and booleans: 0 and false are equal, non-zero and true are equal
        if (b.asBoolean()) {
            return a.asNumber() != 0.0;
        } else {
            return a.asNumber() == 0.0;
        }
    }
    
    if (a.isBoolean() && b.isNumber()) {
        // Same as above, but reversed
        if (a.asBoolean()) {
            return b.asNumber() != 0.0;
        } else {
            return b.asNumber() == 0.0;
        }
    }
    
    // For all other type combinations, return false
    return false;
}

std::string Interpreter::stringify(Value object) {
    if(object.isNone())
    {
        return "none";
    }
    else if(object.isNumber())
    {
        double integral = object.asNumber();
        double fractional = std::modf(object.asNumber(), &integral);

        std::stringstream ss;
        if(std::abs(fractional) < std::numeric_limits<double>::epsilon())
        {
            ss << std::fixed << std::setprecision(0) << integral;
            return ss.str();
        }
        else
        {
            ss << std::fixed << std::setprecision(std::numeric_limits<double>::digits10 - 1) << object.asNumber();
            std::string str = ss.str();
            str.erase(str.find_last_not_of('0') + 1, std::string::npos);
            if (str.back() == '.') {
                str.pop_back();
            }

            return str;
        }
    }
    else if(object.isString())
    {
        return object.asString();
    }
    else if(object.isBoolean())
    {
        return object.asBoolean() == 1 ? "true" : "false";
    }
    else if(object.isFunction())
    {
        return "<function " + object.asFunction()->name + ">";
    }
    else if(object.isBuiltinFunction())
    {
        return "<builtin_function " + object.asBuiltinFunction()->name + ">";
    }
    else if(object.isArray())
    {
        const std::vector<Value>& arr = object.asArray();
        std::string result = "[";
        
        for (size_t i = 0; i < arr.size(); i++) {
            if (i > 0) result += ", ";
            result += stringify(arr[i]);
        }
        
        result += "]";
        return result;
    }

    throw std::runtime_error("Could not convert object to string");
}



void Interpreter::cleanupUnusedFunctions() {
    // Only remove functions that are definitely not referenced anywhere (use_count == 1)
    // This is more conservative to prevent dangling pointer issues
    functions.erase(
        std::remove_if(functions.begin(), functions.end(),
            [](const std::shared_ptr<Function>& func) {
                return func.use_count() == 1;  // Only referenced by this vector, nowhere else
            }),
        functions.end()
    );
}

void Interpreter::cleanupUnusedThunks() {
    // Only remove thunks that are definitely not referenced anywhere (use_count == 1)
    // This is more conservative to prevent dangling pointer issues
    thunks.erase(
        std::remove_if(thunks.begin(), thunks.end(),
            [](const std::shared_ptr<Thunk>& thunk) {
                return thunk.use_count() == 1;  // Only referenced by this vector, nowhere else
            }),
        thunks.end()
    );
}

void Interpreter::forceCleanup() {
    // More aggressive cleanup when breaking array references
    functions.erase(
        std::remove_if(functions.begin(), functions.end(),
            [](const std::shared_ptr<Function>& func) {
                return func.use_count() <= 2; // More aggressive than == 1
            }),
        functions.end()
    );
    
    thunks.erase(
        std::remove_if(thunks.begin(), thunks.end(),
            [](const std::shared_ptr<Thunk>& thunk) {
                return thunk.use_count() <= 2; // More aggressive than == 1
            }),
        thunks.end()
    );
}

















