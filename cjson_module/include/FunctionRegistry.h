#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include "TypeWrapper.h"
#include "RuntimeDiagnostics.h"

struct Function;
struct BuiltinFunction;
struct Thunk;

/**
 * @class FunctionRegistry
 * @brief Manages function registration, lookup, and cleanup.
 *
 * Handles all aspects of function management including:
 * - Built-in function registration and storage
 * - User function registration and storage
 * - Thunk management for TCO
 * - Function cleanup and memory management
 */
class FunctionRegistry {
private:
    std::vector<std::shared_ptr<BuiltinFunction>> builtinFunctions;
    std::vector<std::shared_ptr<Function>> functions;
    std::vector<std::shared_ptr<Thunk>> thunks;
    
    // Automatic cleanup tracking
    int thunkCreationCount = 0;
    static const int CLEANUP_THRESHOLD = 10000;
    
    RuntimeDiagnostics diagnostics;

public:
    FunctionRegistry() = default;
    ~FunctionRegistry() = default;

    // Function management
    void addFunction(std::shared_ptr<Function> function);
    void addBuiltinFunction(std::shared_ptr<BuiltinFunction> func);
    std::shared_ptr<Function> lookupFunction(const std::string& name, size_t arity);
    
    // Thunk management
    void addThunk(std::shared_ptr<Thunk> thunk);
    void cleanupUnusedFunctions();
    void cleanupUnusedThunks();
    void forceCleanup();
    
    // Getters for diagnostics and legacy support
    const std::vector<std::shared_ptr<BuiltinFunction>>& getBuiltinFunctions() const { return builtinFunctions; }
    const std::vector<std::shared_ptr<Function>>& getFunctions() const { return functions; }
    const std::vector<std::shared_ptr<Thunk>>& getThunks() const { return thunks; }
};
