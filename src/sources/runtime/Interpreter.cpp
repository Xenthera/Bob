#include "Interpreter.h"
#include "Evaluator.h"
#include "Executor.h"
#include "BobStdLib.h"
#include "ErrorReporter.h"
#include "Environment.h"
#include "Expression.h"
#include <iostream>

Interpreter::Interpreter(bool isInteractive) 
    : isInteractive(isInteractive), errorReporter(nullptr) {
    evaluator = std::make_unique<Evaluator>(this);
    executor = std::make_unique<Executor>(this, evaluator.get());
    environment = std::make_shared<Environment>();
}

Interpreter::~Interpreter() = default;

void Interpreter::interpret(std::vector<std::shared_ptr<Stmt>> statements) {
    executor->interpret(statements);
}

void Interpreter::execute(const std::shared_ptr<Stmt>& statement, ExecutionContext* context) {
    statement->accept(executor.get(), context);
}

void Interpreter::executeBlock(std::vector<std::shared_ptr<Stmt>> statements, std::shared_ptr<Environment> env, ExecutionContext* context) {
    executor->executeBlock(statements, env, context);
}

Value Interpreter::evaluate(const std::shared_ptr<Expr>& expr) {
    Value result = expr->accept(evaluator.get());
    if (inThunkExecution) {
        return result;
    }
    return runTrampoline(result);
}

Value Interpreter::runTrampoline(Value initialResult) {
    Value current = initialResult;
    while (current.isThunk()) {
        current = current.asThunk()->execute();
    }
    return current;
}

bool Interpreter::isTruthy(Value object) {
    return diagnostics.isTruthy(object);
}

bool Interpreter::isEqual(Value a, Value b) {
    return diagnostics.isEqual(a, b);
}

std::string Interpreter::stringify(Value object) {
    return diagnostics.stringify(object);
}

void Interpreter::addStdLibFunctions() {
    BobStdLib::addToEnvironment(environment, *this, errorReporter);
}

void Interpreter::addBuiltinFunction(std::shared_ptr<BuiltinFunction> func) {
    builtinFunctions.push_back(func);
}

 

void Interpreter::addFunction(std::shared_ptr<Function> function) {
    functions.push_back(function);
}

void Interpreter::setErrorReporter(ErrorReporter* reporter) {
    errorReporter = reporter;
    if (environment) {
        environment->setErrorReporter(reporter);
    }
}

bool Interpreter::isInteractiveMode() const {
    return isInteractive;
}

std::shared_ptr<Environment> Interpreter::getEnvironment() {
    return environment;
}

void Interpreter::setEnvironment(std::shared_ptr<Environment> env) {
    environment = env;
}

void Interpreter::reportError(int line, int column, const std::string& errorType, const std::string& message, const std::string& lexeme) {
    if (errorReporter) {
        errorReporter->reportError(line, column, errorType, message, lexeme);
    }
}

void Interpreter::cleanupUnusedFunctions() {
    diagnostics.cleanupUnusedFunctions(functions);
}

void Interpreter::cleanupUnusedThunks() {
    diagnostics.cleanupUnusedThunks(thunks);
}

void Interpreter::forceCleanup() {
    diagnostics.forceCleanup(builtinFunctions, functions, thunks);
}

void Interpreter::registerExtension(const std::string& targetName, const std::string& methodName, std::shared_ptr<Function> fn) {
    // Builtin targets routed to builtinExtensions
    if (targetName == "string" || targetName == "array" || targetName == "dict" || targetName == "any" || targetName == "number") {
        builtinExtensions[targetName][methodName] = fn;
        return;
    }
    // Otherwise treat as user class name
    classExtensions[targetName][methodName] = fn;
}

std::shared_ptr<Function> Interpreter::lookupExtension(const std::string& targetName, const std::string& methodName) {
    // If this is a user class name, prefer class extensions
    auto cit = classExtensions.find(targetName);
    if (cit != classExtensions.end()) {
        auto mit = cit->second.find(methodName);
        if (mit != cit->second.end()) return mit->second;
        // If not on class, fall through to any
        auto anyIt2 = builtinExtensions.find("any");
        if (anyIt2 != builtinExtensions.end()) {
            auto am = anyIt2->second.find(methodName);
            if (am != anyIt2->second.end()) return am->second;
        }
        return nullptr;
    }

    // Builtin targets
    auto bit = builtinExtensions.find(targetName);
    if (bit != builtinExtensions.end()) {
        auto mit = bit->second.find(methodName);
        if (mit != bit->second.end()) return mit->second;
    }
    // any fallback for builtins and unknowns
    auto anyIt = builtinExtensions.find("any");
    if (anyIt != builtinExtensions.end()) {
        auto mit = anyIt->second.find(methodName);
        if (mit != anyIt->second.end()) return mit->second;
    }
    return nullptr;
}

