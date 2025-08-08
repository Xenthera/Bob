#include "Evaluator.h"
#include "Interpreter.h"
#include "helperFunctions/HelperFunctions.h"

Evaluator::Evaluator(Interpreter* interpreter) : interpreter(interpreter) {}

Value Evaluator::visitLiteralExpr(const std::shared_ptr<LiteralExpr>& expr) {
    if (expr->isNull) {
        return NONE_VALUE;
    }
    if (expr->isNumber) {
        double num;
        if (expr->value.length() > 2 && expr->value[0] == '0' && expr->value[1] == 'b') {
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

Value Evaluator::visitGroupingExpr(const std::shared_ptr<GroupingExpr>& expression) {
    return interpreter->evaluate(expression->expression);
}

Value Evaluator::visitUnaryExpr(const std::shared_ptr<UnaryExpr>& expression)
{
    Value right = interpreter->evaluate(expression->right);

    switch (expression->oper.type) {
        case MINUS:
            if (!right.isNumber()) {
                interpreter->reportError(expression->oper.line, expression->oper.column, "Runtime Error", 
                    "Operand must be a number when using: " + expression->oper.lexeme, expression->oper.lexeme);
                throw std::runtime_error("Operand must be a number when using: " + expression->oper.lexeme);
            }
            return Value(-right.asNumber());

        case BANG:
            return Value(!interpreter->isTruthy(right));

        case BIN_NOT:
            if (!right.isNumber()) {
                interpreter->reportError(expression->oper.line, expression->oper.column, "Runtime Error", 
                    "Operand must be a number when using: " + expression->oper.lexeme, expression->oper.lexeme);
                throw std::runtime_error("Operand must be a number when using: " + expression->oper.lexeme);
            }
            return Value(static_cast<double>(~(static_cast<long>(right.asNumber()))));

        default:
            interpreter->reportError(expression->oper.line, expression->oper.column, "Runtime Error", 
                "Invalid unary operator: " + expression->oper.lexeme, expression->oper.lexeme);
            throw std::runtime_error("Invalid unary operator: " + expression->oper.lexeme);
    }
}

Value Evaluator::visitBinaryExpr(const std::shared_ptr<BinaryExpr>& expression) {
    Value left = interpreter->evaluate(expression->left);
    Value right = interpreter->evaluate(expression->right);

    // Handle logical operators (AND, OR) - these work with any types
    if (expression->oper.type == AND) {
        return interpreter->isTruthy(left) ? right : left;
    }
    if (expression->oper.type == OR) {
        return interpreter->isTruthy(left) ? left : right;
    }

    // Handle equality operators - these work with any types
    if (expression->oper.type == DOUBLE_EQUAL || expression->oper.type == BANG_EQUAL) {
        bool equal = interpreter->isEqual(left, right);
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
                default: break; // Unreachable
            }
        }
        
        // Error for non-number comparisons
        std::string opName;
        switch (expression->oper.type) {
            case GREATER: opName = ">"; break;
            case GREATER_EQUAL: opName = ">="; break;
            case LESS: opName = "<"; break;
            case LESS_EQUAL: opName = "<="; break;
            default: break; // Unreachable
        }
        
        interpreter->reportError(expression->oper.line, expression->oper.column, "Runtime Error", 
            ErrorUtils::makeOperatorError(opName, left.getType(), right.getType()), opName);
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
                interpreter->reportError(expression->oper.line, expression->oper.column, "Runtime Error", 
                    "Unknown operator: " + expression->oper.lexeme, expression->oper.lexeme);
                throw std::runtime_error("Unknown operator: " + expression->oper.lexeme);
        }
    } catch (const std::runtime_error& e) {
        // The Value operators provide good error messages, just add context
        interpreter->reportError(expression->oper.line, expression->oper.column, "Runtime Error", 
            e.what(), expression->oper.lexeme);
        throw;
    }
}

Value Evaluator::visitVarExpr(const std::shared_ptr<VarExpr>& expression)
{
    return interpreter->getEnvironment()->get(expression->name);
}

