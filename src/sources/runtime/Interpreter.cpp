#include "Interpreter.h"
#include "register.h"
#include "Evaluator.h"
#include "Executor.h"
#include "ValuePool.h"
#include "ModuleDef.h"
#include "BobStdLib.h"
#include "ErrorReporter.h"
#include "Environment.h"
#include "Expression.h"
#include "Parser.h"
#include <filesystem>
#if defined(_WIN32)
  #include <direct.h>
#else
  #include <unistd.h>
#endif
#include <fstream>
#include <iostream>

Interpreter::Interpreter(bool isInteractive) 
    : isInteractive(isInteractive), errorReporter(nullptr) {
    // Initialize value pool for performance
    ValuePool::initialize();
    
    evaluator = std::make_unique<Evaluator>(this);
    executor = std::make_unique<Executor>(this, evaluator.get());
    environment = std::make_shared<Environment>();
    // Default module search paths: current dir and tests
    moduleSearchPaths = { ".", "tests" };

    // Register all builtin modules via aggregator
    registerAllBuiltinModules(*this);
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
bool Interpreter::defineGlobalVar(const std::string& name, const Value& value) {
    if (!environment) return false;
    try {
        environment->define(name, value);
        return true;
    } catch (...) { return false; }
}

bool Interpreter::tryGetGlobalVar(const std::string& name, Value& out) const {
    if (!environment) return false;
    try {
        out = environment->get(Token{IDENTIFIER, name, 0, 0});
        return true;
    } catch (...) { return false; }
}

bool Interpreter::hasReportedError() const {
    return inlineErrorReported;
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
void Interpreter::setModulePolicy(bool allowFiles, bool preferFiles, const std::vector<std::string>& searchPaths) {
    allowFileImports = allowFiles;
    preferFileOverBuiltin = preferFiles;
    moduleSearchPaths = searchPaths;
}

static std::string joinPath(const std::string& baseDir, const std::string& rel) {
    namespace fs = std::filesystem;
    fs::path p = fs::path(baseDir) / fs::path(rel);
    return fs::path(p).lexically_normal().string();
}

static std::string locateModuleFile(const std::string& baseDir, const std::vector<std::string>& searchPaths, const std::string& nameDotBob) {
    namespace fs = std::filesystem;
    // Only search relative to the importing file's directory
    // 1) baseDir/name.bob
    if (!baseDir.empty()) {
        std::string p = joinPath(baseDir, nameDotBob);
        if (fs::exists(fs::path(p))) return p;
    }
    // 2) baseDir/searchPath/name.bob (search paths are relative to baseDir)
    for (const auto& sp : searchPaths) {
        if (!baseDir.empty()) {
            std::string pb = joinPath(baseDir, joinPath(sp, nameDotBob));
            if (fs::exists(fs::path(pb))) return pb;
        }
    }
    return "";
}

Value Interpreter::importModule(const std::string& spec, int line, int column) {
    // Determine if spec is a path string (heuristic: contains '/' or ends with .bob)
    bool looksPath = spec.find('/') != std::string::npos || (spec.size() >= 4 && spec.rfind(".bob") == spec.size() - 4) || spec.find("..") != std::string::npos;
    // Cache key resolution
    std::string key = spec;
    std::string baseDir = "";
    if (errorReporter && !errorReporter->getCurrentFileName().empty()) {
        std::filesystem::path p(errorReporter->getCurrentFileName());
        baseDir = p.has_parent_path() ? p.parent_path().string() : baseDir;
    }
    if (looksPath) {
        if (!allowFileImports) {
            reportError(line, column, "Import Error", "File imports are disabled by policy", spec);
            throw std::runtime_error("File imports disabled");
        }
        // Resolve STRING path specs:
        // - Absolute: use as-is
        // - Starts with ./ or ../: resolve relative to the importing file directory (baseDir)
        // - Otherwise: resolve relative to current working directory
        if (!spec.empty() && spec[0] == '/') {
            key = spec;
        } else if (spec.rfind("./", 0) == 0 || spec.rfind("../", 0) == 0) {
            key = joinPath(baseDir, spec);
        } else {
            // Resolve all non-absolute paths relative to the importing file directory only
            key = joinPath(baseDir, spec);
        }
    } else {
        // Name import: try file in baseDir or search paths; else builtin
        if (preferFileOverBuiltin && allowFileImports) {
            std::string found = locateModuleFile(baseDir, moduleSearchPaths, spec + ".bob");
            if (!found.empty()) { key = found; looksPath = true; }
        }
        if (!looksPath && allowBuiltinImports && builtinModules.has(spec)) {
            key = std::string("builtin:") + spec;
        }
    }
    // Return from cache
    auto it = moduleCache.find(key);
    if (it != moduleCache.end()) return it->second;

    // If still not a path, it must be builtin, installed, or missing
    if (!looksPath) {
        // Check builtin modules first
        if (builtinModules.has(spec)) {
            // Builtin: return from cache or construct and cache
            auto itc = moduleCache.find(key);
            if (itc != moduleCache.end()) return itc->second;
            Value v = builtinModules.create(spec, *this);
            if (v.isNone()) { // cloaked by policy
                reportError(line, column, "Import Error", "Module not found: " + spec + ".bob", spec);
                throw std::runtime_error("Module not found");
            }
            moduleCache[key] = v;
            return v;
        }
        
        // Check installed modules
        if (builtinModules.isInstalledModule(spec)) {
            try {
                std::cout << "DEBUG: Loading installed module: " << spec << std::endl;
    auto moduleDef = builtinModules.loadInstalledModule(spec);
    std::cout << "DEBUG: Module loaded, registering..." << std::endl;
                // Register the module like builtin modules
                std::cout << "DEBUG: Calling registerModule..." << std::endl;
                moduleDef->registerModule(*this);
                std::cout << "DEBUG: registerModule completed" << std::endl;
                // The module is now registered and can be created like builtin modules
                Value v = builtinModules.create(spec, *this);
                moduleCache[key] = v;
                return v;
            } catch (const std::exception& e) {
                reportError(line, column, "Import Error", "Failed to load installed module: " + std::string(e.what()), spec);
                throw std::runtime_error("Failed to load installed module");
            }
        }
        
        // Module not found
        reportError(line, column, "Import Error", "Module not found: " + spec + ".bob", spec);
        throw std::runtime_error("Module not found");
    }

    // File module: read and execute in isolated env
    std::ifstream file(key);
    if (!file.is_open()) {
        reportError(line, column, "Import Error", "Could not open module file: " + key, spec);
        throw std::runtime_error("Module file open failed");
    }
    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Prepare reporter with module source and cache it for runtime errors
    if (errorReporter) {
        errorReporter->pushSource(code, key);
        errorReporter->cacheModuleSource(key, code);
    }

    // New lexer and parser
    Lexer lx; if (errorReporter) lx.setErrorReporter(errorReporter);
    std::vector<Token> toks = lx.Tokenize(code);
    Parser p(toks); if (errorReporter) p.setErrorReporter(errorReporter);
    std::vector<std::shared_ptr<Stmt>> stmts = p.parse();

    // Isolated environment
    auto saved = getEnvironment();
    auto modEnv = std::make_shared<Environment>(saved);
    modEnv->setErrorReporter(errorReporter);
    setEnvironment(modEnv);
    
    // Set the module filename for error reporting context
    if (errorReporter) {
        errorReporter->loadSource(code, key);
        errorReporter->setCurrentModule(key);
    }

    // Execute
    executor->interpret(stmts);

    // Build module object from env
    std::unordered_map<std::string, Value> exported = modEnv->getAll();
    // Derive module name from key basename
    std::string modName = key;
    size_t pos = modName.find_last_of("/\\"); if (pos != std::string::npos) modName = modName.substr(pos+1);
    if (modName.size() > 4 && modName.substr(modName.size()-4) == ".bob") modName = modName.substr(0, modName.size()-4);
    auto m = std::make_shared<Module>(modName, exported);
    Value moduleVal(m);
    // Cache
    moduleCache[key] = moduleVal;

    // Restore env and reporter
    setEnvironment(saved);
    // Clear module context and pop source context only after successful module execution
    if (errorReporter) {
        errorReporter->clearCurrentModule();
        errorReporter->popSource();
    }
    return moduleVal;
}

bool Interpreter::fromImport(const std::string& spec, const std::vector<std::pair<std::string, std::string>>& items, int line, int column) {
    Value mod = importModule(spec, line, column);
    if (!(mod.isModule() || mod.isDict())) {
        reportError(line, column, "Import Error", "Module did not evaluate to a module", spec);
        return false;
    }
    std::unordered_map<std::string, Value> const* src = nullptr;
    std::unordered_map<std::string, Value> temp;
    if (mod.isModule()) {
        // Module exports
        src = mod.asModule()->exports.get();
    } else {
        src = &mod.asDict();
    }
    for (const auto& [name, alias] : items) {
        auto it = src->find(name);
        if (it == src->end()) {
            reportError(line, column, "Import Error", "Name not found in module: " + name, spec);
            return false;
        }
        environment->define(alias, it->second);
    }
    return true;
}



void Interpreter::addFunction(std::shared_ptr<Function> function) {
    functionRegistry.addFunction(function);
    
    // Simply define the function in the environment (no overloading)
    if (environment) {
        environment->define(function->name, Value(function));
    }
}

void Interpreter::addClassMethod(const std::string& className, std::shared_ptr<Function> method) {
    classRegistry.addClassMethodOverload(className, method);
}

void Interpreter::registerClass(const std::string& className, const std::string& parentName) {
    classRegistry.registerClass(className, parentName);
}

std::string Interpreter::getParentClass(const std::string& className) const {
    return classRegistry.getParentClass(className);
}

void Interpreter::setClassTemplate(const std::string& className, const std::unordered_map<std::string, Value>& tmpl) {
    classRegistry.setClassTemplate(className, tmpl);
}

bool Interpreter::getClassTemplate(const std::string& className, std::unordered_map<std::string, Value>& out) const {
    return classRegistry.getClassTemplate(className, out);
}

std::unordered_map<std::string, Value> Interpreter::buildMergedTemplate(const std::string& className) const {
    return classRegistry.buildMergedTemplate(className);
}

void Interpreter::setClassFieldInitializers(const std::string& className, const std::vector<std::pair<std::string, std::shared_ptr<Expr>>>& inits) {
    classRegistry.setClassFieldInitializers(className, inits);
}

bool Interpreter::getClassFieldInitializers(const std::string& className, std::vector<std::pair<std::string, std::shared_ptr<Expr>>>& out) const {
    return classRegistry.getClassFieldInitializers(className, out);
}

void Interpreter::addExtension(const std::string& targetName, const std::string& methodName, std::shared_ptr<Function> fn) {
    extensionRegistry.registerExtension(targetName, methodName, fn);
}

void Interpreter::setErrorReporter(ErrorReporter* reporter) {
    errorReporter = reporter;
    if (environment) {
        environment->setErrorReporter(reporter);
    }
}

void Interpreter::enterTry() {
    tryDepth++;
    if (errorReporter) errorReporter->enterTry();
}

void Interpreter::exitTry() {
    if (tryDepth > 0) tryDepth--;
    if (errorReporter) errorReporter->exitTry();
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
    // Always track last error site
    setLastErrorSite(line, column);
    
    // Use the new error reporter interface
    if (errorReporter) {
        errorReporter->reportError(line, column, errorType, message, lexeme);
    }
}



Value Interpreter::evaluateCallExprInline(const std::shared_ptr<CallExpr>& expression) {

    auto callInfo = analyzeCallExpression(expression);
    auto callee = resolveCallee(callInfo);
    
    // Handle tail call optimization
    if (expression->isTailCall) {
        auto thunk = std::make_shared<Thunk>([this, callee, callInfo]() -> Value {
            return executeCall(callee, callInfo);
        });
        
        functionRegistry.addThunk(thunk);
        return Value(thunk);
    } else {
        return executeCall(callee, callInfo);
    }
}



CallInfo Interpreter::analyzeCallExpression(const std::shared_ptr<CallExpr>& expression) {
    CallInfo info;
    info.line = expression->paren.line;
    info.column = expression->paren.column;
    
    // Check if this is a method call (obj.method())
    if (auto prop = std::dynamic_pointer_cast<PropertyExpr>(expression->callee)) {
        if (auto varObj = std::dynamic_pointer_cast<VarExpr>(prop->object)) {
            if (varObj->name.lexeme == "super") {
                info.isSuperCall = true;
                if (environment) {
                    try {
                        info.receiver = environment->get(Token{IDENTIFIER, "this", prop->name.line, prop->name.column});
                    } catch (...) {
                        info.receiver = NONE_VALUE;
                    }
                }
            }
        }
        if (!info.isSuperCall) {
            info.receiver = evaluate(prop->object);
        }
        info.methodName = prop->name.lexeme;
        info.isMethodCall = true;
    }

    // Evaluate the callee (function to call)
    if (!info.isSuperCall) {
        info.callee = evaluate(expression->callee);
    }

    // Evaluate all arguments
    for (const auto& argument : expression->arguments) {
        info.arguments.push_back(evaluate(argument));
    }
    
    return info;
}

Value Interpreter::resolveCallee(const CallInfo& callInfo) {
    Value callee = callInfo.callee;
    
    // Handle super call resolution
    if (callInfo.isSuperCall && callInfo.receiver.isDict()) {
        // Check if this is a special super object from extension methods
        const auto& d = callInfo.receiver.asDict();
        auto superIt = d.find("__super");
        if (superIt != d.end() && superIt->second.isBoolean() && superIt->second.asBoolean()) {
            // This is a super object from an extension method
            auto classIt = d.find("__class");
            if (classIt != d.end() && classIt->second.isString()) {
                std::string parentClass = classIt->second.asString();

                
                // Look for the method in the parent class
                std::unordered_map<std::string, Value> tmpl;
                if (getClassTemplate(parentClass, tmpl)) {
                    auto vIt = tmpl.find(callInfo.methodName);
                    if (vIt != tmpl.end() && vIt->second.isFunction()) {

                        callee = vIt->second;
                    }
                }
                if (!callee.isFunction()) {
                    // Try extension methods on the parent class
                    if (auto fn = extensionRegistry.lookupExtension(parentClass, callInfo.methodName)) {

                        callee = Value(fn);
                    }
                }
            }
        } else {
        // Resolve using the current executing class context when available
        std::string curClass;
        if (environment) {
            try {
                Value cc = environment->get(Token{IDENTIFIER, "__currentClass", callInfo.line, callInfo.column});
                if (cc.isString()) curClass = cc.asString();
            } catch (...) {}
        }
        // If still not set, try using the callee's ownerClass (when a direct function ref)
        if (curClass.empty() && callee.isFunction()) {
            Function* cf = callee.asFunction();
            if (cf && !cf->ownerClass.empty()) curClass = cf->ownerClass;
        }
        // Fallback to the receiver's class tag
        if (curClass.empty()) {
            const auto& d = callInfo.receiver.asDict();
            auto itc = d.find("__class");
            if (itc != d.end() && itc->second.isString()) curClass = itc->second.asString();
        }

        std::string cur = getParentClass(curClass);

        int guard = 0;
        while (!cur.empty() && guard++ < 64) {
            std::unordered_map<std::string, Value> tmpl;
            if (getClassTemplate(cur, tmpl)) {
                auto vIt = tmpl.find(callInfo.methodName);
                if (vIt != tmpl.end() && vIt->second.isFunction()) {

                    callee = vIt->second;
                    break;
                }
            }
            if (auto fn = extensionRegistry.lookupExtension(cur, callInfo.methodName)) {

                callee = Value(fn);
                break;
            }
            cur = getParentClass(cur);

        }
        // If still not found, try built-in fallbacks to keep behavior consistent
        if (!callee.isFunction()) {
            if (auto dictFn = extensionRegistry.lookupExtension("dict", callInfo.methodName)) callee = Value(dictFn);
            else if (auto anyFn = extensionRegistry.lookupExtension("any", callInfo.methodName)) callee = Value(anyFn);
        }
        }
    }
    // If property wasn't found as a callable, try extension lookup
    else if (callInfo.isMethodCall && !(callee.isFunction() || callee.isBuiltinFunction())) {
        if (callInfo.isSuperCall && !callInfo.receiver.isDict()) {
            std::string errorMsg = "super can only be used inside class methods";
            if (errorReporter) {
                errorReporter->reportError(callInfo.line, callInfo.column, "Runtime Error",
                    errorMsg, "");
            }
            throw std::runtime_error(errorMsg);
        }
        
        if (!callInfo.methodName.empty()) {
            callee = resolveExtensionMethod(callInfo.receiver, callInfo.methodName);
        }
    }
    
    // Allow dispatchers (builtin) and plain functions
            if (!(callee.isFunction() || callee.isBuiltinFunction())) {
            std::string errorMsg = callInfo.isSuperCall ? ("Undefined super method '" + callInfo.methodName + "'")
                                                       : ("Can only call functions, got " + callee.getType());
            if (errorReporter) {
                // Try to load module source for better error reporting
                if (errorReporter->getCurrentFileName().empty()) {
                    // If no current filename, try to find the module filename from the call context
                    // This is a fallback - ideally the filename should be set by the module execution context
                }
                errorReporter->reportError(callInfo.line, callInfo.column, "Runtime Error",
                    errorMsg, "");
            }
            throw std::runtime_error(errorMsg);
        }
    
    return callee;
}

Value Interpreter::resolveExtensionMethod(const Value& receiver, const std::string& methodName) {
    // Built-ins direct
    if (receiver.isArray()) {
        if (auto fn = extensionRegistry.lookupExtension("array", methodName)) return Value(fn);
        else if (methodName == "len") {
            return createBuiltinMethod("array", "len", receiver);
        } else if (methodName == "push") {
            return createBuiltinMethod("array", "push", receiver);
        } else if (methodName == "pop") {
            return createBuiltinMethod("array", "pop", receiver);
        } else if (auto anyFn = extensionRegistry.lookupExtension("any", methodName)) return Value(anyFn);
    } else if (receiver.isString()) {
        if (auto fn = extensionRegistry.lookupExtension("string", methodName)) return Value(fn);
        else if (methodName == "len") {
            return createBuiltinMethod("string", "len", receiver);
        } else if (auto anyFn = extensionRegistry.lookupExtension("any", methodName)) return Value(anyFn);
    } else if (receiver.isNumber()) {
        if (auto fn = extensionRegistry.lookupExtension("number", methodName)) return Value(fn);
        else if (methodName == "toInt") {
            return createBuiltinMethod("number", "toInt", receiver);
        } else if (auto anyFn = extensionRegistry.lookupExtension("any", methodName)) return Value(anyFn);
    } else if (receiver.isDict()) {
        const auto& d = receiver.asDict();
        std::string cls;
        auto it = d.find("__class");
        if (it != d.end() && it->second.isString()) cls = it->second.asString();
        
        // Walk class chain first
        std::string cur = cls;
        while (!cur.empty()) {
            if (auto fn = extensionRegistry.lookupExtension(cur, methodName)) { 
                return Value(fn); 
            }
            cur = getParentClass(cur);
        }
        
        // Fallbacks
        if (auto dictFn = extensionRegistry.lookupExtension("dict", methodName)) return Value(dictFn);
        else if (methodName == "len") {
            return createBuiltinMethod("dict", "len", receiver);
        } else if (methodName == "keys") {
            return createBuiltinMethod("dict", "keys", receiver);
        } else if (methodName == "values") {
            return createBuiltinMethod("dict", "values", receiver);
        } else if (methodName == "has") {
            return createBuiltinMethod("dict", "has", receiver);
        } else if (auto anyFn = extensionRegistry.lookupExtension("any", methodName)) return Value(anyFn);
    } else {
        if (auto anyFn = extensionRegistry.lookupExtension("any", methodName)) return Value(anyFn);
    }
    
    return NONE_VALUE;
}

Value Interpreter::createBuiltinMethod(const std::string& type, const std::string& methodName, const Value& receiver) {
    if (type == "array") {
        if (methodName == "len") {
            auto bf = std::make_shared<BuiltinFunction>("array.len", [receiver](std::vector<Value> args, int, int){
                return Value(static_cast<double>(receiver.asArray().size()));
            });
            return Value(bf);
        } else if (methodName == "push") {
            auto bf = std::make_shared<BuiltinFunction>("array.push", [receiver](std::vector<Value> args, int, int){
                std::vector<Value>& arr = const_cast<std::vector<Value>&>(receiver.asArray());
                for (size_t i = 0; i < args.size(); ++i) arr.push_back(args[i]);
                return receiver;
            });
            return Value(bf);
        } else if (methodName == "pop") {
            auto bf = std::make_shared<BuiltinFunction>("array.pop", [receiver](std::vector<Value> args, int, int){
                std::vector<Value>& arr = const_cast<std::vector<Value>&>(receiver.asArray());
                if (arr.empty()) return NONE_VALUE;
                Value v = arr.back();
                arr.pop_back();
                return v;
            });
            return Value(bf);
        }
    } else if (type == "string") {
        if (methodName == "len") {
            auto bf = std::make_shared<BuiltinFunction>("string.len", [receiver](std::vector<Value> args, int, int){
                return Value(static_cast<double>(receiver.asString().length()));
            });
            return Value(bf);
        }
    } else if (type == "number") {
        if (methodName == "toInt") {
            auto bf = std::make_shared<BuiltinFunction>("number.toInt", [receiver](std::vector<Value> args, int, int){
                return Value(static_cast<double>(static_cast<long long>(receiver.asNumber())));
            });
            return Value(bf);
        }
    } else if (type == "dict") {
        if (methodName == "len") {
            auto bf = std::make_shared<BuiltinFunction>("dict.len", [receiver](std::vector<Value> args, int, int){
                return Value(static_cast<double>(receiver.asDict().size()));
            });
            return Value(bf);
        } else if (methodName == "keys") {
            auto bf = std::make_shared<BuiltinFunction>("dict.keys", [receiver](std::vector<Value> args, int, int){
                std::vector<Value> keys; const auto& m = receiver.asDict();
                for (const auto& kv : m) keys.push_back(Value(kv.first));
                return Value(keys);
            });
            return Value(bf);
        } else if (methodName == "values") {
            auto bf = std::make_shared<BuiltinFunction>("dict.values", [receiver](std::vector<Value> args, int, int){
                std::vector<Value> vals; const auto& m = receiver.asDict();
                for (const auto& kv : m) vals.push_back(kv.second);
                return Value(vals);
            });
            return Value(bf);
        } else if (methodName == "has") {
            auto bf = std::make_shared<BuiltinFunction>("dict.has", [receiver](std::vector<Value> args, int, int){
                if (args.size() != 1 || !args[0].isString()) return Value(false);
                const auto& m = receiver.asDict();
                return Value(m.find(args[0].asString()) != m.end());
            });
            return Value(bf);
        }
    }
    
    return NONE_VALUE;
}

Value Interpreter::executeCall(const Value& callee, const CallInfo& callInfo) {
    if (callee.isBuiltinFunction()) {
        // Handle builtin functions with direct evaluation
        BuiltinFunction* builtinFunction = callee.asBuiltinFunction();
        return builtinFunction->func(callInfo.arguments, callInfo.line, callInfo.column);
    }

    // Hold a shared_ptr to keep the function alive across tail calls
    std::shared_ptr<Function> functionShared;
    if (callee.isFunction()) {
        functionShared = callee.function;
    }
    Function* function = callee.asFunction();
    
    // Builtin targets: select extension overload by arity
    // For super calls, we must NOT re-dispatch by arity here; keep the resolved callee (parent method)
    if (callInfo.isMethodCall && !callInfo.isSuperCall && !callInfo.receiver.isDict()) {
        std::string target;
        if (callInfo.receiver.isString()) target = "string";
        else if (callInfo.receiver.isArray()) target = "array";
        else if (callInfo.receiver.isDict()) target = "dict";
        else if (callInfo.receiver.isNumeric()) target = "number";
        if (!target.empty()) {
            if (auto sel = extensionRegistry.lookupExtensionOverload(target, callInfo.methodName, callInfo.arguments.size())) {
                functionShared = sel;
                function = sel.get();
            } else {
                if (auto any = extensionRegistry.lookupExtensionOverload("any", callInfo.methodName, callInfo.arguments.size())) { 
                    functionShared = any; 
                    function = any.get(); 
                }
            }
        }
    }
    
    // Method overloading by arity: prefer class overloads if receiver is a user instance; otherwise builtin/any extensions
    // For super calls, do not override the callee selected from the parent chain above
    if (callInfo.isMethodCall && !callInfo.isSuperCall) {
        bool resolved = false;
        if (callInfo.receiver.isDict()) {
            const auto& d = callInfo.receiver.asDict();
            auto itc = d.find("__class");
            if (itc != d.end() && itc->second.isString()) {
                const std::string& cls = itc->second.asString(); // Use reference to avoid copy
                if (auto sel = classRegistry.lookupClassMethodOverload(cls, callInfo.methodName, callInfo.arguments.size())) { 
                    functionShared = sel; 
                    function = sel.get(); 
                    resolved = true; 
                }
            }
        }
        if (!resolved) {
            std::string target;
            if (callInfo.receiver.isString()) target = "string";
            else if (callInfo.receiver.isArray()) target = "array";
            else if (callInfo.receiver.isDict()) target = "dict";
            else if (callInfo.receiver.isNumeric()) target = "number";
            if (!target.empty()) {
                if (auto sel = extensionRegistry.lookupExtensionOverload(target, callInfo.methodName, callInfo.arguments.size())) { 
                    functionShared = sel; 
                    function = sel.get(); 
                    resolved = true; 
                }
                else if (auto any = extensionRegistry.lookupExtensionOverload("any", callInfo.methodName, callInfo.arguments.size())) { 
                    functionShared = any; 
                    function = any.get(); 
                    resolved = true; 
                }
            }
        }
    }
    
    // Check arity (like original)
    if (callInfo.arguments.size() != function->params.size()) {
        if (errorReporter) {
            errorReporter->reportError(callInfo.line, callInfo.column, "Runtime Error",
                "Expected " + std::to_string(function->params.size()) +
                " arguments but got " + std::to_string(callInfo.arguments.size()) + ".", "");
        }
        throw std::runtime_error("Expected " + std::to_string(function->params.size()) +
                               " arguments but got " + std::to_string(callInfo.arguments.size()) + ".");
    }
    

    
    // Regular function execution
    ScopedEnv _env(environment);
    environment = std::make_shared<Environment>(function->closure);
    environment->setErrorReporter(errorReporter);
    
    // Restore module context for error reporting if this function is from a module
    std::string moduleContext;
    if (errorReporter && !function->sourceModule.empty()) {
        moduleContext = errorReporter->getCurrentModule();
        errorReporter->setCurrentModule(function->sourceModule);
        errorReporter->loadModuleSourceForError(function->sourceModule);
    }
    if (callInfo.isMethodCall) {
        environment->define("this", callInfo.receiver);
        if (callInfo.isSuperCall) environment->define("super", callInfo.receiver);
        if (function && !function->ownerClass.empty()) {
            environment->define("__currentClass", Value(function->ownerClass));
        }
    }
    
    for (size_t i = 0; i < function->params.size(); i++) {
        environment->define(function->params[i], callInfo.arguments[i]);
    }
    
    ExecutionContext context;
    context.isFunctionBody = true;
    
    for (const auto& stmt : function->body) {
        stmt->accept(executor.get(), &context);
        if (context.hasThrow) { 
            setPendingThrow(context.thrownValue, context.throwLine, context.throwColumn); 
            return NONE_VALUE; 
        }
        if (context.hasReturn) { 
            return context.returnValue; 
        }
    }
    
    // Restore module context if it was preserved
    if (errorReporter && !moduleContext.empty()) {
        errorReporter->setCurrentModule(moduleContext);
    }
    
    return context.returnValue;
}