void Interpreter::registerClass(const std::string& className, const std::string& parentName) {
    if (!parentName.empty()) {
        classParents[className] = parentName;
    }
}

std::string Interpreter::getParentClass(const std::string& className) const {
    auto it = classParents.find(className);
    if (it != classParents.end()) return it->second;
    return "";
}

void Interpreter::setClassTemplate(const std::string& className, const std::unordered_map<std::string, Value>& tmpl) {
    classTemplates[className] = tmpl;
}

bool Interpreter::getClassTemplate(const std::string& className, std::unordered_map<std::string, Value>& out) const {
    auto it = classTemplates.find(className);
    if (it == classTemplates.end()) return false;
    out = it->second;
    return true;
}

std::unordered_map<std::string, Value> Interpreter::buildMergedTemplate(const std::string& className) const {
    std::unordered_map<std::string, Value> merged;
    // Merge parent chain first
    std::string cur = className;
    std::vector<std::string> chain;
    while (!cur.empty()) { chain.push_back(cur); cur = getParentClass(cur); }
    for (auto it = chain.rbegin(); it != chain.rend(); ++it) {
        auto ct = classTemplates.find(*it);
        if (ct != classTemplates.end()) {
            for (const auto& kv : ct->second) merged[kv.first] = kv.second;
        }
    }
    return merged;
}

