#include "Executor.h"
#include "Evaluator.h"
#include "Interpreter.h"
#include "Environment.h"
#include "Parser.h"
#include "AssignmentUtils.h"
#include <iostream>

Executor::Executor(Interpreter* interpreter, Evaluator* evaluator) 
    : interpreter(interpreter), evaluator(evaluator) {}

Executor::~Executor() {}

void Executor::interpret(const std::vector<std::shared_ptr<Stmt>>& statements) {
    ExecutionContext top;
    for (const auto& statement : statements) {
        execute(statement, &top);
        if (top.hasThrow) break;
    }
    if (top.hasThrow) {
        // If already reported inline, don't double-report here
        if (!interpreter->hasInlineErrorReported()) {
            std::string msg = "Uncaught exception";
            if (top.thrownValue.isString()) msg = top.thrownValue.asString();
            if (top.thrownValue.isDict()) {
                auto& d = top.thrownValue.asDict();
                auto it = d.find("message");
                if (it != d.end() && it->second.isString()) msg = it->second.asString();
            }
            interpreter->reportError(0, 0, "Runtime Error", msg, "");
        }
        // Clear inline marker after handling
        interpreter->clearInlineErrorReported();
    }
}

void Executor::execute(const std::shared_ptr<Stmt>& statement, ExecutionContext* context) {
    try {
        statement->accept(this, context);
    } catch (const std::runtime_error& e) {
        if (context) {
            std::unordered_map<std::string, Value> err;
            err["type"] = Value(std::string("RuntimeError"));
            err["message"] = Value(std::string(e.what()));
            context->hasThrow = true;
            context->thrownValue = Value(err);
        }
    }
}

void Executor::executeBlock(const std::vector<std::shared_ptr<Stmt>>& statements, std::shared_ptr<Environment> env, ExecutionContext* context) {
    std::shared_ptr<Environment> previous = interpreter->getEnvironment();
    interpreter->setEnvironment(env);

    for (const auto& statement : statements) {
        execute(statement, context);
        // Bridge any pending throws from expression evaluation into the context
        Value pending;
        if (interpreter->consumePendingThrow(pending)) {
            if (context) { context->hasThrow = true; context->thrownValue = pending; }
        }
        // If an inline reporter already handled this error and we are at top level (no try),
        // avoid reporting it again here.
        if (context && (context->hasReturn || context->shouldBreak || context->shouldContinue || context->hasThrow)) {
            interpreter->setEnvironment(previous);
            return;
        }
    }
    interpreter->setEnvironment(previous);
}

void Executor::visitBlockStmt(const std::shared_ptr<BlockStmt>& statement, ExecutionContext* context) {
    auto newEnv = std::make_shared<Environment>(interpreter->getEnvironment());
    executeBlock(statement->statements, newEnv, context);
}

void Executor::visitExpressionStmt(const std::shared_ptr<ExpressionStmt>& statement, ExecutionContext* context) {
    Value value = statement->expression->accept(evaluator);
    Value thrown;
    if (interpreter->consumePendingThrow(thrown)) {
        if (context) { context->hasThrow = true; context->thrownValue = thrown; }
        return;
    }

    if (interpreter->isInteractiveMode())
        std::cout << "\u001b[38;5;8m[" << interpreter->stringify(value) << "]\u001b[38;5;15m\n";
}

void Executor::visitVarStmt(const std::shared_ptr<VarStmt>& statement, ExecutionContext* context) {
    Value value = NONE_VALUE;
    if (statement->initializer != nullptr) {
        value = statement->initializer->accept(evaluator);
        Value thrownInit;
        if (interpreter->consumePendingThrow(thrownInit)) { if (context) { context->hasThrow = true; context->thrownValue = thrownInit; } return; }
    }
    interpreter->getEnvironment()->define(statement->name.lexeme, value);
}

void Executor::visitFunctionStmt(const std::shared_ptr<FunctionStmt>& statement, ExecutionContext* context) {
    std::vector<std::string> paramNames;
    for (const Token& param : statement->params) {
        paramNames.push_back(param.lexeme);
    }
    
    auto function = std::make_shared<Function>(statement->name.lexeme, 
                                   paramNames, 
                                   statement->body, 
                                   interpreter->getEnvironment());
    interpreter->addFunction(function);
    interpreter->getEnvironment()->define(statement->name.lexeme, Value(function));
}

