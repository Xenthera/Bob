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

struct ReturnContext {
    Value returnValue;
    bool hasReturn;
    ReturnContext() : returnValue(NONE_VALUE), hasReturn(false) {}
};

static ReturnContext g_returnContext;

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

Value Interpreter::visitBinaryExpr(const std::shared_ptr<BinaryExpr>& expression) 
{
    Value left = evaluate(expression->left);
    Value right = evaluate(expression->right);

    switch (expression->oper.type) {
        case BANG_EQUAL:
            return Value(!isEqual(left, right));
        case DOUBLE_EQUAL:
            return Value(isEqual(left, right));
        default:
            ;
    }

    if(left.isNumber() && right.isNumber())
    {
        double left_double = left.asNumber();
        double right_double = right.asNumber();
        switch (expression->oper.type) {
            case GREATER:
                return Value(left_double > right_double);
            case GREATER_EQUAL:
                return Value(left_double >= right_double);
            case LESS:
                return Value(left_double < right_double);
            case LESS_EQUAL:
                return Value(left_double <= right_double);
            case MINUS:
                return Value(left_double - right_double);
            case PLUS:
                return Value(left_double + right_double);
            case SLASH:
                if(right_double == 0) throw std::runtime_error("DivisionByZeroError: Cannot divide by 0");
                return Value(left_double / right_double);
            case STAR:
                return Value(left_double * right_double);
            case PERCENT:
                return Value(fmod(left_double, right_double));
            default:
                return NONE_VALUE; //unreachable
        }
    }
    else if(left.isString() && right.isString())
    {
        switch (expression->oper.type) {
            case PLUS: {
                std::string left_string = left.asString();
                std::string right_string = right.asString();
                return Value(left_string + right_string);
            }
        }
        throw std::runtime_error("Cannot use '" + expression->oper.lexeme + "' on two strings");

    }
    else if(left.isString() && right.isNumber())
    {
        std::string left_string = left.asString();
        double right_number = right.asNumber();
        
        switch (expression->oper.type) {
            case PLUS: {
                // Use the same logic as stringify for consistent formatting
                double integral = right_number;
                double fractional = std::modf(right_number, &integral);
                
                std::stringstream ss;
                if(std::abs(fractional) < std::numeric_limits<double>::epsilon())
                {
                    ss << std::fixed << std::setprecision(0) << integral;
                }
                else
                {
                    ss << std::fixed << std::setprecision(std::numeric_limits<double>::digits10 - 1) << right_number;
                    std::string str = ss.str();
                    str.erase(str.find_last_not_of('0') + 1, std::string::npos);
                    if (str.back() == '.') {
                        str.pop_back();
                    }
                    return Value(left_string + str);
                }
                return Value(left_string + ss.str());
            }
            case STAR:
                if(isWholeNumer(right_number))
                {
                    std::string s;
                    for (int i = 0; i < (int)right_number; ++i) {
                        s += left_string;
                    }
                    return Value(s);
                }
                else
                {
                    throw std::runtime_error("String multiplier must be whole number");
                }
        }
        throw std::runtime_error("Cannot use '" + expression->oper.lexeme + "' on a string and a number");
    }
    else if(left.isNumber() && right.isString())
    {
        double left_number = left.asNumber();
        std::string right_string = right.asString();
        
        switch (expression->oper.type) {
            case PLUS: {
                // Use the same logic as stringify for consistent formatting
                double integral = left_number;
                double fractional = std::modf(left_number, &integral);
                
                std::stringstream ss;
                if(std::abs(fractional) < std::numeric_limits<double>::epsilon())
                {
                    ss << std::fixed << std::setprecision(0) << integral;
                }
                else
                {
                    ss << std::fixed << std::setprecision(std::numeric_limits<double>::digits10 - 1) << left_number;
                    std::string str = ss.str();
                    str.erase(str.find_last_not_of('0') + 1, std::string::npos);
                    if (str.back() == '.') {
                        str.pop_back();
                    }
                    return Value(str + right_string);
                }
                return Value(ss.str() + right_string);
            }
            case STAR:
                if(isWholeNumer(left_number))
                {
                    std::string s;
                    for (int i = 0; i < (int)left_number; ++i) {
                        s += right_string;
                    }
                    return Value(s);
                }
                else
                {
                    throw std::runtime_error("String multiplier must be whole number");
                }
        }
        throw std::runtime_error("Cannot use '" + expression->oper.lexeme + "' on a number and a string");
    }
    else if(left.isBoolean() && right.isString())
    {
        bool left_bool = left.asBoolean();
        std::string right_string = right.asString();
        
        switch (expression->oper.type) {
            case PLUS: {
                std::string bool_str = left_bool ? "true" : "false";
                return Value(bool_str + right_string);
            }
        }
        throw std::runtime_error("Cannot use '" + expression->oper.lexeme + "' on a boolean and a string");
    }
    else if(left.isString() && right.isBoolean())
    {
        std::string left_string = left.asString();
        bool right_bool = right.asBoolean();
        
        switch (expression->oper.type) {
            case PLUS: {
                std::string bool_str = right_bool ? "true" : "false";
                return Value(left_string + bool_str);
            }
        }
        throw std::runtime_error("Cannot use '" + expression->oper.lexeme + "' on a string and a boolean");
    }
    else if(left.isString() && right.isNone())
    {
        std::string left_string = left.asString();
        
        switch (expression->oper.type) {
            case PLUS: {
                return Value(left_string + "none");
            }
        }
        throw std::runtime_error("Cannot use '" + expression->oper.lexeme + "' on a string and none");
    }
    else if(left.isNone() && right.isString())
    {
        std::string right_string = right.asString();
        
        switch (expression->oper.type) {
            case PLUS: {
                return Value("none" + right_string);
            }
        }
        throw std::runtime_error("Cannot use '" + expression->oper.lexeme + "' on none and a string");
    }
    else
    {
        throw std::runtime_error("Operands must be of same type when using: " + expression->oper.lexeme);
    }

}