Value Evaluator::visitIncrementExpr(const std::shared_ptr<IncrementExpr>& expression) {
    // Get the current value of the operand
    Value currentValue = interpreter->evaluate(expression->operand);
    
    if (!currentValue.isNumber()) {
        interpreter->reportError(expression->oper.line, expression->oper.column, 
            "Runtime Error", "Increment/decrement can only be applied to numbers.", "");
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
        interpreter->reportError(expression->oper.line, expression->oper.column, 
            "Runtime Error", "Invalid increment/decrement operator.", "");
        throw std::runtime_error("Invalid increment/decrement operator.");
    }
    
    // Update the variable or array element
    if (auto varExpr = std::dynamic_pointer_cast<VarExpr>(expression->operand)) {
        interpreter->getEnvironment()->assign(varExpr->name, Value(newValue));
    } else if (auto arrayExpr = std::dynamic_pointer_cast<ArrayIndexExpr>(expression->operand)) {
        // Handle array indexing increment/decrement
        Value array = interpreter->evaluate(arrayExpr->array);
        Value index = interpreter->evaluate(arrayExpr->index);
        
        if (!array.isArray()) {
            interpreter->reportError(expression->oper.line, expression->oper.column, 
                "Runtime Error", "Can only index arrays", "");
            throw std::runtime_error("Can only index arrays");
        }
        
        if (!index.isNumber()) {
            interpreter->reportError(expression->oper.line, expression->oper.column, 
                "Runtime Error", "Array index must be a number", "");
            throw std::runtime_error("Array index must be a number");
        }
        
        int idx = static_cast<int>(index.asNumber());
        std::vector<Value>& arr = array.asArray();
        
        if (idx < 0 || idx >= static_cast<int>(arr.size())) {
            interpreter->reportError(arrayExpr->bracket.line, arrayExpr->bracket.column, 
                "Runtime Error", "Array index out of bounds", "");
            throw std::runtime_error("Array index out of bounds");
        }
        
        // Update the array element
        arr[idx] = Value(newValue);
    } else {
        interpreter->reportError(expression->oper.line, expression->oper.column, 
            "Runtime Error", "Increment/decrement can only be applied to variables or array elements.", "");
        throw std::runtime_error("Increment/decrement can only be applied to variables or array elements.");
    }
    
    // Return the appropriate value based on prefix/postfix
    if (expression->isPrefix) {
            return Value(newValue);  // Prefix: return new value
    } else {
        return currentValue;     // Postfix: return old value
    }
}


Value Evaluator::visitAssignExpr(const std::shared_ptr<AssignExpr>& expression) {
    Value value = interpreter->evaluate(expression->value);
    
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
            Value currentValue = interpreter->getEnvironment()->get(expression->name);
            
            // ... (rest of compound assignment logic) ...
            
            break;
        }
        default:
            break;
    }
    interpreter->getEnvironment()->assign(expression->name, value);
    return value;
}

Value Evaluator::visitTernaryExpr(const std::shared_ptr<TernaryExpr>& expression) {
    Value condition = interpreter->evaluate(expression->condition);
    
    if (interpreter->isTruthy(condition)) {
        return interpreter->evaluate(expression->thenExpr);
    } else {
        return interpreter->evaluate(expression->elseExpr);
    }
}

Value Evaluator::visitCallExpr(const std::shared_ptr<CallExpr>& expression) {
    Value callee = expression->callee->accept(this);
    
    std::vector<Value> arguments;
    for (const auto& argument : expression->arguments) {
        arguments.push_back(argument->accept(this));
    }
    
    if (callee.isFunction()) {
        Function* function = callee.asFunction();
        
        // Check arity
        if (arguments.size() != function->params.size()) {
            interpreter->reportError(expression->paren.line, expression->paren.column, "Runtime Error",
                "Expected " + std::to_string(function->params.size()) + " arguments but got " + 
                std::to_string(arguments.size()) + ".", "");
            throw std::runtime_error("Wrong number of arguments.");
        }
        
        // Create new environment for function call
        auto environment = std::make_shared<Environment>(function->closure);
        for (size_t i = 0; i < function->params.size(); i++) {
            environment->define(function->params[i], arguments[i]);
        }
        
        // Execute function body
        auto previous = interpreter->getEnvironment();
        interpreter->setEnvironment(environment);
        
        ExecutionContext context;
        context.isFunctionBody = true;
        
        try {
            for (const auto& stmt : function->body) {
                interpreter->execute(stmt, &context);
                if (context.hasReturn) {
                    interpreter->setEnvironment(previous);
                    return context.returnValue;
                }
            }
        } catch (...) {
            interpreter->setEnvironment(previous);
            throw;
        }
        
        interpreter->setEnvironment(previous);
        return NONE_VALUE;
        
    } else if (callee.isBuiltinFunction()) {
        BuiltinFunction* builtinFunction = callee.asBuiltinFunction();
        return builtinFunction->func(arguments, expression->paren.line, expression->paren.column);
        
    } else {
        interpreter->reportError(expression->paren.line, expression->paren.column, "Runtime Error",
            "Can only call functions and classes.", "");
        throw std::runtime_error("Can only call functions and classes.");
    }
}

