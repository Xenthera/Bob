#include "ClassRegistry.h"
#include "TypeWrapper.h"
#include "Value.h"
#include "Expression.h"

void ClassRegistry::addClassMethodOverload(const std::string& className, std::shared_ptr<Function> function) {
    classMethodOverloads[className][function->name][function->params.size()] = function;
}

std::shared_ptr<Function> ClassRegistry::lookupClassMethodOverload(const std::string& className, const std::string& methodName, size_t arity) {
    // Walk inheritance chain from className upward
    std::string cur = className;
    int guard = 0;
    while (!cur.empty() && guard++ < 256) {
        auto classIt = classMethodOverloads.find(cur);
        if (classIt != classMethodOverloads.end()) {
            auto methodIt = classIt->second.find(methodName);
            if (methodIt != classIt->second.end()) {
                auto arityIt = methodIt->second.find(arity);
                if (arityIt != methodIt->second.end()) {
                    return arityIt->second;
                }
            }
        }
        cur = getParentClass(cur);
    }
    return nullptr;
}

std::shared_ptr<Function> ClassRegistry::lookupClassMethodDirect(const std::string& className, const std::string& methodName, size_t arity) {
    // Only look for methods defined on the current class, not inherited ones
    auto classIt = classMethodOverloads.find(className);
    if (classIt != classMethodOverloads.end()) {
        auto methodIt = classIt->second.find(methodName);
        if (methodIt != classIt->second.end()) {
            auto arityIt = methodIt->second.find(arity);
            if (arityIt != methodIt->second.end()) {
                return arityIt->second;
            }
        }
    }
    return nullptr;
}



void ClassRegistry::registerClass(const std::string& className, const std::string& parentName) {
    classParents[className] = parentName;
}

std::string ClassRegistry::getParentClass(const std::string& className) const {
    auto it = classParents.find(className);
    return it != classParents.end() ? it->second : "";
}

void ClassRegistry::setClassTemplate(const std::string& className, const std::unordered_map<std::string, Value>& tmpl) {
    classTemplates[className] = tmpl;
}

bool ClassRegistry::getClassTemplate(const std::string& className, std::unordered_map<std::string, Value>& out) const {
    auto it = classTemplates.find(className);
    if (it == classTemplates.end()) return false;
    out = it->second;
    return true;
}

std::unordered_map<std::string, Value> ClassRegistry::buildMergedTemplate(const std::string& className) const {
    std::unordered_map<std::string, Value> merged;
    
    // Walk inheritance chain and merge templates
    std::string cur = className;
    int guard = 0;
    while (!cur.empty() && guard++ < 256) {
        auto it = classTemplates.find(cur);
        if (it != classTemplates.end()) {
            // Merge parent template into merged (parent fields come first)
            for (const auto& pair : it->second) {
                if (merged.find(pair.first) == merged.end()) {
                    merged[pair.first] = pair.second;
                }
            }
        }
        cur = getParentClass(cur);
    }
    
    return merged;
}

void ClassRegistry::setClassFieldInitializers(const std::string& className, const std::vector<std::pair<std::string, std::shared_ptr<Expr>>>& inits) {
    classFieldInitializers[className] = inits;
}

bool ClassRegistry::getClassFieldInitializers(const std::string& className, std::vector<std::pair<std::string, std::shared_ptr<Expr>>>& out) const {
    auto it = classFieldInitializers.find(className);
    if (it == classFieldInitializers.end()) return false;
    out = it->second;
    return true;
}
