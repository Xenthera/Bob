//
// Created by Bobby Lucero on 5/27/23.
//
#include <utility>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <limits>
#include <cmath>
#include "../headers/Interpreter.h"
#include "../headers/helperFunctions/HelperFunctions.h"
#include <unordered_map>
#include "../headers/Interpreter.h"
#include "../headers/StdLib.h"
#include <iostream>
#include <chrono>
#include <cmath>
#include <stdexcept>
#include <algorithm>

struct ReturnContext {
    Value returnValue;
    bool hasReturn;
    ReturnContext() : returnValue(NONE_VALUE), hasReturn(false) {}
};

// Trampoline-based tail call optimization - no exceptions needed




Value Interpreter::visitLiteralExpr(const std::shared_ptr<LiteralExpr>& expr) {
    if(expr->isNull) return NONE_VALUE;
    if(expr->isNumber){
        double num;
        if(expr->value[1] == 'b')
        {
            num = binaryStringToLong(expr->value);
        }
        else
        {
            num = std::stod(expr->value);
        }
        return Value(num);
    }
    if(expr->isBoolean) {
        if(expr->value == "true") return TRUE_VALUE;
        if(expr->value == "false") return FALSE_VALUE;
    }
    return Value(expr->value);
}

Value Interpreter::visitGroupingExpr(const std::shared_ptr<GroupingExpr>& expression) {

    return evaluate(expression->expression);
}

Value Interpreter::visitUnaryExpr(const std::shared_ptr<UnaryExpr>& expression)
{
    Value right = evaluate(expression->right);

    if(expression->oper.type == MINUS)
    {
        if(right.isNumber())
        {
            double value = right.asNumber();
            return Value(-value);
        }
        else
        {
            throw std::runtime_error("Operand must be a number when using: " + expression->oper.lexeme);
        }

    }

    if(expression->oper.type == BANG)
    {
        return Value(!isTruthy(right));
    }

    if(expression->oper.type == BIN_NOT)
    {
        if(right.isNumber())
        {
            double value = right.asNumber();
            return Value(static_cast<double>(~(static_cast<long>(value))));
        }
        else
        {
            throw std::runtime_error("Operand must be an int when using: " + expression->oper.lexeme);
        }
    }

    //unreachable
    throw std::runtime_error("Invalid unary expression");

}

