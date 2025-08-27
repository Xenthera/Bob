#include "Evaluator.h"
#include "Interpreter.h"
#include "Environment.h"
#include "AssignmentUtils.h"
#include "helperFunctions/HelperFunctions.h"
#include "ExecutionContext.h"

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
    if (expr->isInteger) {
        long long val = std::stoll(expr->value);
        // Check if we can use the value pool
        Value* pooled = ValuePool::getInteger(val);
        if (pooled) {
            return *pooled;
        }
        return Value(val); // Create integer value
    }
    if (expr->isBigInt) {
        return Value(GMPWrapper::BigInt::fromString(expr->value)); // Create bigint value
    }
    if (expr->isBoolean) {
        if (expr->value == "true") return TRUE_VALUE;
        if (expr->value == "false") return FALSE_VALUE;
    }
    if (expr->value == "0") return Value(0LL);
    if (expr->value == "1") return Value(1LL);
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
            if (right.isInteger()) {
                return Value(-right.asInteger());
            }
            if (right.isNumber()) {
                return Value(-right.asNumber());
            }
            throw std::runtime_error("Operand must be a number when using: " + expression->oper.lexeme);

        case BANG:
            return Value(!interpreter->isTruthy(right));

        case BIN_NOT:
            if (right.isInteger()) {
                return Value(static_cast<long long>(~right.asInteger()));
            }
            if (right.isNumber()) {
                return Value(static_cast<double>(~(static_cast<long>(right.asNumber()))));
            }
            throw std::runtime_error("Operand must be a number when using: " + expression->oper.lexeme);

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

    // Handle comparison operators - work with numbers and integers
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
        
        if (left.isInteger() && right.isInteger()) {
            long long leftNum = left.asInteger();
            long long rightNum = right.asInteger();
            
            switch (expression->oper.type) {
                case GREATER: return Value(leftNum > rightNum);
                case GREATER_EQUAL: return Value(leftNum >= rightNum);
                case LESS: return Value(leftNum < rightNum);
                case LESS_EQUAL: return Value(leftNum <= rightNum);
                default: break; // Unreachable
            }
        }
        
        // Mixed integer/number comparisons
        if (left.isInteger() && right.isNumber()) {
            double leftNum = static_cast<double>(left.asInteger());
            double rightNum = right.asNumber();
            
            switch (expression->oper.type) {
                case GREATER: return Value(leftNum > rightNum);
                case GREATER_EQUAL: return Value(leftNum >= rightNum);
                case LESS: return Value(leftNum < rightNum);
                case LESS_EQUAL: return Value(leftNum <= rightNum);
                default: break; // Unreachable
            }
        }
        
        if (left.isNumber() && right.isInteger()) {
            double leftNum = left.asNumber();
            double rightNum = static_cast<double>(right.asInteger());
            
            switch (expression->oper.type) {
                case GREATER: return Value(leftNum > rightNum);
                case GREATER_EQUAL: return Value(leftNum >= rightNum);
                case LESS: return Value(leftNum < rightNum);
                case LESS_EQUAL: return Value(leftNum <= rightNum);
                default: break; // Unreachable
            }
        }
        
        // BigInt comparisons
        if (left.isBigInt() && right.isBigInt()) {
            switch (expression->oper.type) {
                case GREATER: return Value(left.asBigInt() > right.asBigInt());
                case GREATER_EQUAL: return Value(left.asBigInt() >= right.asBigInt());
                case LESS: return Value(left.asBigInt() < right.asBigInt());
                case LESS_EQUAL: return Value(left.asBigInt() <= right.asBigInt());
                default: break; // Unreachable
            }
        }
        
        // Mixed BigInt comparisons
        if (left.isBigInt() && right.isInteger()) {
            switch (expression->oper.type) {
                case GREATER: return Value(left.asBigInt() > GMPWrapper::BigInt::fromLongLong(right.asInteger()));
                case GREATER_EQUAL: return Value(left.asBigInt() >= GMPWrapper::BigInt::fromLongLong(right.asInteger()));
                case LESS: return Value(left.asBigInt() < GMPWrapper::BigInt::fromLongLong(right.asInteger()));
                case LESS_EQUAL: return Value(left.asBigInt() <= GMPWrapper::BigInt::fromLongLong(right.asInteger()));
                default: break; // Unreachable
            }
        }
        
        if (left.isInteger() && right.isBigInt()) {
            switch (expression->oper.type) {
                case GREATER: return Value(GMPWrapper::BigInt::fromLongLong(left.asInteger()) > right.asBigInt());
                case GREATER_EQUAL: return Value(GMPWrapper::BigInt::fromLongLong(left.asInteger()) >= right.asBigInt());
                case LESS: return Value(GMPWrapper::BigInt::fromLongLong(left.asInteger()) < right.asBigInt());
                case LESS_EQUAL: return Value(GMPWrapper::BigInt::fromLongLong(left.asInteger()) <= right.asBigInt());
                default: break; // Unreachable
            }
        }
        
        if (left.isBigInt() && right.isNumber()) {
            switch (expression->oper.type) {
                case GREATER: return Value(left.asBigInt() > GMPWrapper::doubleToBigInt(right.asNumber()));
                case GREATER_EQUAL: return Value(left.asBigInt() >= GMPWrapper::doubleToBigInt(right.asNumber()));
                case LESS: return Value(left.asBigInt() < GMPWrapper::doubleToBigInt(right.asNumber()));
                case LESS_EQUAL: return Value(left.asBigInt() <= GMPWrapper::doubleToBigInt(right.asNumber()));
                default: break; // Unreachable
            }
        }
        
        if (left.isNumber() && right.isBigInt()) {
            switch (expression->oper.type) {
                case GREATER: return Value(GMPWrapper::doubleToBigInt(left.asNumber()) > right.asBigInt());
                case GREATER_EQUAL: return Value(GMPWrapper::doubleToBigInt(left.asNumber()) >= right.asBigInt());
                case LESS: return Value(GMPWrapper::doubleToBigInt(left.asNumber()) < right.asBigInt());
                case LESS_EQUAL: return Value(GMPWrapper::doubleToBigInt(left.asNumber()) <= right.asBigInt());
                default: break; // Unreachable
            }
        }
        
        // Error for unsupported comparisons
        std::string opName;
        switch (expression->oper.type) {
            case GREATER: opName = ">"; break;
            case GREATER_EQUAL: opName = ">="; break;
            case LESS: opName = "<"; break;
            case LESS_EQUAL: opName = "<="; break;
            default: break; // Unreachable
        }
        
        throw std::runtime_error(ErrorUtils::makeOperatorError(opName, left.getType(), right.getType()));
    }

    // Handle all other operators using Value's operator overloads
    try {
        switch (expression->oper.type) {
            case PLUS: return left + right;
            case MINUS: return left - right;
            case STAR: return left * right;
            case SLASH: {
                if (right.isNumber() && right.asNumber() == 0.0) {
                    // Report precise site for division by zero
                    interpreter->reportError(expression->oper.line, expression->oper.column,
                                             "Runtime Error", "Division by zero", "/");
                    throw std::runtime_error("Division by zero");
                }
                return left / right;
            }
            case PERCENT: {
                if (right.isNumber() && right.asNumber() == 0.0) {
                    interpreter->reportError(expression->oper.line, expression->oper.column,
                                             "Runtime Error", "Modulo by zero", "%");
                    throw std::runtime_error("Modulo by zero");
                }
                return left % right;
            }
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
        throw; // Propagate to statement driver (try/catch) without reporting here
    }
}

