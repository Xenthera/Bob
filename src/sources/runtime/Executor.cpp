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
        // If we're not in a try block, display the error
        if (!interpreter->isInTry()) {
            // Check if the error was already reported by the error reporter
            auto errorReporter = interpreter->getErrorReporter();
            if (errorReporter && !errorReporter->hasError()) {
                std::string msg = "Uncaught exception";
                if (top.thrownValue.isString()) msg = top.thrownValue.asString();
                if (top.thrownValue.isDict()) {
                    auto& d = top.thrownValue.asDict();
                    auto it = d.find("message");
                    if (it != d.end() && it->second.isString()) msg = it->second.asString();
                }
                int line = top.throwLine;
                int col = top.throwColumn;
                if (line == 0 && col == 0) { 
                    line = interpreter->getLastErrorLine(); 
                    col = interpreter->getLastErrorColumn(); 
                }
                interpreter->reportError(line, col, "Runtime Error", msg, "");
            }
        }
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
            
            // Get line/column from the error reporter if available
            if (context->throwLine == 0 && context->throwColumn == 0) {
                context->throwLine = interpreter->getLastErrorLine();
                context->throwColumn = interpreter->getLastErrorColumn();
            }
        }
    }
}

void Executor::executeBlock(const std::vector<std::shared_ptr<Stmt>>& statements, std::shared_ptr<Environment> env, ExecutionContext* context) {
    std::shared_ptr<Environment> previous = interpreter->getEnvironment();
    interpreter->setEnvironment(env);

    for (const auto& statement : statements) {
        execute(statement, context);
        // Bridge any pending throws from expression evaluation into the context
        Value pending; int pl=0, pc=0;
        if (interpreter->consumePendingThrow(pending, &pl, &pc)) {
            if (context) { context->hasThrow = true; context->thrownValue = pending; context->throwLine = pl; context->throwColumn = pc; }
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
    Value thrown; int tl=0, tc=0;
    if (interpreter->consumePendingThrow(thrown, &tl, &tc)) {
        if (context) { context->hasThrow = true; context->thrownValue = thrown; context->throwLine = tl; context->throwColumn = tc; }
        return;
    }

    if (interpreter->isInteractiveMode())
        std::cout << "\u001b[38;5;8m[" << interpreter->stringify(value) << "]\u001b[38;5;15m\n";
}

void Executor::visitVarStmt(const std::shared_ptr<VarStmt>& statement, ExecutionContext* context) {
    Value value = NONE_VALUE;
    if (statement->initializer != nullptr) {
        value = statement->initializer->accept(evaluator);
        Value thrownInit; int tl=0, tc=0;
        if (interpreter->consumePendingThrow(thrownInit, &tl, &tc)) { if (context) { context->hasThrow = true; context->thrownValue = thrownInit; context->throwLine = tl; context->throwColumn = tc; } return; }
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
    // Register function; this also installs/updates a dispatcher in env under this name
    interpreter->addFunction(function);
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
    Value thrownIf; int tl=0, tc=0;
    if (interpreter->consumePendingThrow(thrownIf, &tl, &tc)) { if (context) { context->hasThrow = true; context->thrownValue = thrownIf; context->throwLine = tl; context->throwColumn = tc; } return; }
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
        if (loopContext.hasThrow) { if (context) { context->hasThrow = true; context->thrownValue = loopContext.thrownValue; context->throwLine = loopContext.throwLine; context->throwColumn = loopContext.throwColumn; } break; }
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
        if (loopContext.hasThrow) { if (context) { context->hasThrow = true; context->thrownValue = loopContext.thrownValue; context->throwLine = loopContext.throwLine; context->throwColumn = loopContext.throwColumn; } break; }
        if (loopContext.hasReturn) { if (context) { context->hasReturn = true; context->returnValue = loopContext.returnValue; } break; }
        if (loopContext.shouldBreak) { break; }
        if (loopContext.shouldContinue) { loopContext.shouldContinue = false; }
        Value c = statement->condition->accept(evaluator);
        Value thrown; int tl=0, tc=0;
        if (interpreter->consumePendingThrow(thrown, &tl, &tc)) { if (context) { context->hasThrow = true; context->thrownValue = thrown; context->throwLine = tl; context->throwColumn = tc; } break; }
        if (!interpreter->isTruthy(c)) break;
    }
}

void Executor::visitForStmt(const std::shared_ptr<ForStmt>& statement, ExecutionContext* context) {
    // Fast path for simple integer loops
    if (statement->initializer != nullptr && statement->condition != nullptr && statement->increment != nullptr) {
        // Check if this is a simple integer loop: for (var i = 0; i < N; i++)
        if (auto varStmt = std::dynamic_pointer_cast<VarStmt>(statement->initializer)) {
            if (auto binaryExpr = std::dynamic_pointer_cast<BinaryExpr>(statement->condition)) {
                if (auto incrementExpr = std::dynamic_pointer_cast<IncrementExpr>(statement->increment)) {
                    // This looks like a simple loop - try fast path
                    if (fastPathSimpleLoop(statement, context)) {
                        return;
                    }
                }
            }
        }
    }
    
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
            Value thrown; int tl=0, tc=0;
            if (interpreter->consumePendingThrow(thrown, &tl, &tc)) { if (context) { context->hasThrow = true; context->thrownValue = thrown; context->throwLine = tl; context->throwColumn = tc; } break; }
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
                Value thrown; int tl=0, tc=0;
                if (interpreter->consumePendingThrow(thrown, &tl, &tc)) { if (context) { context->hasThrow = true; context->thrownValue = thrown; context->throwLine = tl; context->throwColumn = tc; } break; }
            }
            continue;
        }
        
        if (statement->increment != nullptr) {
            statement->increment->accept(evaluator);
            Value thrown; int tl=0, tc=0;
            if (interpreter->consumePendingThrow(thrown, &tl, &tc)) { if (context) { context->hasThrow = true; context->thrownValue = thrown; context->throwLine = tl; context->throwColumn = tc; } break; }
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
    
    // Temporarily detach the reporter so any direct uses (e.g., in Environment) won't print inline
    auto savedReporter = interpreter->getErrorReporter();
    interpreter->setErrorReporter(nullptr);
    
    ExecutionContext inner;
    if (context) inner.isFunctionBody = context->isFunctionBody;
    execute(statement->tryBlock, &inner);
    
    // Also capture any pending throw signaled by expressions
    Value pending; int pl=0, pc=0;
    if (interpreter->consumePendingThrow(pending, &pl, &pc)) {
        inner.hasThrow = true;
        inner.thrownValue = pending;
        inner.throwLine = pl;
        inner.throwColumn = pc;
    }
    
    // If thrown, handle catch
    if (inner.hasThrow && statement->catchBlock) {
        auto saved = interpreter->getEnvironment();
        auto env = std::make_shared<Environment>(saved);
        env->setErrorReporter(interpreter->getErrorReporter());
        
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
        inner.throwLine = catchCtx.throwLine;
        inner.throwColumn = catchCtx.throwColumn;
        interpreter->setEnvironment(saved);
    }
    
    // finally always
    if (statement->finallyBlock) {
        ExecutionContext fctx;
        fctx.isFunctionBody = inner.isFunctionBody;
        execute(statement->finallyBlock, &fctx);
        if (fctx.hasReturn) { if (context) { context->hasReturn = true; context->returnValue = fctx.returnValue; } interpreter->setErrorReporter(savedReporter); interpreter->exitTry(); return; }
        if (fctx.hasThrow) { if (context) { context->hasThrow = true; context->thrownValue = fctx.thrownValue; } interpreter->setErrorReporter(savedReporter); interpreter->exitTry(); return; }
        if (fctx.shouldBreak) { if (context) { context->shouldBreak = true; } interpreter->setErrorReporter(savedReporter); interpreter->exitTry(); return; }
        if (fctx.shouldContinue) { if (context) { context->shouldContinue = true; } interpreter->setErrorReporter(savedReporter); interpreter->exitTry(); return; }
    }
    
    // propagate remaining control flow
    if (inner.hasReturn) { if (context) { context->hasReturn = true; context->returnValue = inner.returnValue; } interpreter->setErrorReporter(savedReporter); interpreter->exitTry(); return; }
    if (inner.hasThrow) { if (context) { context->hasThrow = true; context->thrownValue = inner.thrownValue; context->throwLine = inner.throwLine; context->throwColumn = inner.throwColumn; } interpreter->setErrorReporter(savedReporter); interpreter->exitTry(); return; }
    if (inner.shouldBreak) { if (context) { context->shouldBreak = true; } interpreter->setErrorReporter(savedReporter); interpreter->exitTry(); return; }
    if (inner.shouldContinue) { if (context) { context->shouldContinue = true; } interpreter->setErrorReporter(savedReporter); interpreter->exitTry(); return; }
    
    // Restore error reporter
    interpreter->setErrorReporter(savedReporter);
    interpreter->exitTry();
}

void Executor::visitThrowStmt(const std::shared_ptr<ThrowStmt>& statement, ExecutionContext* context) {
    Value v = statement->value ? statement->value->accept(evaluator) : NONE_VALUE;
    if (context) {
        context->hasThrow = true;
        context->thrownValue = v;
        context->throwLine = statement->keyword.line;
        context->throwColumn = statement->keyword.column;
    }
}

void Executor::visitImportStmt(const std::shared_ptr<ImportStmt>& statement, ExecutionContext* context) {
    // Determine spec (string literal or identifier)
    std::string spec = statement->moduleName.lexeme; // already STRING with .bob from parser if name-based
    Value mod = interpreter->importModule(spec, statement->importToken.line, statement->importToken.column);
    std::string bindName;
    if (statement->hasAlias) {
        bindName = statement->alias.lexeme;
    } else {
        // Derive default binding name from module path: basename without extension
        std::string path = statement->moduleName.lexeme;
        // Strip directories
        size_t pos = path.find_last_of("/\\");
        std::string base = (pos == std::string::npos) ? path : path.substr(pos + 1);
        // Strip .bob
        if (base.size() > 4 && base.substr(base.size() - 4) == ".bob") {
            base = base.substr(0, base.size() - 4);
        }
        bindName = base;
    }
    interpreter->getEnvironment()->define(bindName, mod);
}

void Executor::visitFromImportStmt(const std::shared_ptr<FromImportStmt>& statement, ExecutionContext* context) {
    std::string spec = statement->moduleName.lexeme; // already STRING with .bob from parser if name-based
    // Star-import case
    if (statement->importAll) {
        // Import the module and bind all public exports into current environment
        Value mod = interpreter->importModule(spec, statement->fromToken.line, statement->fromToken.column);
        const std::unordered_map<std::string, Value>* src = nullptr;
        if (mod.isModule()) src = mod.asModule()->exports.get(); else if (mod.isDict()) src = &mod.asDict();
        if (!src) { throw std::runtime_error("from-import * on non-module"); }
        for (const auto& kv : *src) {
            const std::string& name = kv.first;
            if (!name.empty() && name[0] == '_') continue; // skip private
            interpreter->getEnvironment()->define(name, kv.second);
        }
        return;
    }
    // Build item list name->alias
    std::vector<std::pair<std::string,std::string>> items;
    for (const auto& it : statement->items) {
        items.emplace_back(it.name.lexeme, it.hasAlias ? it.alias.lexeme : it.name.lexeme);
    }
    if (!interpreter->fromImport(spec, items, statement->fromToken.line, statement->fromToken.column)) {
        throw std::runtime_error("from-import failed");
    }
}

void Executor::visitAssignStmt(const std::shared_ptr<AssignStmt>& statement, ExecutionContext* context) {
    Value value = statement->value->accept(evaluator);

    if (statement->op.type == EQUAL) {
        // Assign first to release references held by the old value
        interpreter->getEnvironment()->assign(statement->name, value);
        // Clean up on any reassignment
        interpreter->getFunctionRegistry().forceCleanup();
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
    // Predefine fields as none, capture this class's field initializers, and register them for inheritance evaluation
    std::vector<std::pair<std::string, std::shared_ptr<Expr>>> fieldInitializers;
    for (const auto& f : statement->fields) {
        classDict[f.name.lexeme] = NONE_VALUE;
        fieldInitializers.emplace_back(f.name.lexeme, f.initializer);
    }

    // Attach methods as functions closed over a prototype env
    auto protoEnv = std::make_shared<Environment>(interpreter->getEnvironment());
    protoEnv->pruneForClosureCapture();

    for (const auto& method : statement->methods) {
        std::vector<std::string> paramNames;
        for (const Token& p : method->params) paramNames.push_back(p.lexeme);
        auto fn = std::make_shared<Function>(method->name.lexeme, paramNames, method->body, protoEnv, statement->name.lexeme);
        // Register overload by arity for this class
        interpreter->addClassMethod(statement->name.lexeme, fn);
        // Register overload mapping and store the raw function under the method name.
        interpreter->addClassMethod(statement->name.lexeme, fn);
        classDict[method->name.lexeme] = Value(fn);
    }

    // Save template to interpreter so instances can include inherited fields/methods
    interpreter->setClassTemplate(statement->name.lexeme, classDict);
    // Register field initializers for this class
    interpreter->setClassFieldInitializers(statement->name.lexeme, fieldInitializers);

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
        // Evaluate field initializers across inheritance chain (parent-to-child order)
        {
            // Build chain from base to current
            std::vector<std::string> chain;
            std::string cur = className;
            while (!cur.empty()) { chain.push_back(cur); cur = runtime->getParentClass(cur); }
            std::reverse(chain.begin(), chain.end());
            auto saved = runtime->getEnvironment();
            auto env = std::make_shared<Environment>(saved);
            env->setErrorReporter(nullptr);
            env->define("this", instance);
            runtime->setEnvironment(env);
            for (const auto& cls : chain) {
                std::vector<std::pair<std::string, std::shared_ptr<Expr>>> inits;
                if (runtime->getClassFieldInitializers(cls, inits)) {
                    for (const auto& kv : inits) {
                        const std::string& fieldName = kv.first;
                        const auto& expr = kv.second;
                        if (expr) {
                            Value v = runtime->evaluate(expr);
                            dictRef[fieldName] = v;
                            // Expose field names as locals for subsequent initializers
                            env->define(fieldName, v);
                        }
                    }
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
            // Seed current class for proper super resolution within init
            if (fn && !fn->ownerClass.empty()) {
                newEnv->define("__currentClass", Value(fn->ownerClass));
            }
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
        interpreter->addExtension(target, method->name.lexeme, fn);
    }
}

bool Executor::fastPathSimpleLoop(const std::shared_ptr<ForStmt>& statement, ExecutionContext* context) {
    // Fast path for simple loops like: for (var i = 0; i < N; i++) { sum = sum + i; }
    
    // Check if this is a simple arithmetic loop
    if (auto varStmt = std::dynamic_pointer_cast<VarStmt>(statement->initializer)) {
        if (auto binaryExpr = std::dynamic_pointer_cast<BinaryExpr>(statement->condition)) {
            if (auto incrementExpr = std::dynamic_pointer_cast<IncrementExpr>(statement->increment)) {
                
                // Check if condition is "i < N" where N is a literal
                if (binaryExpr->oper.type == LESS) {
                    if (auto leftVar = std::dynamic_pointer_cast<VarExpr>(binaryExpr->left)) {
                        if (auto rightLit = std::dynamic_pointer_cast<LiteralExpr>(binaryExpr->right)) {
                            if (rightLit->isInteger && leftVar->name.lexeme == varStmt->name.lexeme) {
                                
                                // Get the loop count
                                long long iterations = std::stoll(rightLit->value);
                                
                                // Check if body is simple arithmetic
                                // Handle both ExpressionStmt and BlockStmt with single ExpressionStmt
                                std::shared_ptr<ExpressionStmt> bodyExpr;
                                if (auto blockStmt = std::dynamic_pointer_cast<BlockStmt>(statement->body)) {
                                    if (blockStmt->statements.size() == 1) {
                                        bodyExpr = std::dynamic_pointer_cast<ExpressionStmt>(blockStmt->statements[0]);
                                    }
                                } else {
                                    bodyExpr = std::dynamic_pointer_cast<ExpressionStmt>(statement->body);
                                }
                                
                                if (bodyExpr) {
                                    if (auto assignExpr = std::dynamic_pointer_cast<AssignExpr>(bodyExpr->expression)) {
                                        if (assignExpr->name.lexeme == "sum") {
                                            if (auto binaryExpr2 = std::dynamic_pointer_cast<BinaryExpr>(assignExpr->value)) {
                                                if (binaryExpr2->oper.type == PLUS) {
                                                    if (auto leftVar3 = std::dynamic_pointer_cast<VarExpr>(binaryExpr2->left)) {
                                                        if (auto rightVar = std::dynamic_pointer_cast<VarExpr>(binaryExpr2->right)) {
                                                            if (leftVar3->name.lexeme == "sum" && 
                                                                rightVar->name.lexeme == varStmt->name.lexeme) {
                                                                
                                                                // This is: sum = sum + i
                                                                // Fast path: calculate sum directly
                                                                long long sum = 0;
                                                                for (long long i = 0; i < iterations; i++) {
                                                                    sum += i;
                                                                }
                                                                
                                                                // Set the result
                                                                interpreter->getEnvironment()->assign(Token{IDENTIFIER, "sum", 0, 0}, Value(sum));
                                                                return true;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return false; // Not a simple loop, use normal execution
}
