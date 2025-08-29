#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include "TypeWrapper.h"
#include "Value.h"

struct Function;
struct Expr;

/**
 * @class ClassRegistry
 * @brief Manages class registration, inheritance, and method overloading.
 *
 * Handles all aspects of class management including:
 * - Class method overloading by arity
 * - Class inheritance relationships
 * - Class templates and field initializers
 * - Method resolution through inheritance chains
 */
class ClassRegistry {
private:
    // Class method overloading: className -> methodName -> (arity -> function)
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<size_t, std::shared_ptr<Function>>>> classMethodOverloads;
    
    // Class inheritance relationships
    std::unordered_map<std::string, std::string> classParents; // child -> parent
    
    // Class templates for instance creation
    std::unordered_map<std::string, std::unordered_map<std::string, Value>> classTemplates; // className -> template dict
    
    // Field initializers per class in source order (to evaluate across inheritance chain)
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::shared_ptr<Expr>>>> classFieldInitializers; // className -> [(field, expr)]

public:
    ClassRegistry() = default;
    ~ClassRegistry() = default;

    // Method overloading
    void addClassMethodOverload(const std::string& className, std::shared_ptr<Function> function);
    std::shared_ptr<Function> lookupClassMethodOverload(const std::string& className, const std::string& methodName, size_t arity);
    std::shared_ptr<Function> lookupClassMethodDirect(const std::string& className, const std::string& methodName, size_t arity = 0);
    
    // Class inheritance
    void registerClass(const std::string& className, const std::string& parentName);
    std::string getParentClass(const std::string& className) const;
    
    // Class templates
    void setClassTemplate(const std::string& className, const std::unordered_map<std::string, Value>& tmpl);
    bool getClassTemplate(const std::string& className, std::unordered_map<std::string, Value>& out) const;
    std::unordered_map<std::string, Value> buildMergedTemplate(const std::string& className) const;
    
    // Field initializers
    void setClassFieldInitializers(const std::string& className, const std::vector<std::pair<std::string, std::shared_ptr<Expr>>>& inits);
    bool getClassFieldInitializers(const std::string& className, std::vector<std::pair<std::string, std::shared_ptr<Expr>>>& out) const;
};
