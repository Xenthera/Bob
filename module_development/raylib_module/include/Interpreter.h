#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <stack>
#include <optional>
#include <functional>

#include "Value.h"
#include "TypeWrapper.h"
#include "RuntimeDiagnostics.h"
#include "ModuleRegistry.h"
#include "FunctionRegistry.h"
#include "ClassRegistry.h"
#include "ExtensionRegistry.h"
#include <unordered_set>

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

// Call analysis information for method resolution
struct CallInfo {
    bool isMethodCall = false;
    bool isSuperCall = false;
    std::string methodName;
    Value receiver = NONE_VALUE;
    Value callee = NONE_VALUE;
    std::vector<Value> arguments;
    int line = 0;
    int column = 0;
};

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
    
    // Registry components
    FunctionRegistry functionRegistry;
    ClassRegistry classRegistry;
    ExtensionRegistry extensionRegistry;
    
    ErrorReporter* errorReporter;
    bool inThunkExecution = false;
    
    RuntimeDiagnostics diagnostics;  // Utility functions for runtime operations
    std::unique_ptr<Evaluator> evaluator;
    std::unique_ptr<Executor> executor;
    // Module cache: module key -> module dict value
    std::unordered_map<std::string, Value> moduleCache;
    // Builtin module registry
    ModuleRegistry builtinModules;
    // Import policy flags
    bool allowFileImports = true;
    bool preferFileOverBuiltin = true;
    bool allowBuiltinImports = true;
    std::vector<std::string> moduleSearchPaths; // e.g., BOBPATH
    // Pending throw propagation from expression evaluation
    bool hasPendingThrow = false;
    Value pendingThrow = NONE_VALUE;
    int pendingThrowLine = 0;
    int pendingThrowColumn = 0;
    int lastErrorLine = 0;
    int lastErrorColumn = 0;
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
    ErrorReporter* getErrorReporter() const { return errorReporter; }
    
    // Function management (delegated to FunctionRegistry)
    void addFunction(std::shared_ptr<Function> function);
    
    // Class management (delegated to ClassRegistry)
    void addClassMethod(const std::string& className, std::shared_ptr<Function> method);
    void registerClass(const std::string& className, const std::string& parentName);
    std::string getParentClass(const std::string& className) const;
    void setClassTemplate(const std::string& className, const std::unordered_map<std::string, Value>& tmpl);
    bool getClassTemplate(const std::string& className, std::unordered_map<std::string, Value>& out) const;
    std::unordered_map<std::string, Value> buildMergedTemplate(const std::string& className) const;
    void setClassFieldInitializers(const std::string& className, const std::vector<std::pair<std::string, std::shared_ptr<Expr>>>& inits);
    bool getClassFieldInitializers(const std::string& className, std::vector<std::pair<std::string, std::shared_ptr<Expr>>>& out) const;
    
    // Extension management (delegated to ExtensionRegistry)
    void addExtension(const std::string& targetName, const std::string& methodName, std::shared_ptr<Function> fn);
    
    // Internal registry access (for advanced use cases)
    FunctionRegistry& getFunctionRegistry() { return functionRegistry; }
    ClassRegistry& getClassRegistry() { return classRegistry; }
    ExtensionRegistry& getExtensionRegistry() { return extensionRegistry; }
    
    void reportError(int line, int column, const std::string& errorType, const std::string& message, const std::string& lexeme = "");
    void addStdLibFunctions();
    // Module APIs
    Value importModule(const std::string& spec, int line, int column); // returns module dict
    bool fromImport(const std::string& spec, const std::vector<std::pair<std::string, std::string>>& items, int line, int column); // name->alias
    void setModulePolicy(bool allowFiles, bool preferFiles, const std::vector<std::string>& searchPaths);
    void setBuiltinModulePolicy(bool allowBuiltins) { allowBuiltinImports = allowBuiltins; builtinModules.setPolicy(allowBuiltins); }
    void setBuiltinModuleAllowList(const std::vector<std::string>& allowed) { builtinModules.setAllowList(allowed); }
    void setBuiltinModuleDenyList(const std::vector<std::string>& denied) { builtinModules.setDenyList(denied); }
    void registerBuiltinModule(const std::string& name, std::function<Value(Interpreter&)> factory) { builtinModules.registerFactory(name, std::move(factory)); }
    
    // Simple module registration API
    using ModuleBuilder = ModuleRegistry::ModuleBuilder;

    void registerModule(const std::string& name, std::function<void(ModuleBuilder&)> init) {
        builtinModules.registerModule(name, std::move(init));
    }
    
    ModuleRegistry& getModuleRegistry() { return builtinModules; }
    // Global environment helpers
    bool defineGlobalVar(const std::string& name, const Value& value);
    bool tryGetGlobalVar(const std::string& name, Value& out) const;
     // Throw propagation helpers
     void setPendingThrow(const Value& v, int line = 0, int column = 0) { hasPendingThrow = true; pendingThrow = v; pendingThrowLine = line; pendingThrowColumn = column; }
     bool consumePendingThrow(Value& out, int* lineOut = nullptr, int* colOut = nullptr) { if (!hasPendingThrow) return false; out = pendingThrow; if (lineOut) *lineOut = pendingThrowLine; if (colOut) *colOut = pendingThrowColumn; hasPendingThrow = false; pendingThrow = NONE_VALUE; pendingThrowLine = 0; pendingThrowColumn = 0; return true; }
     // Try tracking
     void enterTry();
     void exitTry();
     bool isInTry() const { return tryDepth > 0; }
     void markInlineErrorReported() { inlineErrorReported = true; }
     bool hasInlineErrorReported() const { return inlineErrorReported; }
     void clearInlineErrorReported() { inlineErrorReported = false; }
     bool hasReportedError() const;
     // Last error site tracking
     void setLastErrorSite(int line, int column) { lastErrorLine = line; lastErrorColumn = column; }
     int getLastErrorLine() const { return lastErrorLine; }
     int getLastErrorColumn() const { return lastErrorColumn; }

    // Process/host metadata (for sys module)
    void setArgv(const std::vector<std::string>& args, const std::string& executablePath) { argvData = args; executableFile = executablePath; }
    std::vector<std::string> getArgv() const { return argvData; }
    std::string getExecutablePath() const { return executableFile; }
    std::unordered_map<std::string, Value> getModuleCacheSnapshot() const { return moduleCache; }

    // Registry accessors for diagnostics and testing
    const FunctionRegistry& getFunctionRegistry() const { return functionRegistry; }
    const ClassRegistry& getClassRegistry() const { return classRegistry; }
    const ExtensionRegistry& getExtensionRegistry() const { return extensionRegistry; }

private:
    Value runTrampoline(Value initialResult);
    
    // Call expression analysis and execution helpers
    CallInfo analyzeCallExpression(const std::shared_ptr<CallExpr>& expression);
    Value resolveCallee(const CallInfo& callInfo);
    Value executeCall(const Value& callee, const CallInfo& callInfo);
    Value resolveExtensionMethod(const Value& receiver, const std::string& methodName);
    Value createBuiltinMethod(const std::string& type, const std::string& methodName, const Value& receiver);
    
    // Stored argv/executable for sys module
    std::vector<std::string> argvData;
    std::string executableFile;
};