Value Evaluator::visitVarExpr(const std::shared_ptr<VarExpr>& expression)
{
    // Special handling for 'super' in extension methods
    if (expression->name.lexeme == "super") {

        // Check if we're in an extension method context
        try {
            Value currentClass = interpreter->getEnvironment()->get(Token{IDENTIFIER, "__currentClass", 0, 0});

            if (currentClass.isString() && !currentClass.asString().empty()) {
                // We're in an extension method, create a special super object
                std::string parentClass = interpreter->getParentClass(currentClass.asString());

                if (!parentClass.empty()) {
                    // Create a special object that represents the parent class
                    std::unordered_map<std::string, Value> superObj;
                    superObj["__class"] = Value(parentClass);
                    superObj["__super"] = Value(true); // Mark as super object

                    return Value(superObj);
                }
            }
        } catch (const std::exception& e) {

            // If __currentClass is not available, fall through to normal lookup
        }
    }
    
    return interpreter->getEnvironment()->get(expression->name);
}

Value Evaluator::visitIncrementExpr(const std::shared_ptr<IncrementExpr>& expression) {
    // Get the current value of the operand
    Value currentValue = interpreter->evaluate(expression->operand);
    
            if (!currentValue.isNumeric()) {
        interpreter->reportError(expression->oper.line, expression->oper.column, 
            "Runtime Error", "Increment/decrement can only be applied to numbers or integers.", "");
        throw std::runtime_error("Increment/decrement can only be applied to numbers or integers.");
    }
    
    Value newValue;
    
    // Determine the operation based on the operator
    if (expression->oper.type == PLUS_PLUS) {
        if (currentValue.isInteger()) {
            long long currentInt = currentValue.asInteger();
            // Check for overflow
            if (currentInt == LLONG_MAX) {
                // Promote to BigInt to avoid overflow
                newValue = Value(GMPWrapper::BigInt::fromLongLong(currentInt) + GMPWrapper::BigInt(1));
            } else {
                newValue = Value(currentInt + 1);
            }
        } else if (currentValue.isBigInt()) {
            // Handle BigInt increment
            newValue = Value(currentValue.asBigInt() + GMPWrapper::BigInt(1));
        } else {
            double currentNum = currentValue.asNumber();
            newValue = Value(currentNum + 1.0);
        }
    } else if (expression->oper.type == MINUS_MINUS) {
        if (currentValue.isInteger()) {
            long long currentInt = currentValue.asInteger();
            // Check for underflow
            if (currentInt == LLONG_MIN) {
                // Promote to BigInt to avoid underflow
                newValue = Value(GMPWrapper::BigInt::fromLongLong(currentInt) - GMPWrapper::BigInt(1));
            } else {
                newValue = Value(currentInt - 1);
            }
        } else if (currentValue.isBigInt()) {
            // Handle BigInt decrement
            newValue = Value(currentValue.asBigInt() - GMPWrapper::BigInt(1));
        } else {
            double currentNum = currentValue.asNumber();
            newValue = Value(currentNum - 1.0);
        }
    } else {
        interpreter->reportError(expression->oper.line, expression->oper.column, 
            "Runtime Error", "Invalid increment/decrement operator.", "");
        throw std::runtime_error("Invalid increment/decrement operator.");
    }
    
    // Update the variable, array element, or object property
    if (auto varExpr = std::dynamic_pointer_cast<VarExpr>(expression->operand)) {
        interpreter->getEnvironment()->assign(varExpr->name, newValue);
    } else if (auto arrayExpr = std::dynamic_pointer_cast<ArrayIndexExpr>(expression->operand)) {
        // Handle array indexing increment/decrement
        Value array = interpreter->evaluate(arrayExpr->array);
        Value index = interpreter->evaluate(arrayExpr->index);
        
            if (array.isArray()) {
        if (!index.isNumeric()) {
            interpreter->reportError(expression->oper.line, expression->oper.column, 
                "Runtime Error", "Array index must be a number", "");
            throw std::runtime_error("Array index must be a number");
        }
            
            int idx;
            if (index.isInteger()) {
                idx = static_cast<int>(index.asInteger());
            } else {
                idx = static_cast<int>(index.asNumber());
            }
            std::vector<Value>& arr = array.asArray();
            
            if (idx < 0 || idx >= static_cast<int>(arr.size())) {
                interpreter->reportError(arrayExpr->bracket.line, arrayExpr->bracket.column, 
                    "Runtime Error", "Array index out of bounds", "");
                throw std::runtime_error("Array index out of bounds");
            }
            
            // Update the array element
            arr[idx] = newValue;
        } else if (array.isString()) {
            // Handle string indexing increment/decrement (read-only)
            interpreter->reportError(expression->oper.line, expression->oper.column, 
                "Runtime Error", "Cannot increment/decrement string characters (strings are immutable)", "");
            throw std::runtime_error("Cannot increment/decrement string characters (strings are immutable)");
        } else {
            interpreter->reportError(expression->oper.line, expression->oper.column, 
                "Runtime Error", "Can only index arrays and strings", "");
            throw std::runtime_error("Can only index arrays and strings");
        }
    } else if (auto propExpr = std::dynamic_pointer_cast<PropertyExpr>(expression->operand)) {
        // obj.prop++ / obj.prop--
        Value object = interpreter->evaluate(propExpr->object);
        if (!object.isDict()) {
            throw std::runtime_error("Can only increment/decrement properties on objects");
        }
        std::unordered_map<std::string, Value>& dict = object.asDict();
        dict[propExpr->name.lexeme] = newValue;
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

    if (expression->op.type == EQUAL) {
        // Assign first to release references held by the old values
        interpreter->getEnvironment()->assign(expression->name, value);
        // Perform cleanup on any reassignment
        interpreter->getFunctionRegistry().forceCleanup();
        return value;
    }

    // Compound assignment operators
    Value currentValue = interpreter->getEnvironment()->get(expression->name);
    try {
        Value newValue = computeCompoundAssignment(currentValue, expression->op.type, value);
        interpreter->getEnvironment()->assign(expression->name, newValue);
        return newValue;
    } catch (const std::runtime_error&) {
        interpreter->reportError(expression->op.line, expression->op.column, "Runtime Error",
                                 "Unknown assignment operator: " + expression->op.lexeme, "");
        throw;
    }
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
    // Delegate to inline implementation in Interpreter for performance
    return interpreter->evaluateCallExprInline(expression);
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
        if (!index.isNumeric()) {
        interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
            "Array index must be a number", "");
        interpreter->markInlineErrorReported();
            throw std::runtime_error("Array index must be a number");
        }
        
        int idx;
        if (index.isInteger()) {
            idx = static_cast<int>(index.asInteger());
        } else {
            idx = static_cast<int>(index.asNumber());
        }
        const std::vector<Value>& arr = array.asArray();
        
        if (idx < 0 || idx >= static_cast<int>(arr.size())) {
        interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
            "Array index out of bounds", "");
        interpreter->markInlineErrorReported();
            throw std::runtime_error("Array index out of bounds");
        }
        
        return arr[idx];
        
    } else if (array.isDict()) {
        // Handle dictionary indexing
        if (!index.isString()) {
        interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
            "Dictionary key must be a string", "");
        interpreter->markInlineErrorReported();
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
        
    } else if (array.isString()) {
        // Handle string indexing
        if (!index.isNumeric()) {
        interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
            "String index must be a number", "");
        interpreter->markInlineErrorReported();
            throw std::runtime_error("String index must be a number");
        }
        
        int idx;
        if (index.isInteger()) {
            idx = static_cast<int>(index.asInteger());
        } else {
            idx = static_cast<int>(index.asNumber());
        }
        const std::string& str = array.asString();
        
        if (idx < 0) {
            idx = static_cast<int>(str.length()) + idx;
        }
        
        if (idx < 0 || idx >= static_cast<int>(str.length())) {
        interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
            "String index out of bounds", "");
        interpreter->markInlineErrorReported();
            throw std::runtime_error("String index out of bounds");
        }
        
        return Value(std::string(1, str[idx]));
        
    } else {
        interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
            "Can only index arrays, dictionaries, and strings", "");
        interpreter->markInlineErrorReported();
        throw std::runtime_error("Can only index arrays, dictionaries, and strings");
    }
}

