#include "Interpreter.h"
#include "Evaluator.h"
#include "Executor.h"
#include "BobStdLib.h"
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

void Interpreter::addThunk(std::shared_ptr<Thunk> thunk) {
    thunks.push_back(thunk);
}

void Interpreter::addFunction(std::shared_ptr<Function> function) {
    functions.push_back(function);
}

void Interpreter::setErrorReporter(ErrorReporter* reporter) {
    errorReporter = reporter;
    if (environment) {
        environment->setErrorReporter(reporter);
    }
    addStdLibFunctions();
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

Value Interpreter::evaluateCallExprInline(const std::shared_ptr<CallExpr>& expression) {
    Value callee = evaluate(expression->callee);  // Direct call instead of through evaluator
    
    if (callee.isBuiltinFunction()) {
        // Handle builtin functions with direct evaluation
        std::vector<Value> arguments;
        for (const auto& argument : expression->arguments) {
            arguments.push_back(evaluate(argument));  // Direct call
        }
        BuiltinFunction* builtinFunction = callee.asBuiltinFunction();
        return builtinFunction->func(arguments, expression->paren.line, expression->paren.column);
    }
    
    if (!callee.isFunction()) {
        // Provide better error message with type information (like original)
        std::string errorMsg = "Can only call functions, got " + callee.getType();
        if (errorReporter) {
            errorReporter->reportError(expression->paren.line, expression->paren.column, "Runtime Error",
                errorMsg, "");
        }
        throw std::runtime_error(errorMsg);
    }
    
    Function* function = callee.asFunction();
    
    std::vector<Value> arguments;
    for (const auto& argument : expression->arguments) {
        arguments.push_back(evaluate(argument));  // Direct call instead of through evaluator
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
        // Create a thunk for tail call optimization - original inline version
        auto thunk = std::make_shared<Thunk>([this, function, arguments]() -> Value {
            // Use RAII to manage environment (exactly like original)
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
            
                            // Execute function body (inline like original - direct accept for performance)
                for (const auto& stmt : function->body) {
                    stmt->accept(executor.get(), &context);  // Direct call like original
                    if (context.hasReturn) {
                        return context.returnValue;
                    }
                }
            
            return context.returnValue;
        });
        
        // Store the thunk to keep it alive and return as Value (exactly like original)
        thunks.push_back(thunk);
        
        // Automatic cleanup check
        thunkCreationCount++;
        if (thunkCreationCount >= CLEANUP_THRESHOLD) {
            cleanupUnusedThunks();
            thunkCreationCount = 0;
        }
        
        return Value(thunk);
    } else {
        // Normal function call - create new environment (exactly like original)
        ScopedEnv _env(environment);
        environment = std::make_shared<Environment>(function->closure);
        environment->setErrorReporter(errorReporter);
        
        for (size_t i = 0; i < function->params.size(); i++) {
            environment->define(function->params[i], arguments[i]);
        }
        
        ExecutionContext context;
        context.isFunctionBody = true;
        
        // Execute function body (exactly like original - direct accept for performance)
        for (const auto& stmt : function->body) {
            stmt->accept(executor.get(), &context);  // Direct call like original
            if (context.hasReturn) {
                return context.returnValue;
            }
        }
        
        return context.returnValue;
    }
}

// Function creation count management
void Interpreter::incrementFunctionCreationCount() {
    functionCreationCount++;
}

int Interpreter::getFunctionCreationCount() const {
    return functionCreationCount;
}

void Interpreter::resetFunctionCreationCount() {
    functionCreationCount = 0;
}

int Interpreter::getCleanupThreshold() const {
    return 1000000; // Same as CLEANUP_THRESHOLD used for thunks
}