Value Evaluator::visitArrayLiteralExpr(const std::shared_ptr<ArrayLiteralExpr>& expr) {
    std::vector<Value> elements;
    
    for (const auto& element : expr->elements) {
        elements.push_back(interpreter->evaluate(element));
    }
    
    return Value(elements);
}

Value Evaluator::visitArrayIndexExpr(const std::shared_ptr<ArrayIndexExpr>& expr) {
    Value array = expr->array->accept(this);
    Value index = expr->index->accept(this);
    
    if (array.isArray()) {
        // Handle array indexing
        if (!index.isNumber()) {
            interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                "Array index must be a number", "");
            throw std::runtime_error("Array index must be a number");
        }
        
        int idx = static_cast<int>(index.asNumber());
        const std::vector<Value>& arr = array.asArray();
        
        if (idx < 0 || idx >= static_cast<int>(arr.size())) {
            interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                "Array index out of bounds", "");
            throw std::runtime_error("Array index out of bounds");
        }
        
        return arr[idx];
        
    } else if (array.isDict()) {
        // Handle dictionary indexing
        if (!index.isString()) {
            interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                "Dictionary key must be a string", "");
            throw std::runtime_error("Dictionary key must be a string");
        }
        
        std::string key = index.asString();
        const std::unordered_map<std::string, Value>& dict = array.asDict();
        
        auto it = dict.find(key);
        if (it != dict.end()) {
            return it->second;
        } else {
            return NONE_VALUE;  // Return none for missing keys
        }
        
    } else {
        interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
            "Can only index arrays and dictionaries", "");
        throw std::runtime_error("Can only index arrays and dictionaries");
    }
}

Value Evaluator::visitArrayAssignExpr(const std::shared_ptr<ArrayAssignExpr>& expr) {
    Value array = expr->array->accept(this);
    Value index = expr->index->accept(this);
    Value value = expr->value->accept(this);
    
    if (array.isArray()) {
        // Handle array assignment
        if (!index.isNumber()) {
            interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                "Array index must be a number", "");
            throw std::runtime_error("Array index must be a number");
        }
        
        int idx = static_cast<int>(index.asNumber());
        std::vector<Value>& arr = array.asArray();
        
        if (idx < 0 || idx >= static_cast<int>(arr.size())) {
            interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                "Array index out of bounds", "");
            throw std::runtime_error("Array index out of bounds");
        }
        
        arr[idx] = value;
        return value;
        
    } else if (array.isDict()) {
        // Handle dictionary assignment
        if (!index.isString()) {
            interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                "Dictionary key must be a string", "");
            throw std::runtime_error("Dictionary key must be a string");
        }
        
        std::string key = index.asString();
        std::unordered_map<std::string, Value>& dict = array.asDict();
        
        dict[key] = value;
        return value;
        
    } else {
        interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
            "Can only assign to array or dictionary elements", "");
        throw std::runtime_error("Can only assign to array or dictionary elements");
    }
}


Value Evaluator::visitDictLiteralExpr(const std::shared_ptr<DictLiteralExpr>& expr) {
    std::unordered_map<std::string, Value> dict;
    
    for (const auto& pair : expr->pairs) {
        Value value = interpreter->evaluate(pair.second);
        dict[pair.first] = value;
    }
    
    return Value(dict);
}

Value Evaluator::visitFunctionExpr(const std::shared_ptr<FunctionExpr>& expression) {
    std::vector<std::string> paramNames;
    for (const Token& param : expression->params) {
        paramNames.push_back(param.lexeme);
    }
    
    auto function = std::make_shared<Function>("", paramNames, expression->body, interpreter->getEnvironment());
    interpreter->addFunction(function);
    return Value(function);
}