Value Evaluator::visitPropertyExpr(const std::shared_ptr<PropertyExpr>& expr) {
    Value object = expr->object->accept(this);
    std::string propertyName = expr->name.lexeme;
    
    if (object.isModule()) {
        // Forward to module exports
        auto* mod = object.asModule();
        if (mod && mod->exports) {
            auto it = mod->exports->find(propertyName);
            if (it != mod->exports->end()) return it->second;
        }
        return NONE_VALUE;
    } else if (object.isDict()) {
        Value v = getDictProperty(object, propertyName);
        if (!v.isNone()) {
            // Only create dispatchers for class methods, not for plain functions stored as properties
            const auto& d = object.asDict();
            auto itc = d.find("__class");
            if (itc != d.end() && itc->second.isString() && v.isFunction()) {
                // This is a class instance, create a dispatcher for method calls
                std::string curCls = itc->second.asString();
                auto dispatcher = std::make_shared<BuiltinFunction>(curCls + "." + propertyName, [runtime=interpreter, self=object, curCls, propertyName](std::vector<Value> args, int line, int col) -> Value {
                    std::shared_ptr<Function> sel;
                    
                    // First check for instance methods on the current class
                    if (!curCls.empty()) sel = runtime->getClassRegistry().lookupClassMethodDirect(curCls, propertyName, args.size());
                    
                    // Then check for extension methods on the current class
                    if (!sel && !curCls.empty()) sel = runtime->getExtensionRegistry().lookupExtension(curCls, propertyName);
                    
                    // Then check for inherited class methods
                    if (!sel && !curCls.empty()) sel = runtime->getClassRegistry().lookupClassMethodOverload(curCls, propertyName, args.size());
                    
                    // Then check for class extensions in inheritance chain
                    if (!sel && !curCls.empty()) {
                        std::string cur = curCls;
                        while (!cur.empty()) {
                            if (auto fn = runtime->getExtensionRegistry().lookupExtension(cur, propertyName)) {
                                sel = fn;
                                break;
                            }
                            cur = runtime->getParentClass(cur);
                        }
                    }
                    

                    
                    // Then check for built-in extensions
                    if (!sel) sel = runtime->getExtensionRegistry().lookupExtensionOverload("dict", propertyName, args.size());
                    if (!sel) sel = runtime->getExtensionRegistry().lookupExtensionOverload("any", propertyName, args.size());
                    
                    if (!sel) { 
                        runtime->reportError(line, col, "Runtime Error", "Method '" + propertyName + "' not found", ""); 
                        throw std::runtime_error("Method not found"); 
                    }
                    
                    std::shared_ptr<Environment> saved = runtime->getEnvironment();
                    runtime->setEnvironment(std::make_shared<Environment>(sel->closure));
                    runtime->getEnvironment()->setErrorReporter(runtime->getErrorReporter());
                    runtime->getEnvironment()->define("this", self);
                    // Set __currentClass for class methods
                    if (!curCls.empty()) {
                        runtime->getEnvironment()->define("__currentClass", Value(curCls));
                    }
                    for (size_t i = 0; i < sel->params.size(); ++i) runtime->getEnvironment()->define(sel->params[i], args[i]);
                    ExecutionContext ctx; ctx.isFunctionBody = true;
                    for (const auto& s : sel->body) { 
                        runtime->execute(s, &ctx); 
                        if (ctx.hasThrow) { 
                            runtime->setPendingThrow(ctx.thrownValue, ctx.throwLine, ctx.throwColumn); 
                            runtime->setEnvironment(saved); 
                            return NONE_VALUE; 
                        } 
                        if (ctx.hasReturn) { 
                            runtime->setEnvironment(saved); 
                            return ctx.returnValue; 
                        } 
                    }
                    runtime->setEnvironment(saved); 
                    return ctx.returnValue;
                });
                return Value(dispatcher);
            }
            return v;
        }
        // Fallback to class extensions with inheritance walk
        const auto& d = object.asDict();
        std::string cls = "";
        auto it = d.find("__class");
        if (it != d.end() && it->second.isString()) cls = it->second.asString();
        

        
        if (!cls.empty()) {
            std::string cur = cls;
            while (!cur.empty()) {
                if (auto fn = interpreter->getExtensionRegistry().lookupExtension(cur, propertyName)) {
                    // Wrap the extension function in a dispatcher to provide proper "this" context
                    auto dispatcher = std::make_shared<BuiltinFunction>(cur + "." + propertyName, [runtime=interpreter, self=object, fn, cur](std::vector<Value> args, int line, int col) -> Value {
                        std::shared_ptr<Environment> saved = runtime->getEnvironment();
                        runtime->setEnvironment(std::make_shared<Environment>(fn->closure));
                        runtime->getEnvironment()->setErrorReporter(runtime->getErrorReporter());
                        runtime->getEnvironment()->define("this", self);
                        runtime->getEnvironment()->define("__currentClass", Value(cur));

                        for (size_t i = 0; i < fn->params.size(); ++i) {
                            if (i < args.size()) {
                                runtime->getEnvironment()->define(fn->params[i], args[i]);
                            }
                        }

                        ExecutionContext ctx; ctx.isFunctionBody = true;
                        for (const auto& s : fn->body) {
                            runtime->execute(s, &ctx);
                            if (ctx.hasThrow) {
                                runtime->setPendingThrow(ctx.thrownValue, ctx.throwLine, ctx.throwColumn);
                                runtime->setEnvironment(saved);
                                return NONE_VALUE;
                            }
                            if (ctx.hasReturn) {
                                runtime->setEnvironment(saved);
                                return ctx.returnValue;
                            }
                        }
                        runtime->setEnvironment(saved);
                        return ctx.returnValue;
                    });
                    return Value(dispatcher);
                }
                cur = interpreter->getParentClass(cur);
            }
        }
        

        
        // Provide method-style builtins on dict
        if (propertyName == "len") {
            auto bf = std::make_shared<BuiltinFunction>("dict.len", [object](std::vector<Value>, int, int){
                return Value(static_cast<double>(object.asDict().size()));
            });
            return Value(bf);
        } else if (propertyName == "keys") {
            auto bf = std::make_shared<BuiltinFunction>("dict.keys", [object](std::vector<Value>, int, int){
                std::vector<Value> keys; const auto& m = object.asDict();
                for (const auto& kv : m) keys.push_back(Value(kv.first));
                return Value(keys);
            });
            return Value(bf);
        } else if (propertyName == "values") {
            auto bf = std::make_shared<BuiltinFunction>("dict.values", [object](std::vector<Value>, int, int){
                std::vector<Value> vals; const auto& m = object.asDict();
                for (const auto& kv : m) vals.push_back(kv.second);
                return Value(vals);
            });
            return Value(bf);
        } else if (propertyName == "has") {
            auto bf = std::make_shared<BuiltinFunction>("dict.has", [object](std::vector<Value> args, int, int){
                if (args.size() != 1 || !args[0].isString()) return Value(false);
                const auto& m = object.asDict();
                return Value(m.find(args[0].asString()) != m.end());
            });
            return Value(bf);
        }
        // Fallback to dict and any extensions
        if (auto fn = interpreter->getExtensionRegistry().lookupExtension("dict", propertyName)) return Value(fn);
        if (auto anyFn = interpreter->getExtensionRegistry().lookupExtension("any", propertyName)) return Value(anyFn);
        return NONE_VALUE;
    } else if (object.isArray()) {
        Value v = getArrayProperty(object, propertyName);
        if (!v.isNone()) return v;
        // Provide method-style builtins on array
        if (propertyName == "len") {
            auto bf = std::make_shared<BuiltinFunction>("array.len", [object](std::vector<Value>, int, int){
                return Value(static_cast<double>(object.asArray().size()));
            });
            return Value(bf);
        } else if (propertyName == "push") {
            auto bf = std::make_shared<BuiltinFunction>("array.push", [object](std::vector<Value> args, int, int){
                std::vector<Value>& arr = const_cast<std::vector<Value>&>(object.asArray());
                for (size_t i = 0; i < args.size(); ++i) arr.push_back(args[i]);
                return object;
            });
            return Value(bf);
        } else if (propertyName == "pop") {
            auto bf = std::make_shared<BuiltinFunction>("array.pop", [object](std::vector<Value>, int, int){
                std::vector<Value>& arr = const_cast<std::vector<Value>&>(object.asArray());
                if (arr.empty()) return NONE_VALUE;
                Value v = arr.back();
                arr.pop_back();
                return v;
            });
            return Value(bf);
        }
        // Fallback to array extensions
        if (auto fn = interpreter->getExtensionRegistry().lookupExtension("array", propertyName)) return Value(fn);
        if (auto anyFn = interpreter->getExtensionRegistry().lookupExtension("any", propertyName)) return Value(anyFn);
        return NONE_VALUE;
    } else {
        // Try extension dispatch for built-ins and any
        std::string target;
        if (object.isString()) target = "string";
        else if (object.isNumeric()) target = "number";
        else if (object.isArray()) target = "array"; // handled above, but keep for completeness
        else if (object.isDict()) target = "dict";   // handled above
        else target = object.isModule() ? "any" : "any";

        // Provide method-style builtins for string/number
        if (object.isString() && propertyName == "len") {
            auto bf = std::make_shared<BuiltinFunction>("string.len", [object](std::vector<Value>, int, int){
                return Value(static_cast<double>(object.asString().length()));
            });
            return Value(bf);
        }
        if (object.isNumber() && propertyName == "toInt") {
            auto bf = std::make_shared<BuiltinFunction>("number.toInt", [object](std::vector<Value>, int, int){
                return Value(static_cast<double>(static_cast<long long>(object.asNumber())));
            });
            return Value(bf);
        }

        if (object.isModule()) {
            // Modules are immutable and have no dynamic methods
            return NONE_VALUE;
        }
        auto fn = interpreter->getExtensionRegistry().lookupExtension(target, propertyName);
        if (!object.isModule() && (fn || interpreter->getExtensionRegistry().lookupExtension("any", propertyName))) {
            auto dispatcher = std::make_shared<BuiltinFunction>(target + "." + propertyName, [runtime=interpreter, self=object, target, propertyName](std::vector<Value> args, int line, int col) -> Value {
                std::shared_ptr<Function> sel = runtime->getExtensionRegistry().lookupExtensionOverload(target, propertyName, args.size());
                if (!sel) sel = runtime->getExtensionRegistry().lookupExtensionOverload("any", propertyName, args.size());
                if (!sel) { runtime->reportError(line, col, "Runtime Error", "No overload of method '" + propertyName + "' for " + std::to_string(args.size()) + " argument(s)", ""); throw std::runtime_error("No extension overload"); }
                std::shared_ptr<Environment> saved = runtime->getEnvironment();
                runtime->setEnvironment(std::make_shared<Environment>(sel->closure));
                runtime->getEnvironment()->setErrorReporter(runtime->getErrorReporter());
                runtime->getEnvironment()->define("this", self);
                for (size_t i = 0; i < sel->params.size(); ++i) runtime->getEnvironment()->define(sel->params[i], args[i]);
                ExecutionContext ctx; ctx.isFunctionBody = true;
                for (const auto& s : sel->body) { runtime->execute(s, &ctx); if (ctx.hasThrow) { runtime->setPendingThrow(ctx.thrownValue, ctx.throwLine, ctx.throwColumn); runtime->setEnvironment(saved); return NONE_VALUE; } if (ctx.hasReturn) { runtime->setEnvironment(saved); return ctx.returnValue; } }
                runtime->setEnvironment(saved); return ctx.returnValue;
            });
            return Value(dispatcher);
        }

        interpreter->reportError(expr->name.line, expr->name.column, "Runtime Error",
            "Cannot access property '" + propertyName + "' on this type", "");
        interpreter->markInlineErrorReported();
        throw std::runtime_error("Cannot access property '" + propertyName + "' on this type");
    }
}

