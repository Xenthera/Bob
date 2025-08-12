#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <memory>
#include "TypeWrapper.h"  // BuiltinFunction, Value

class Interpreter; // fwd

class ModuleRegistry {
public:
    struct ModuleBuilder {
        std::string moduleName;
        Interpreter& interpreterRef;
        std::unordered_map<std::string, Value> exports;
        ModuleBuilder(const std::string& n, Interpreter& i) : moduleName(n), interpreterRef(i) {}
        void fn(const std::string& name, std::function<Value(std::vector<Value>, int, int)> func) {
            exports[name] = Value(std::make_shared<BuiltinFunction>(name, func));
        }
        void val(const std::string& name, const Value& v) { exports[name] = v; }
    };

    using Factory = std::function<Value(Interpreter&)>;

    void registerFactory(const std::string& name, Factory factory) {
        factories[name] = std::move(factory);
    }

    void registerModule(const std::string& name, std::function<void(ModuleBuilder&)> init) {
        registerFactory(name, [name, init](Interpreter& I) -> Value {
            ModuleBuilder b(name, I);
            init(b);
            auto m = std::make_shared<Module>(name, b.exports);
            return Value(m);
        });
    }

    bool has(const std::string& name) const {
        auto it = factories.find(name);
        if (it == factories.end()) return false;
        // Respect policy for presence checks to optionally cloak denied modules
        if (!allowBuiltins) return false;
        if (!allowList.empty() && allowList.find(name) == allowList.end()) return false;
        if (denyList.find(name) != denyList.end()) return false;
        return true;
    }

    Value create(const std::string& name, Interpreter& I) const {
        auto it = factories.find(name);
        if (it == factories.end()) return NONE_VALUE;
        if (!allowBuiltins) return NONE_VALUE;
        if (!allowList.empty() && allowList.find(name) == allowList.end()) return NONE_VALUE;
        if (denyList.find(name) != denyList.end()) return NONE_VALUE;
        return it->second(I);
    }

    void setPolicy(bool allow) { allowBuiltins = allow; }
    void setAllowList(const std::vector<std::string>& allowed) { allowList = std::unordered_set<std::string>(allowed.begin(), allowed.end()); }
    void setDenyList(const std::vector<std::string>& denied) { denyList = std::unordered_set<std::string>(denied.begin(), denied.end()); }

private:
    std::unordered_map<std::string, Factory> factories;
    std::unordered_set<std::string> allowList;
    std::unordered_set<std::string> denyList;
    bool allowBuiltins = true;
};