void Executor::visitReturnStmt(const std::shared_ptr<ReturnStmt>& statement, ExecutionContext* context) {
    Value value = NONE_VALUE;
    if (statement->value != nullptr) {
        value = statement->value->accept(evaluator);
    }
    
    if (context && context->isFunctionBody) {
        context->hasReturn = true;
        context->returnValue = value;
    }
}

void Executor::visitIfStmt(const std::shared_ptr<IfStmt>& statement, ExecutionContext* context) {
    Value condValIf = statement->condition->accept(evaluator);
    Value thrownIf;
    if (interpreter->consumePendingThrow(thrownIf)) { if (context) { context->hasThrow = true; context->thrownValue = thrownIf; } return; }
    if (interpreter->isTruthy(condValIf)) {
        execute(statement->thenBranch, context);
    } else if (statement->elseBranch != nullptr) {
        execute(statement->elseBranch, context);
    }
}

void Executor::visitWhileStmt(const std::shared_ptr<WhileStmt>& statement, ExecutionContext* context) {
    ExecutionContext loopContext;
    if (context) {
        loopContext.isFunctionBody = context->isFunctionBody;
    }
    
    while (interpreter->isTruthy(statement->condition->accept(evaluator))) {
        execute(statement->body, &loopContext);
        if (loopContext.hasThrow) { if (context) { context->hasThrow = true; context->thrownValue = loopContext.thrownValue; } break; }
        if (loopContext.hasReturn) {
            if (context) {
                context->hasReturn = true;
                context->returnValue = loopContext.returnValue;
            }
            break;
        }
        
        if (loopContext.shouldBreak) {
            break;
        }
        
        if (loopContext.shouldContinue) {
            loopContext.shouldContinue = false;
            continue;
        }
    }
}

void Executor::visitDoWhileStmt(const std::shared_ptr<DoWhileStmt>& statement, ExecutionContext* context) {
    ExecutionContext loopContext;
    if (context) {
        loopContext.isFunctionBody = context->isFunctionBody;
    }
    
    while (true) {
        execute(statement->body, &loopContext);
        if (loopContext.hasThrow) { if (context) { context->hasThrow = true; context->thrownValue = loopContext.thrownValue; } break; }
        if (loopContext.hasReturn) { if (context) { context->hasReturn = true; context->returnValue = loopContext.returnValue; } break; }
        if (loopContext.shouldBreak) { break; }
        if (loopContext.shouldContinue) { loopContext.shouldContinue = false; }
        Value c = statement->condition->accept(evaluator);
        Value thrown;
        if (interpreter->consumePendingThrow(thrown)) { if (context) { context->hasThrow = true; context->thrownValue = thrown; } break; }
        if (!interpreter->isTruthy(c)) break;
    }
}

void Executor::visitForStmt(const std::shared_ptr<ForStmt>& statement, ExecutionContext* context) {
    if (statement->initializer != nullptr) {
        execute(statement->initializer, context);
    }
    
    ExecutionContext loopContext;
    if (context) {
        loopContext.isFunctionBody = context->isFunctionBody;
    }
    
    while (true) {
        if (statement->condition != nullptr) {
            Value c = statement->condition->accept(evaluator);
            Value thrown;
            if (interpreter->consumePendingThrow(thrown)) { if (context) { context->hasThrow = true; context->thrownValue = thrown; } break; }
            if (!interpreter->isTruthy(c)) break;
        }
        execute(statement->body, &loopContext);
        if (loopContext.hasThrow) { if (context) { context->hasThrow = true; context->thrownValue = loopContext.thrownValue; } break; }
        if (loopContext.hasReturn) {
            if (context) {
                context->hasReturn = true;
                context->returnValue = loopContext.returnValue;
            }
            break;
        }
        
        if (loopContext.shouldBreak) {
            break;
        }
        
        if (loopContext.shouldContinue) {
            loopContext.shouldContinue = false;
            if (statement->increment != nullptr) {
                statement->increment->accept(evaluator);
                Value thrown;
                if (interpreter->consumePendingThrow(thrown)) { if (context) { context->hasThrow = true; context->thrownValue = thrown; } break; }
            }
            continue;
        }
        
        if (statement->increment != nullptr) {
            statement->increment->accept(evaluator);
            Value thrown;
            if (interpreter->consumePendingThrow(thrown)) { if (context) { context->hasThrow = true; context->thrownValue = thrown; } break; }
        }
    }
}