Value Evaluator::visitArrayAssignExpr(const std::shared_ptr<ArrayAssignExpr>& expr) {
    Value array = expr->array->accept(this);
    Value index = expr->index->accept(this);
    Value value = expr->value->accept(this);
    
    if (array.isArray()) {
        // Handle array assignment
        if (!index.isNumeric()) {
            if (!interpreter->isInTry()) {
                interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                    "Array index must be a number", "");
                interpreter->markInlineErrorReported();
            }
            throw std::runtime_error("Array index must be a number");
        }
        
        int idx;
        if (index.isInteger()) {
            idx = static_cast<int>(index.asInteger());
        } else {
            idx = static_cast<int>(index.asNumber());
        }
        std::vector<Value>& arr = array.asArray();
        
        if (idx < 0 || idx >= static_cast<int>(arr.size())) {
            if (!interpreter->isInTry()) {
                interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                    "Array index out of bounds", "");
                interpreter->markInlineErrorReported();
            }
            throw std::runtime_error("Array index out of bounds");
        }
        
        arr[idx] = value;
        return value;
        
    } else if (array.isDict()) {
        // Handle dictionary assignment
        if (!index.isString()) {
            if (!interpreter->isInTry()) {
                interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                    "Dictionary key must be a string", "");
                interpreter->markInlineErrorReported();
            }
            throw std::runtime_error("Dictionary key must be a string");
        }
        
        std::string key = index.asString();
        std::unordered_map<std::string, Value>& dict = array.asDict();
        
        dict[key] = value;
        return value;
        
    } else if (array.isString()) {
        // Handle string assignment (read-only - strings are immutable)
        if (!interpreter->isInTry()) {
            interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                "Cannot assign to string characters (strings are immutable)", "");
            interpreter->markInlineErrorReported();
        }
        throw std::runtime_error("Cannot assign to string characters (strings are immutable)");
        
    } else {
        if (!interpreter->isInTry()) {
            interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                "Can only assign to array or dictionary elements", "");
            interpreter->markInlineErrorReported();
        }
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

