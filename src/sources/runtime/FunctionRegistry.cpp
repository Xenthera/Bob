#include "FunctionRegistry.h"
#include "TypeWrapper.h"
#include "Value.h"
#include "Environment.h"
#include "Lexer.h"
#include <algorithm>

void FunctionRegistry::addFunction(std::shared_ptr<Function> function) {
    // Keep legacy list for diagnostics/cleanup
    functions.push_back(function);
}

void FunctionRegistry::addBuiltinFunction(std::shared_ptr<BuiltinFunction> func) {
    builtinFunctions.push_back(func);
}

std::shared_ptr<Function> FunctionRegistry::lookupFunction(const std::string& name, size_t arity) {
    // Simple function lookup (no overloading)
    // This method is kept for interface compatibility but doesn't use arity
    // The actual lookup should be done through the environment
    return nullptr;
}

void FunctionRegistry::addThunk(std::shared_ptr<Thunk> thunk) {
    thunks.push_back(thunk);
    thunkCreationCount++;
    
    // Automatic cleanup when threshold is reached
    if (thunkCreationCount >= CLEANUP_THRESHOLD) {
        cleanupUnusedThunks();
        thunkCreationCount = 0;
    }
}

void FunctionRegistry::cleanupUnusedFunctions() {
    diagnostics.cleanupUnusedFunctions(functions);
}

void FunctionRegistry::cleanupUnusedThunks() {
    diagnostics.cleanupUnusedThunks(thunks);
}

void FunctionRegistry::forceCleanup() {
    diagnostics.forceCleanup(builtinFunctions, functions, thunks);
}
