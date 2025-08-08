#pragma once
#include "Expression.h"
#include "Statement.h"
#include "helperFunctions/ShortHands.h"
#include "TypeWrapper.h"
#include "Environment.h"
#include "Value.h"
#include "BobStdLib.h"
#include "ErrorReporter.h"
#include "ExecutionContext.h"
#include "RuntimeDiagnostics.h"

#include <vector>
#include <memory>
#include <unordered_map>
#include <stack>
#include <optional>
#include <functional>

// Forward declaration
class Evaluator;

// RAII helper for thunk execution flag
struct ScopedThunkFlag {
    bool& flag;
    bool prev;
    ScopedThunkFlag(bool& f) : flag(f), prev(f) { flag = true; }
    ~ScopedThunkFlag() { flag = prev; }
};

// RAII helper for environment management
struct ScopedEnv {
    std::shared_ptr<Environment>& target;
    std::shared_ptr<Environment> prev;
    ScopedEnv(std::shared_ptr<Environment>& e) : target(e), prev(e) {}
    ~ScopedEnv() { target = prev; }
};

// Thunk class for trampoline-based tail call optimization
struct Thunk {
public:
    using ThunkFunction = std::function<Value()>;
    
    explicit Thunk(ThunkFunction func) : func(std::move(func)) {}
    
    Value execute() const {
        return func();
    }
    
    bool isThunk() const { return true; }
    
private:
    ThunkFunction func;
};

class Executor;

class Interpreter {
private:
    std::shared_ptr<Environment> environment;
    bool isInteractive;
    std::vector<std::shared_ptr<BuiltinFunction>> builtinFunctions;
    std::vector<std::shared_ptr<Function>> functions;
    std::vector<std::shared_ptr<Thunk>> thunks;  // Store thunks to prevent memory leaks
    ErrorReporter* errorReporter;
    bool inThunkExecution = false;
    
    // Automatic cleanup tracking
    int functionCreationCount = 0;
    int thunkCreationCount = 0;
    static const int CLEANUP_THRESHOLD = 1000000;
    
    RuntimeDiagnostics diagnostics;  // Utility functions for runtime operations
    std::unique_ptr<Evaluator> evaluator;
    std::unique_ptr<Executor> executor;
    
public:
    explicit Interpreter(bool isInteractive);
    virtual ~Interpreter();

    // Public interface for main
    void interpret(std::vector<std::shared_ptr<Stmt>> statements);
    void setErrorReporter(ErrorReporter* reporter);

    // Methods needed by Evaluator
    Value evaluate(const std::shared_ptr<Expr>& expr);
    Value evaluateCallExprInline(const std::shared_ptr<CallExpr>& expression);  // Inline TCO for performance
    void execute(const std::shared_ptr<Stmt>& statement, ExecutionContext* context = nullptr);
    void executeBlock(std::vector<std::shared_ptr<Stmt>> statements, std::shared_ptr<Environment> env, ExecutionContext* context = nullptr);
    bool isTruthy(Value object);
    bool isEqual(Value a, Value b);
    std::string stringify(Value object);
    bool isInteractiveMode() const;
    std::shared_ptr<Environment> getEnvironment();
    void setEnvironment(std::shared_ptr<Environment> env);
    void addThunk(std::shared_ptr<Thunk> thunk);
    void addFunction(std::shared_ptr<Function> function);
    void reportError(int line, int column, const std::string& errorType, const std::string& message, const std::string& lexeme = "");
    void addBuiltinFunction(std::shared_ptr<BuiltinFunction> func);
    void cleanupUnusedFunctions();
    void cleanupUnusedThunks();
    void forceCleanup();
    
    // Function creation count management
    void incrementFunctionCreationCount();
    int getFunctionCreationCount() const;
    void resetFunctionCreationCount();
    int getCleanupThreshold() const;
    
    // Public access for Evaluator
    bool& getInThunkExecutionRef() { return inThunkExecution; }

private:
    Value evaluateWithoutTrampoline(const std::shared_ptr<Expr>& expr);
    void addStdLibFunctions();
    Value runTrampoline(Value initialResult);
};