Value Interpreter::visitVariableExpr(const std::shared_ptr<VarExpr>& expression)
{
    return environment->get(expression->name);
}

void Interpreter::addStdLibFunctions() {
    // Add standard library functions to the environment
    StdLib::addToEnvironment(environment, *this);
}

void Interpreter::addBuiltinFunction(std::shared_ptr<BuiltinFunction> func) {
    builtinFunctions.push_back(func);
}

Value Interpreter::visitAssignExpr(const std::shared_ptr<AssignExpr>& expression) {
    Value value = evaluate(expression->value);
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
        // Builtin functions now work directly with Value
        return callee.asBuiltinFunction()->func(arguments);
    }
    
    if (callee.isFunction()) {
        Function* function = callee.asFunction();
        if (arguments.size() != function->params.size()) {
            throw std::runtime_error("Expected " + std::to_string(function->params.size()) +
                                   " arguments but got " + std::to_string(arguments.size()) + ".");
        }
        
        auto previousEnv = environment;
        environment = std::make_shared<Environment>(function->closure);
        
        for (size_t i = 0; i < function->params.size(); i++) {
            environment->define(function->params[i], arguments[i]);
        }
        
        Value returnValue = NONE_VALUE;
        bool hasReturn = false;
        
        for (const auto& stmt : function->body) {
            // Reset return context for each statement
            g_returnContext.hasReturn = false;
            g_returnContext.returnValue = NONE_VALUE;
            
            execute(stmt);
            if (g_returnContext.hasReturn) {
                returnValue = g_returnContext.returnValue;
                hasReturn = true;
                break;
            }
        }
        
        environment = previousEnv;
        return returnValue;
    }
    
    throw std::runtime_error("Can only call functions and classes.");
}

void Interpreter::visitBlockStmt(const std::shared_ptr<BlockStmt>& statement) {
    auto newEnv = std::make_shared<Environment>(environment);
    executeBlock(statement->statements, newEnv);
}

void Interpreter::visitExpressionStmt(const std::shared_ptr<ExpressionStmt>& statement) {
    Value value = evaluate(statement->expression);

    if(IsInteractive)
        std::cout << "\u001b[38;5;8m[" << stringify(value) << "]\u001b[38;5;15m" << std::endl;
}



void Interpreter::visitVarStmt(const std::shared_ptr<VarStmt>& statement)
{
    Value value = NONE_VALUE;
    if(statement->initializer != nullptr)
    {
        value = evaluate(statement->initializer);
    }

    //std::cout << "Visit var stmt: " << statement->name.lexeme << " set to: " << stringify(value) << std::endl;

    environment->define(statement->name.lexeme, value);
}

void Interpreter::visitFunctionStmt(const std::shared_ptr<FunctionStmt>& statement)
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

void Interpreter::visitReturnStmt(const std::shared_ptr<ReturnStmt>& statement)
{
    Value value = NONE_VALUE;
    if (statement->value != nullptr) {
        value = evaluate(statement->value);
    }
    
    g_returnContext.hasReturn = true;
    g_returnContext.returnValue = value;
}

void Interpreter::visitIfStmt(const std::shared_ptr<IfStmt>& statement)
{
    if (isTruthy(evaluate(statement->condition))) {
        execute(statement->thenBranch);
    } else if (statement->elseBranch != nullptr) {
        execute(statement->elseBranch);
    }
}

void Interpreter::interpret(std::vector<std::shared_ptr<Stmt>> statements) {
    for(const std::shared_ptr<Stmt>& s : statements)
    {
        execute(s);
    }
}

void Interpreter::execute(const std::shared_ptr<Stmt>& statement)
{
    statement->accept(this);
}

void Interpreter::executeBlock(std::vector<std::shared_ptr<Stmt>> statements, std::shared_ptr<Environment> env)
{
    std::shared_ptr<Environment> previous = this->environment;
    this->environment = env;

    for(const std::shared_ptr<Stmt>& s : statements)
    {
        execute(s);
    }

    this->environment = previous;
}

Value Interpreter::evaluate(const std::shared_ptr<Expr>& expr) {
    return expr->accept(this);
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