Value Evaluator::visitPropertyAssignExpr(const std::shared_ptr<PropertyAssignExpr>& expr) {
    Value object = expr->object->accept(this);
    Value value = expr->value->accept(this);
    std::string propertyName = expr->name.lexeme;
    
    if (object.isModule()) {
        // Modules are immutable: disallow setting properties
        if (!interpreter->isInTry()) {
            interpreter->reportError(expr->name.line, expr->name.column, "Import Error",
                "Cannot assign property '" + propertyName + "' on module (immutable)", "");
            interpreter->markInlineErrorReported();
        }
        throw std::runtime_error("Cannot assign property on module (immutable)");
    } else if (object.isDict()) {
        // Modify the dictionary in place
        std::unordered_map<std::string, Value>& dict = object.asDict();
        dict[propertyName] = value;
        return value;  // Return the assigned value
    } else {
        if (!interpreter->isInTry()) {
            interpreter->reportError(expr->name.line, expr->name.column, "Runtime Error",
                "Cannot assign property '" + propertyName + "' on non-object", "");
            interpreter->markInlineErrorReported();
        }
        throw std::runtime_error("Cannot assign property '" + propertyName + "' on non-object");
    }
}

Value Evaluator::visitFunctionExpr(const std::shared_ptr<FunctionExpr>& expression) {
    std::vector<std::string> paramNames;
    for (const Token& param : expression->params) {
        paramNames.push_back(param.lexeme);
    }
    
    auto closureEnv = std::make_shared<Environment>(*interpreter->getEnvironment());
    closureEnv->pruneForClosureCapture();
    auto function = std::make_shared<Function>("", paramNames, expression->body, closureEnv);
    return Value(function);
}