Value Interpreter::visitBinaryExpr(const std::shared_ptr<BinaryExpr>& expression) {
    Value left = evaluate(expression->left);
    Value right = evaluate(expression->right);

    if (left.isNumber() && right.isNumber()) {
        double leftNum = left.asNumber();
        double rightNum = right.asNumber();

        switch (expression->oper.type) {
            case PLUS: return Value(leftNum + rightNum);
            case MINUS: return Value(leftNum - rightNum);
            case SLASH: {
                if (rightNum == 0) {
                    if (errorReporter) {
                        errorReporter->reportError(expression->oper.line, expression->oper.column, "Division by Zero", 
                            "Cannot divide by zero", expression->oper.lexeme);
                    }
                    throw std::runtime_error("Division by zero");
                }
                return Value(leftNum / rightNum);
            }
            case STAR: return Value(leftNum * rightNum);
            case PERCENT: {
                if (rightNum == 0) {
                    if (errorReporter) {
                        errorReporter->reportError(expression->oper.line, expression->oper.column, "Modulo by Zero", 
                            "Cannot perform modulo operation with zero", expression->oper.lexeme);
                    }
                    throw std::runtime_error("Modulo by zero");
                }
                return Value(std::fmod(leftNum, rightNum));
            }
            case GREATER: return Value(leftNum > rightNum);
            case GREATER_EQUAL: return Value(leftNum >= rightNum);
            case LESS: return Value(leftNum < rightNum);
            case LESS_EQUAL: return Value(leftNum <= rightNum);
            case DOUBLE_EQUAL: return Value(leftNum == rightNum);
            case BANG_EQUAL: return Value(leftNum != rightNum);
            case BIN_AND: return Value(static_cast<double>(static_cast<int>(leftNum) & static_cast<int>(rightNum)));
            case BIN_OR: return Value(static_cast<double>(static_cast<int>(leftNum) | static_cast<int>(rightNum)));
            case BIN_XOR: return Value(static_cast<double>(static_cast<int>(leftNum) ^ static_cast<int>(rightNum)));
            case BIN_SLEFT: return Value(static_cast<double>(static_cast<int>(leftNum) << static_cast<int>(rightNum)));
            case BIN_SRIGHT: return Value(static_cast<double>(static_cast<int>(leftNum) >> static_cast<int>(rightNum)));
            case AND: {
                if (!isTruthy(left)) {
                    return left; // Return the falsy value
                } else {
                    return right; // Return the second value
                }
            }
            case OR: {
                if (isTruthy(left)) {
                    return left; // Return the truthy value
                } else {
                    return right; // Return the second value
                }
            }
        }
    }

    if (left.isString() && right.isString()) {
        std::string left_string = left.asString();
        std::string right_string = right.asString();
        
        switch (expression->oper.type) {
            case PLUS: return Value(left_string + right_string);
            case DOUBLE_EQUAL: return Value(left_string == right_string);
            case BANG_EQUAL: return Value(left_string != right_string);
            case AND: {
                if (!isTruthy(left)) {
                    return left; // Return the falsy value
                } else {
                    return right; // Return the second value
                }
            }
            case OR: {
                if (isTruthy(left)) {
                    return left; // Return the truthy value
                } else {
                    return right; // Return the second value
                }
            }
            default:
                if (errorReporter) {
                    errorReporter->reportError(expression->oper.line, expression->oper.column, "Runtime Error", 
                        "Cannot use '" + expression->oper.lexeme + "' on two strings", expression->oper.lexeme);
                }
                throw std::runtime_error("Cannot use '" + expression->oper.lexeme + "' on two strings");
        }
    }

    if (left.isString() && right.isNumber()) {
        std::string left_string = left.asString();
        double right_num = right.asNumber();
        
        switch (expression->oper.type) {
            case PLUS: return left + right;
            case STAR: {
                if (!isWholeNumer(right_num)) {
                    if (errorReporter) {
                        errorReporter->reportError(expression->oper.line, expression->oper.column, "Invalid String Multiplication", 
                            "String multiplier must be a whole number", expression->oper.lexeme);
                    }
                    throw std::runtime_error("String multiplier must be whole number");
                }
                std::string result;
                for (int i = 0; i < static_cast<int>(right_num); i++) {
                    result += left_string;
                }
                return Value(result);
            }
        }
    }

    if (left.isNumber() && right.isString()) {
        double left_num = left.asNumber();
        std::string right_string = right.asString();
        
        switch (expression->oper.type) {
            case PLUS: return left + right;
            case STAR: {
                if (!isWholeNumer(left_num)) {
                    if (errorReporter) {
                        errorReporter->reportError(expression->oper.line, expression->oper.column, "Invalid String Multiplication", 
                            "String multiplier must be a whole number", expression->oper.lexeme);
                    }
                    throw std::runtime_error("String multiplier must be whole number");
                }
                std::string result;
                for (int i = 0; i < static_cast<int>(left_num); i++) {
                    result += right_string;
                }
                return Value(result);
            }
        }
    }

    if (left.isBoolean() && right.isBoolean()) {
        bool left_bool = left.asBoolean();
        bool right_bool = right.asBoolean();
        
        switch (expression->oper.type) {
            case AND: return Value(left_bool && right_bool);
            case OR: return Value(left_bool || right_bool);
            case DOUBLE_EQUAL: return Value(left_bool == right_bool);
            case BANG_EQUAL: return Value(left_bool != right_bool);
        }
    }



    if (left.isBoolean() && right.isString()) {
        bool left_bool = left.asBoolean();
        std::string right_string = right.asString();
        
        switch (expression->oper.type) {
            case PLUS: return left + right;
        }
    }

    if (left.isString() && right.isBoolean()) {
        std::string left_string = left.asString();
        bool right_bool = right.asBoolean();
        
        switch (expression->oper.type) {
            case PLUS: return left + right;
        }
    }

    if (left.isNumber() && right.isBoolean()) {
        double left_num = left.asNumber();
        bool right_bool = right.asBoolean();
        
        switch (expression->oper.type) {
            case AND: {
                if (!isTruthy(left)) {
                    return left; // Return the falsy value
                } else {
                    return right; // Return the second value
                }
            }
            case OR: {
                if (isTruthy(left)) {
                    return left; // Return the truthy value
                } else {
                    return right; // Return the second value
                }
            }
        }
    }

    if (left.isBoolean() && right.isNumber()) {
        bool left_bool = left.asBoolean();
        double right_num = right.asNumber();
        
        switch (expression->oper.type) {
            case AND: {
                if (!isTruthy(left)) {
                    return left; // Return the falsy value
                } else {
                    return right; // Return the second value
                }
            }
            case OR: {
                if (isTruthy(left)) {
                    return left; // Return the truthy value
                } else {
                    return right; // Return the second value
                }
            }
        }
    }

    // Mixed-type logical operations (string && boolean, etc.)
    if (left.isString() && right.isBoolean()) {
        bool right_bool = right.asBoolean();
        
        switch (expression->oper.type) {
            case AND: {
                if (!isTruthy(left)) {
                    return left; // Return the falsy value
                } else {
                    return right; // Return the second value
                }
            }
            case OR: {
                if (isTruthy(left)) {
                    return left; // Return the truthy value
                } else {
                    return right; // Return the second value
                }
            }
            case PLUS: return left + right;
        }
    }

    if (left.isBoolean() && right.isString()) {
        bool left_bool = left.asBoolean();
        
        switch (expression->oper.type) {
            case AND: {
                if (!isTruthy(left)) {
                    return left; // Return the falsy value
                } else {
                    return right; // Return the second value
                }
            }
            case OR: {
                if (isTruthy(left)) {
                    return left; // Return the truthy value
                } else {
                    return right; // Return the second value
                }
            }
            case PLUS: return left + right;
        }
    }

    if (left.isString() && right.isNumber()) {
        double right_num = right.asNumber();
        
        switch (expression->oper.type) {
            case AND: {
                if (!isTruthy(left)) {
                    return left; // Return the falsy value
                } else {
                    return right; // Return the second value
                }
            }
            case OR: {
                if (isTruthy(left)) {
                    return left; // Return the truthy value
                } else {
                    return right; // Return the second value
                }
            }
            case PLUS: return left + right;
            case STAR: {
                if (!isWholeNumer(right_num)) {
                    if (errorReporter) {
                        errorReporter->reportError(expression->oper.line, expression->oper.column, "Invalid String Multiplication", 
                            "String multiplier must be a whole number");
                    }
                    throw std::runtime_error("String multiplier must be whole number");
                }
                std::string result;
                for (int i = 0; i < static_cast<int>(right_num); i++) {
                    result += left.asString();
                }
                return Value(result);
            }
        }
    }

    if (left.isNumber() && right.isString()) {
        double left_num = left.asNumber();
        
        switch (expression->oper.type) {
            case AND: {
                if (!isTruthy(left)) {
                    return left; // Return the falsy value
                } else {
                    return right; // Return the second value
                }
            }
            case OR: {
                if (isTruthy(left)) {
                    return left; // Return the truthy value
                } else {
                    return right; // Return the second value
                }
            }
            case PLUS: return left + right;
            case STAR: {
                if (!isWholeNumer(left_num)) {
                    if (errorReporter) {
                        errorReporter->reportError(expression->oper.line, expression->oper.column, "Invalid String Multiplication", 
                            "String multiplier must be a whole number");
                    }
                    throw std::runtime_error("String multiplier must be whole number");
                }
                std::string result;
                for (int i = 0; i < static_cast<int>(left_num); i++) {
                    result += right.asString();
                }
                return Value(result);
            }
        }
    }

    if (left.isNone() && right.isString()) {
        std::string right_string = right.asString();
        
        switch (expression->oper.type) {
            case PLUS: return left + right;
        }
        if (errorReporter) {
            errorReporter->reportError(expression->oper.line, expression->oper.column, "Runtime Error", 
                "Cannot use '" + expression->oper.lexeme + "' on none and a string", expression->oper.lexeme);
        }
        throw std::runtime_error("Cannot use '" + expression->oper.lexeme + "' on none and a string");
    }
    
    if (left.isString() && right.isNone()) {
        std::string left_string = left.asString();
        
        switch (expression->oper.type) {
            case PLUS: return left + right;
        }
        if (errorReporter) {
            errorReporter->reportError(expression->oper.line, expression->oper.column, "Runtime Error", 
                "Cannot use '" + expression->oper.lexeme + "' on a string and none", expression->oper.lexeme);
        }
        throw std::runtime_error("Cannot use '" + expression->oper.lexeme + "' on a string and none");
    }
    else
    {
        if (errorReporter) {
            errorReporter->reportError(expression->oper.line, expression->oper.column, "Runtime Error", 
                "Operands must be of same type when using: " + expression->oper.lexeme, expression->oper.lexeme);
        }
        throw std::runtime_error("Operands must be of same type when using: " + expression->oper.lexeme);
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
    
    // Update the variable if it's a variable expression
    if (auto varExpr = std::dynamic_pointer_cast<VarExpr>(expression->operand)) {
        environment->assign(varExpr->name, Value(newValue));
    } else {
        if (errorReporter) {
            errorReporter->reportError(expression->oper.line, expression->oper.column, 
                "Runtime Error", "Increment/decrement can only be applied to variables.", "");
        }
        throw std::runtime_error("Increment/decrement can only be applied to variables.");
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
            StdLib::addToEnvironment(environment, *this, errorReporter);
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
            Value currentValue = environment->get(expression->name.lexeme);
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
            throw std::runtime_error("Expected " + std::to_string(function->params.size()) +
                                   " arguments but got " + std::to_string(arguments.size()) + ".");
        }
        
        // Check if this is a tail call
        if (expression->isTailCall) {
            // Create a thunk for tail call optimization
            auto thunk = new Thunk([this, function, arguments]() -> Value {
                // Set up the environment for the tail call
                auto previousEnv = environment;
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
                        environment = previousEnv;
                        return context.returnValue;
                    }
                }
                
                environment = previousEnv;
                return context.returnValue;
            });
            
            // Return the thunk as a Value
            return Value(thunk);
        } else {
            // Normal function call - create new environment
            auto previousEnv = environment;
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
                    environment = previousEnv;
                    return context.returnValue;
                }
            }
            
            environment = previousEnv;
            return context.returnValue;
        }
    }
    
    throw std::runtime_error("Can only call functions and classes.");
}