void Executor::visitBreakStmt(const std::shared_ptr<BreakStmt>& statement, ExecutionContext* context) {
    if (context) {
        context->shouldBreak = true;
    }
}

void Executor::visitContinueStmt(const std::shared_ptr<ContinueStmt>& statement, ExecutionContext* context) {
    if (context) {
        context->shouldContinue = true;
    }
}

void Executor::visitTryStmt(const std::shared_ptr<TryStmt>& statement, ExecutionContext* context) {
    interpreter->enterTry();
    ExecutionContext inner;
    if (context) inner.isFunctionBody = context->isFunctionBody;
    execute(statement->tryBlock, &inner);
    // Also capture any pending throw signaled by expressions
    Value pending;
    if (interpreter->consumePendingThrow(pending)) {
        inner.hasThrow = true;
        inner.thrownValue = pending;
    }
    // If thrown, handle catch
    if (inner.hasThrow && statement->catchBlock) {
        auto saved = interpreter->getEnvironment();
        auto env = std::make_shared<Environment>(saved);
        env->setErrorReporter(nullptr);
        // Bind catch var if provided
        if (!statement->catchVar.lexeme.empty()) {
            env->define(statement->catchVar.lexeme, inner.thrownValue);
        }
        interpreter->setEnvironment(env);
        ExecutionContext catchCtx;
        catchCtx.isFunctionBody = inner.isFunctionBody;
        execute(statement->catchBlock, &catchCtx);
        inner.hasThrow = catchCtx.hasThrow;
        inner.thrownValue = catchCtx.thrownValue;
        interpreter->setEnvironment(saved);
    }
    // finally always
    if (statement->finallyBlock) {
        ExecutionContext fctx;
        fctx.isFunctionBody = inner.isFunctionBody;
        execute(statement->finallyBlock, &fctx);
        if (fctx.hasReturn) { if (context) { context->hasReturn = true; context->returnValue = fctx.returnValue; } return; }
        if (fctx.hasThrow) { if (context) { context->hasThrow = true; context->thrownValue = fctx.thrownValue; } return; }
        if (fctx.shouldBreak) { if (context) { context->shouldBreak = true; } return; }
        if (fctx.shouldContinue) { if (context) { context->shouldContinue = true; } return; }
    }
    // propagate remaining control flow
    if (inner.hasReturn) { if (context) { context->hasReturn = true; context->returnValue = inner.returnValue; } interpreter->exitTry(); return; }
    if (inner.hasThrow) { if (context) { context->hasThrow = true; context->thrownValue = inner.thrownValue; } interpreter->exitTry(); return; }
    if (inner.shouldBreak) { if (context) { context->shouldBreak = true; } interpreter->exitTry(); return; }
    if (inner.shouldContinue) { if (context) { context->shouldContinue = true; } interpreter->exitTry(); return; }
    interpreter->exitTry();
}

void Executor::visitThrowStmt(const std::shared_ptr<ThrowStmt>& statement, ExecutionContext* context) {
    Value v = statement->value ? statement->value->accept(evaluator) : NONE_VALUE;
    if (context) {
        context->hasThrow = true;
        context->thrownValue = v;
    }
}

void Executor::visitAssignStmt(const std::shared_ptr<AssignStmt>& statement, ExecutionContext* context) {
    Value value = statement->value->accept(evaluator);

    if (statement->op.type == EQUAL) {
        // Assign first to release references held by the old value
        interpreter->getEnvironment()->assign(statement->name, value);
        // Clean up on any reassignment
        interpreter->forceCleanup();
        return;
    }

    // Compound assignment operators
    Value currentValue = interpreter->getEnvironment()->get(statement->name);
    try {
        Value newValue = computeCompoundAssignment(currentValue, statement->op.type, value);
        interpreter->getEnvironment()->assign(statement->name, newValue);
    } catch (const std::runtime_error&) {
        interpreter->reportError(statement->op.line, statement->op.column, "Runtime Error",
                                 "Unknown assignment operator: " + statement->op.lexeme, "");
        throw;
    }
}