Value Evaluator::getArrayProperty(const Value& arrayValue, const std::string& propertyName) {
    const std::vector<Value>& arr = arrayValue.asArray();
    
    // Create builtin array properties as an actual dictionary
    std::unordered_map<std::string, Value> arrayProperties;
    arrayProperties["length"] = Value(static_cast<double>(arr.size()));
    arrayProperties["empty"] = Value(arr.empty());
    
    if (!arr.empty()) {
        arrayProperties["first"] = arr[0];
        arrayProperties["last"] = arr[arr.size() - 1];
    } else {
        arrayProperties["first"] = NONE_VALUE;
        arrayProperties["last"] = NONE_VALUE;
    }
    
    // Look up the requested property
    auto it = arrayProperties.find(propertyName);
    if (it != arrayProperties.end()) {
        return it->second;
    } else {
        return NONE_VALUE; // Unknown property
    }
}

Value Evaluator::getDictProperty(const Value& dictValue, const std::string& propertyName) {
    const std::unordered_map<std::string, Value>& dict = dictValue.asDict();
    
    // First check if it's a user-defined property
    auto userProp = dict.find(propertyName);
    if (userProp != dict.end()) {
        return userProp->second;
    }
    
    // If not found, check for builtin dictionary properties
    std::unordered_map<std::string, Value> dictProperties;
    dictProperties["length"] = Value(static_cast<double>(dict.size()));
    dictProperties["empty"] = Value(dict.empty());
    
    // Create keys array
    std::vector<Value> keysArray;
    for (const auto& pair : dict) {
        keysArray.push_back(Value(pair.first));
    }
    dictProperties["keys"] = Value(keysArray);
    
    // Create values array  
    std::vector<Value> valuesArray;
    for (const auto& pair : dict) {
        valuesArray.push_back(pair.second);
    }
    dictProperties["values"] = Value(valuesArray);
    
    auto builtinProp = dictProperties.find(propertyName);
    if (builtinProp != dictProperties.end()) {
        return builtinProp->second;
    }
    
    // Property not found
    return NONE_VALUE;
}


