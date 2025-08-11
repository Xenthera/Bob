#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <stack>
#include <optional>
#include <functional>

#include "Value.h"
#include "RuntimeDiagnostics.h"

struct Expr;
struct Stmt;
struct Environment;
struct BuiltinFunction;
struct Function;
struct Thunk;
class ErrorReporter;
struct ExecutionContext;
struct CallExpr;

 

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
    // Global extension registries
    std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Function>>> classExtensions;
    std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Function>>> builtinExtensions; // keys: "string","array","dict","any"
    std::unordered_map<std::string, std::string> classParents; // child -> parent
    std::unordered_map<std::string, std::unordered_map<std::string, Value>> classTemplates; // className -> template dict
    ErrorReporter* errorReporter;
    bool inThunkExecution = false;
    
    // Automatic cleanup tracking
    int thunkCreationCount = 0;
    static const int CLEANUP_THRESHOLD = 10000;
    
    RuntimeDiagnostics diagnostics;  // Utility functions for runtime operations
    std::unique_ptr<Evaluator> evaluator;
    std::unique_ptr<Executor> executor;
    // Pending throw propagation from expression evaluation
    bool hasPendingThrow = false;
    Value pendingThrow = NONE_VALUE;
    int tryDepth = 0;
    bool inlineErrorReported = false;
    
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
    
    void addFunction(std::shared_ptr<Function> function);
    void reportError(int line, int column, const std::string& errorType, const std::string& message, const std::string& lexeme = "");
    void addBuiltinFunction(std::shared_ptr<BuiltinFunction> func);
    void cleanupUnusedFunctions();
    void cleanupUnusedThunks();
    void forceCleanup();
    // Extension APIs
    void registerExtension(const std::string& targetName, const std::string& methodName, std::shared_ptr<Function> fn);
    std::shared_ptr<Function> lookupExtension(const std::string& targetName, const std::string& methodName);
    void registerClass(const std::string& className, const std::string& parentName);
    std::string getParentClass(const std::string& className) const;
    void setClassTemplate(const std::string& className, const std::unordered_map<std::string, Value>& tmpl);
    bool getClassTemplate(const std::string& className, std::unordered_map<std::string, Value>& out) const;
    std::unordered_map<std::string, Value> buildMergedTemplate(const std::string& className) const;
    void addStdLibFunctions();
     // Throw propagation helpers
     void setPendingThrow(const Value& v) { hasPendingThrow = true; pendingThrow = v; }
     bool consumePendingThrow(Value& out) { if (!hasPendingThrow) return false; out = pendingThrow; hasPendingThrow = false; pendingThrow = NONE_VALUE; return true; }
     // Try tracking
     void enterTry() { tryDepth++; }
     void exitTry() { if (tryDepth > 0) tryDepth--; }
     bool isInTry() const { return tryDepth > 0; }
     void markInlineErrorReported() { inlineErrorReported = true; }
     bool hasInlineErrorReported() const { return inlineErrorReported; }
     void clearInlineErrorReported() { inlineErrorReported = false; }
     bool hasReportedError() const;
    
    

private:
    Value runTrampoline(Value initialResult);
};