void Executor::visitClassStmt(const std::shared_ptr<ClassStmt>& statement, ExecutionContext* context) {
    std::unordered_map<std::string, Value> classDict;
    // If parent exists, copy parent's methods as defaults (single inheritance prototype copy)
    if (statement->hasParent) {
        interpreter->registerClass(statement->name.lexeme, statement->parentName.lexeme);
    } else {
        interpreter->registerClass(statement->name.lexeme, "");
    }
    // Predefine fields as none, keep initializers mapped
    std::unordered_map<std::string, std::shared_ptr<Expr>> fieldInitializers;
    for (const auto& f : statement->fields) {
        classDict[f.name.lexeme] = NONE_VALUE;
        fieldInitializers[f.name.lexeme] = f.initializer;
    }

    // Attach methods as functions closed over a prototype env
    auto protoEnv = std::make_shared<Environment>(interpreter->getEnvironment());
    protoEnv->pruneForClosureCapture();

    for (const auto& method : statement->methods) {
        std::vector<std::string> paramNames;
        for (const Token& p : method->params) paramNames.push_back(p.lexeme);
        auto fn = std::make_shared<Function>(method->name.lexeme, paramNames, method->body, protoEnv, statement->name.lexeme);
        classDict[method->name.lexeme] = Value(fn);
    }

    // Save template to interpreter so instances can include inherited fields/methods
    interpreter->setClassTemplate(statement->name.lexeme, classDict);

    // Define a constructor function Name(...) that builds an instance dict
    auto ctorName = statement->name.lexeme;
    auto ctor = std::make_shared<BuiltinFunction>(ctorName, [runtime=interpreter, className=statement->name.lexeme, fieldInitializers](std::vector<Value> args, int line, int col) -> Value {
        Value instance(std::unordered_map<std::string, Value>{});
        auto& dictRef = instance.asDict();
        // Merge class template including inherited members
        std::unordered_map<std::string, Value> tmpl = runtime->buildMergedTemplate(className);
        for (const auto& kv : tmpl) dictRef[kv.first] = kv.second;
        // Tag instance with class name for extension lookup
        dictRef["__class"] = Value(className);
        // Evaluate field initializers with this bound
        if (!fieldInitializers.empty()) {
            auto saved = runtime->getEnvironment();
            auto env = std::make_shared<Environment>(saved);
            env->setErrorReporter(nullptr);
            env->define("this", instance);
            runtime->setEnvironment(env);
            for (const auto& kv : fieldInitializers) {
                if (kv.second) {
                    Value v = runtime->evaluate(kv.second);
                    dictRef[kv.first] = v;
                }
            }
            runtime->setEnvironment(saved);
        }
        // Auto-call init if present: create call env with this and params
        auto it = dictRef.find("init");
        if (it != dictRef.end() && it->second.isFunction()) {
            Function* fn = it->second.asFunction();
            // New environment from closure
            std::shared_ptr<Environment> newEnv = std::make_shared<Environment>(fn->closure);
            newEnv->setErrorReporter(nullptr);
            newEnv->define("this", instance);
            // Bind params
            size_t n = std::min(fn->params.size(), args.size());
            for (size_t i = 0; i < n; ++i) {
                newEnv->define(fn->params[i], args[i]);
            }
            // Execute body
            auto envSaved = runtime->getEnvironment();
            runtime->setEnvironment(newEnv);
            ExecutionContext ctx; ctx.isFunctionBody = true;
            for (const auto& stmt : fn->body) {
                runtime->execute(stmt, &ctx);
                if (ctx.hasReturn) break;
            }
            runtime->setEnvironment(envSaved);
        }
        return instance;
    });

    interpreter->getEnvironment()->define(statement->name.lexeme, Value(ctor));
}

void Executor::visitExtensionStmt(const std::shared_ptr<ExtensionStmt>& statement, ExecutionContext* context) {
    auto target = statement->target.lexeme;
    for (const auto& method : statement->methods) {
        std::vector<std::string> params;
        for (const Token& p : method->params) params.push_back(p.lexeme);
        auto fn = std::make_shared<Function>(method->name.lexeme, params, method->body, interpreter->getEnvironment(), target);
        interpreter->registerExtension(target, method->name.lexeme, fn);
    }
}