Value Interpreter::evaluateCallExprInline(const std::shared_ptr<CallExpr>& expression) {
    bool isMethodCall = false;
    bool isSuperCall = false;
    std::string methodName;
    Value receiver = NONE_VALUE;
    if (auto prop = std::dynamic_pointer_cast<PropertyExpr>(expression->callee)) {
        if (auto varObj = std::dynamic_pointer_cast<VarExpr>(prop->object)) {
            if (varObj->name.lexeme == "super") {
                isSuperCall = true;
                if (environment) {
                    try {
                        receiver = environment->get(Token{IDENTIFIER, "this", prop->name.line, prop->name.column});
                    } catch (...) {
                        receiver = NONE_VALUE;
                    }
                }
            }
        }
        if (!isSuperCall) {
            receiver = evaluate(prop->object);
        }
        methodName = prop->name.lexeme;
        isMethodCall = true;
    }

    Value callee = NONE_VALUE;
    if (!isSuperCall) {
        callee = evaluate(expression->callee);
    }

    // If property wasn't found as a callable, try extension lookup
    if (isMethodCall && !(callee.isFunction() || callee.isBuiltinFunction())) {
        if (isSuperCall && !receiver.isDict()) {
            std::string errorMsg = "super can only be used inside class methods";
            if (errorReporter) {
                errorReporter->reportError(expression->paren.line, expression->paren.column, "Runtime Error",
                    errorMsg, "");
            }
            throw std::runtime_error(errorMsg);
        }
        if (!methodName.empty()) {
            // Built-ins direct
            if (isSuperCall && receiver.isDict()) {
                // Resolve using current executing class context if available
                std::string curClass;
                if (environment) {
                    try {
                        Value cc = environment->get(Token{IDENTIFIER, "__currentClass", expression->paren.line, expression->paren.column});
                        if (cc.isString()) curClass = cc.asString();
                    } catch (...) {}
                }
                // If not set yet (e.g., resolving before entering callee), inspect callee ownerClass if available
                if (curClass.empty()) {
                    // Try child class extension to determine current context
                    const auto& d = receiver.asDict();
                    auto itc = d.find("__class");
                    if (itc != d.end() && itc->second.isString()) {
                        std::string child = itc->second.asString();
                        if (auto childExt = lookupExtension(child, methodName)) {
                            curClass = child; // use child as current class for super
                        }
                    }
                }
                if (curClass.empty() && callee.isFunction()) {
                    Function* cf = callee.asFunction();
                    if (cf && !cf->ownerClass.empty()) curClass = cf->ownerClass;
                }
                if (curClass.empty()) {
                    const auto& d = receiver.asDict();
                    auto itc = d.find("__class");
                    if (itc != d.end() && itc->second.isString()) curClass = itc->second.asString();
                }
                std::string cur = getParentClass(curClass);
                int guard = 0;
                while (!cur.empty() && guard++ < 64) {
                    auto tmplIt = classTemplates.find(cur);
                    if (tmplIt != classTemplates.end()) {
                        auto vIt = tmplIt->second.find(methodName);
                        if (vIt != tmplIt->second.end() && vIt->second.isFunction()) {
                            callee = vIt->second;
                            break;
                        }
                    }
                    if (auto fn = lookupExtension(cur, methodName)) { callee = Value(fn); break; }
                    cur = getParentClass(cur);
                }
                // If still not found, try built-in fallbacks to keep behavior consistent
                if (!callee.isFunction()) {
                    if (auto dictFn = lookupExtension("dict", methodName)) callee = Value(dictFn);
                    else if (auto anyFn = lookupExtension("any", methodName)) callee = Value(anyFn);
                }
            } else if (receiver.isArray()) {
                if (auto fn = lookupExtension("array", methodName)) callee = Value(fn);
                else if (methodName == "len") {
                    auto bf = std::make_shared<BuiltinFunction>("array.len", [receiver](std::vector<Value> args, int, int){
                        return Value(static_cast<double>(receiver.asArray().size()));
                    });
                    callee = Value(bf);
                } else if (methodName == "push") {
                    auto bf = std::make_shared<BuiltinFunction>("array.push", [receiver](std::vector<Value> args, int, int){
                        std::vector<Value>& arr = const_cast<std::vector<Value>&>(receiver.asArray());
                        for (size_t i = 0; i < args.size(); ++i) arr.push_back(args[i]);
                        return receiver;
                    });
                    callee = Value(bf);
                } else if (methodName == "pop") {
                    auto bf = std::make_shared<BuiltinFunction>("array.pop", [receiver](std::vector<Value> args, int, int){
                        std::vector<Value>& arr = const_cast<std::vector<Value>&>(receiver.asArray());
                        if (arr.empty()) return NONE_VALUE;
                        Value v = arr.back();
                        arr.pop_back();
                        return v;
                    });
                    callee = Value(bf);
                } else if (auto anyFn = lookupExtension("any", methodName)) callee = Value(anyFn);
            } else if (receiver.isString()) {
                if (auto fn = lookupExtension("string", methodName)) callee = Value(fn);
                else if (methodName == "len") {
                    auto bf = std::make_shared<BuiltinFunction>("string.len", [receiver](std::vector<Value> args, int, int){
                        return Value(static_cast<double>(receiver.asString().length()));
                    });
                    callee = Value(bf);
                } else if (auto anyFn = lookupExtension("any", methodName)) callee = Value(anyFn);
            } else if (receiver.isNumber()) {
                if (auto fn = lookupExtension("number", methodName)) callee = Value(fn);
                else if (methodName == "toInt") {
                    auto bf = std::make_shared<BuiltinFunction>("number.toInt", [receiver](std::vector<Value> args, int, int){
                        return Value(static_cast<double>(static_cast<long long>(receiver.asNumber())));
                    });
                    callee = Value(bf);
                } else if (auto anyFn = lookupExtension("any", methodName)) callee = Value(anyFn);
            } else if (receiver.isDict()) {
                const auto& d = receiver.asDict();
                std::string cls;
                auto it = d.find("__class");
                if (it != d.end() && it->second.isString()) cls = it->second.asString();
                // Walk class chain first
                std::string cur = cls;
                while (!cur.empty()) {
                    if (auto fn = lookupExtension(cur, methodName)) { callee = Value(fn); break; }
                    cur = getParentClass(cur);
                }
                // Fallbacks
                if (!callee.isFunction()) {
                    if (auto dictFn = lookupExtension("dict", methodName)) callee = Value(dictFn);
                    else if (methodName == "len") {
                        auto bf = std::make_shared<BuiltinFunction>("dict.len", [receiver](std::vector<Value> args, int, int){
                            return Value(static_cast<double>(receiver.asDict().size()));
                        });
                        callee = Value(bf);
                    } else if (methodName == "keys") {
                        auto bf = std::make_shared<BuiltinFunction>("dict.keys", [receiver](std::vector<Value> args, int, int){
                            std::vector<Value> keys; const auto& m = receiver.asDict();
                            for (const auto& kv : m) keys.push_back(Value(kv.first));
                            return Value(keys);
                        });
                        callee = Value(bf);
                    } else if (methodName == "values") {
                        auto bf = std::make_shared<BuiltinFunction>("dict.values", [receiver](std::vector<Value> args, int, int){
                            std::vector<Value> vals; const auto& m = receiver.asDict();
                            for (const auto& kv : m) vals.push_back(kv.second);
                            return Value(vals);
                        });
                        callee = Value(bf);
                    } else if (methodName == "has") {
                        auto bf = std::make_shared<BuiltinFunction>("dict.has", [receiver](std::vector<Value> args, int, int){
                            if (args.size() != 1 || !args[0].isString()) return Value(false);
                            const auto& m = receiver.asDict();
                            return Value(m.find(args[0].asString()) != m.end());
                        });
                        callee = Value(bf);
                    }
                    else if (auto anyFn = lookupExtension("any", methodName)) callee = Value(anyFn);
                }
            } else {
                if (auto anyFn = lookupExtension("any", methodName)) callee = Value(anyFn);
            }
        }
    }
    
    if (callee.isBuiltinFunction()) {
        // Handle builtin functions with direct evaluation
        std::vector<Value> arguments;
        for (const auto& argument : expression->arguments) {
            arguments.push_back(evaluate(argument));
        }
        BuiltinFunction* builtinFunction = callee.asBuiltinFunction();
        return builtinFunction->func(arguments, expression->paren.line, expression->paren.column);
    }
    
    if (!callee.isFunction()) {
        std::string errorMsg = isSuperCall ? ("Undefined super method '" + methodName + "'")
                                           : ("Can only call functions, got " + callee.getType());
        if (errorReporter) {
            errorReporter->reportError(expression->paren.line, expression->paren.column, "Runtime Error",
                errorMsg, "");
        }
        throw std::runtime_error(errorMsg);
    }
    
    Function* function = callee.asFunction();
    
    std::vector<Value> arguments;
    for (const auto& argument : expression->arguments) {
        arguments.push_back(evaluate(argument));
    }
    
    // Check arity (like original)
    if (arguments.size() != function->params.size()) {
        if (errorReporter) {
            errorReporter->reportError(expression->paren.line, expression->paren.column, "Runtime Error",
                "Expected " + std::to_string(function->params.size()) +
                " arguments but got " + std::to_string(arguments.size()) + ".", "");
        }
        throw std::runtime_error("Expected " + std::to_string(function->params.size()) +
                               " arguments but got " + std::to_string(arguments.size()) + ".");
    }
    
    // Check if this is a tail call for inline TCO
    if (expression->isTailCall) {
        
        auto thunk = std::make_shared<Thunk>([this, function, arguments, isMethodCall, receiver, isSuperCall]() -> Value {
            ScopedEnv _env(environment);
            environment = std::make_shared<Environment>(function->closure);
            environment->setErrorReporter(errorReporter);
            if (isMethodCall) {
                environment->define("this", receiver);
                if (isSuperCall) environment->define("super", receiver);
                if (function && !function->ownerClass.empty()) {
                    environment->define("__currentClass", Value(function->ownerClass));
                }
            }
            
            for (size_t i = 0; i < function->params.size(); i++) {
                environment->define(function->params[i], arguments[i]);
            }
            
            ExecutionContext context;
            context.isFunctionBody = true;
            
            ScopedThunkFlag _inThunk(inThunkExecution);
            
                for (const auto& stmt : function->body) {
                    stmt->accept(executor.get(), &context);
                    if (context.hasReturn) {
                        return context.returnValue;
                    }
                }
            
            return context.returnValue;
        });
        
        thunks.push_back(thunk);
        
        thunkCreationCount++;
        if (thunkCreationCount >= CLEANUP_THRESHOLD) {
            cleanupUnusedThunks();
            thunkCreationCount = 0;
        }
        
        return Value(thunk);
    } else {
        ScopedEnv _env(environment);
        environment = std::make_shared<Environment>(function->closure);
        environment->setErrorReporter(errorReporter);
        if (isMethodCall) {
            environment->define("this", receiver);
            if (isSuperCall) environment->define("super", receiver);
            if (function && !function->ownerClass.empty()) {
                environment->define("__currentClass", Value(function->ownerClass));
            }
        }
        
        for (size_t i = 0; i < function->params.size(); i++) {
            environment->define(function->params[i], arguments[i]);
        }
        
        ExecutionContext context;
        context.isFunctionBody = true;
        
        for (const auto& stmt : function->body) {
            stmt->accept(executor.get(), &context);
            if (context.hasReturn) {
                return context.returnValue;
            }
        }
        
        return context.returnValue;
    }
}
 
