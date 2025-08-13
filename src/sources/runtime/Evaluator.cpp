#include "Evaluator.h"
#include "Interpreter.h"
#include "Environment.h"
#include "AssignmentUtils.h"
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
                throw std::runtime_error("Operand must be a number when using: " + expression->oper.lexeme);
            }
            return Value(-right.asNumber());

        case BANG:
            return Value(!interpreter->isTruthy(right));

        case BIN_NOT:
            if (!right.isNumber()) {
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
    
    // Update the variable, array element, or object property
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
    } else if (auto propExpr = std::dynamic_pointer_cast<PropertyExpr>(expression->operand)) {
        // obj.prop++ / obj.prop--
        Value object = interpreter->evaluate(propExpr->object);
        if (!object.isDict()) {
            throw std::runtime_error("Can only increment/decrement properties on objects");
        }
        std::unordered_map<std::string, Value>& dict = object.asDict();
        dict[propExpr->name.lexeme] = Value(newValue);
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
        interpreter->forceCleanup();
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
        if (!index.isNumber()) {
        interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
            "Array index must be a number", "");
        interpreter->markInlineErrorReported();
            throw std::runtime_error("Array index must be a number");
        }
        
        int idx = static_cast<int>(index.asNumber());
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
        
    } else {
        interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
            "Can only index arrays and dictionaries", "");
        interpreter->markInlineErrorReported();
        throw std::runtime_error("Can only index arrays and dictionaries");
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
            // If this is an inherited inline method, prefer a current-class extension override
            if (v.isFunction()) {
                const auto& d = object.asDict();
                std::string curCls;
                auto itc = d.find("__class");
                if (itc != d.end() && itc->second.isString()) curCls = itc->second.asString();
                Function* f = v.asFunction();
                if (f && !curCls.empty() && !f->ownerClass.empty() && f->ownerClass != curCls) {
                    if (auto ext = interpreter->lookupExtension(curCls, propertyName)) {
                        return Value(ext);
                    }
                }
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
                if (auto fn = interpreter->lookupExtension(cur, propertyName)) return Value(fn);
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
        if (auto fn = interpreter->lookupExtension("dict", propertyName)) return Value(fn);
        if (auto anyFn = interpreter->lookupExtension("any", propertyName)) return Value(anyFn);
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
        if (auto fn = interpreter->lookupExtension("array", propertyName)) return Value(fn);
        if (auto anyFn = interpreter->lookupExtension("any", propertyName)) return Value(anyFn);
        return NONE_VALUE;
    } else {
        // Try extension dispatch for built-ins and any
        std::string target;
        if (object.isString()) target = "string";
        else if (object.isNumber()) target = "number";
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
        auto fn = interpreter->lookupExtension(target, propertyName);
        if (!object.isModule() && fn) { return Value(fn); }
        if (auto anyFn = interpreter->lookupExtension("any", propertyName)) {
            return Value(anyFn);
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
        if (!index.isNumber()) {
            if (!interpreter->isInTry()) {
                interpreter->reportError(expr->bracket.line, expr->bracket.column, "Runtime Error",
                    "Array index must be a number", "");
                interpreter->markInlineErrorReported();
            }
            throw std::runtime_error("Array index must be a number");
        }
        
        int idx = static_cast<int>(index.asNumber());
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
