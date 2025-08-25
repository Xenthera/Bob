#include "ExtensionRegistry.h"
#include "TypeWrapper.h"

void ExtensionRegistry::registerExtension(const std::string& targetName, const std::string& methodName, std::shared_ptr<Function> fn) {
    // Simple extension registration (no overloading)
    if (targetName == "string" || targetName == "array" || targetName == "dict" || targetName == "any" || targetName == "number") {
        // Register into simple map for property lookup
        builtinExtensions[targetName][methodName] = fn;
    } else {
        // Class extensions
        classExtensions[targetName][methodName] = fn;
    }
}

std::shared_ptr<Function> ExtensionRegistry::lookupExtension(const std::string& targetName, const std::string& methodName) {
    // Simple extension lookup (no overloading)
    if (targetName == "string" || targetName == "array" || targetName == "dict" || targetName == "any" || targetName == "number") {
        auto tIt = builtinExtensions.find(targetName);
        if (tIt == builtinExtensions.end()) return nullptr;
        auto mIt = tIt->second.find(methodName);
        if (mIt == tIt->second.end()) return nullptr;
        return mIt->second;
    } else {
        // Class extensions
        auto tIt = classExtensions.find(targetName);
        if (tIt == classExtensions.end()) return nullptr;
        auto mIt = tIt->second.find(methodName);
        if (mIt == tIt->second.end()) return nullptr;
        return mIt->second;
    }
}

std::shared_ptr<Function> ExtensionRegistry::lookupExtensionOverload(const std::string& targetName, const std::string& methodName, size_t arity) {
    // Simple extension lookup (no overloading)
    if (targetName == "string" || targetName == "array" || targetName == "dict" || targetName == "any" || targetName == "number") {
        auto tIt = builtinExtensions.find(targetName);
        if (tIt == builtinExtensions.end()) return nullptr;
        auto mIt = tIt->second.find(methodName);
        if (mIt == tIt->second.end()) return nullptr;
        return mIt->second;
    }
    return nullptr;
}
