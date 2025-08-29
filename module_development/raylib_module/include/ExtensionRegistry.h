#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include "TypeWrapper.h"

struct Function;

/**
 * @class ExtensionRegistry
 * @brief Manages extension method registration and lookup.
 *
 * Handles all aspects of extension management including:
 * - Built-in type extensions (string, array, dict, number, any)
 * - Class extensions for user-defined types
 * - Extension method lookup and resolution
 */
class ExtensionRegistry {
private:
    // Global extension registries
    std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Function>>> classExtensions;
    std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Function>>> builtinExtensions; // keys: "string","array","dict","any"

public:
    ExtensionRegistry() = default;
    ~ExtensionRegistry() = default;

    // Extension registration
    void registerExtension(const std::string& targetName, const std::string& methodName, std::shared_ptr<Function> fn);
    
    // Extension lookup
    std::shared_ptr<Function> lookupExtension(const std::string& targetName, const std::string& methodName);
    std::shared_ptr<Function> lookupExtensionOverload(const std::string& targetName, const std::string& methodName, size_t arity);
    
    // Getters for diagnostics and legacy support
    const std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Function>>>& getClassExtensions() const { return classExtensions; }
    const std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Function>>>& getBuiltinExtensions() const { return builtinExtensions; }
};