Value Interpreter::visitFunctionExpr(const std::shared_ptr<FunctionExpr>& expression) {
    // Convert Token parameters to string parameters
    std::vector<std::string> paramNames;
    for (const Token& param : expression->params) {
        paramNames.push_back(param.lexeme);
    }
    
    auto function = msptr(Function)("anonymous", paramNames, expression->body, environment);
    functions.push_back(function); // Keep the shared_ptr alive
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
    
    auto function = msptr(Function)(statement->name.lexeme, 
                                   paramNames, 
                                   statement->body, 
                                   environment);
    functions.push_back(function); // Keep the shared_ptr alive
    environment->define(statement->name.lexeme, Value(function.get()));
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

void Interpreter::interpret(std::vector<std::shared_ptr<Stmt> > statements) {
    for(const std::shared_ptr<Stmt>& s : statements)
    {
        execute(s, nullptr); // No context needed for top-level execution
    }
}

void Interpreter::execute(const std::shared_ptr<Stmt>& statement, ExecutionContext* context)
{
    statement->accept(this, context);
}

void Interpreter::executeBlock(std::vector<std::shared_ptr<Stmt> > statements, std::shared_ptr<Environment> env, ExecutionContext* context)
{
    std::shared_ptr<Environment> previous = this->environment;
    this->environment = env;

    for(const std::shared_ptr<Stmt>& s : statements)
    {
        execute(s, context);
        if (context && context->hasReturn) {
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
    if(a.isNumber())
    {
        if(b.isNumber())
        {
            return a.asNumber() == b.asNumber();
        }

        return false;
    }
    else if(a.isBoolean())
    {
        if(b.isBoolean())
        {
            return a.asBoolean() == b.asBoolean();
        }

        return false;
    }
    else if(a.isString())
    {
        if(b.isString())
        {
            return a.asString() == b.asString();
        }

        return false;
    }
    else if(a.isNone())
    {
        if(b.isNone())
        {
            return true;
        }

        return false;
    }

    throw std::runtime_error("Invalid isEqual compariosn");
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

    throw std::runtime_error("Could not convert object to string");
}

bool Interpreter::isWholeNumer(double num) {
    double integral = num;
    double fractional = std::modf(num, &integral);

    if(std::abs(fractional) < std::numeric_limits<double>::epsilon())
    {
        return true;
    }
    else
    {
        return false;
    }
}















