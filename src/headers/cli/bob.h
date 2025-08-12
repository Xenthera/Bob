#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "Lexer.h"
#include "Interpreter.h"
#include "ModuleRegistry.h"
#include "helperFunctions/ShortHands.h"
#include "ErrorReporter.h"

#define VERSION "0.0.3"

class Bob
{
public:
    Lexer lexer;
    sptr(Interpreter) interpreter;
    ErrorReporter errorReporter;

    ~Bob() = default;

public:
    // Embedding helpers (bridge to internal interpreter)
    void registerModule(const std::string& name, std::function<void(ModuleRegistry::ModuleBuilder&)> init) {
        if (interpreter) interpreter->registerModule(name, init);
        else pendingConfigurators.push_back([name, init](Interpreter& I){ I.registerModule(name, init); });
    }
    void setBuiltinModulePolicy(bool allow) {
        if (interpreter) interpreter->setBuiltinModulePolicy(allow);
        else pendingConfigurators.push_back([allow](Interpreter& I){ I.setBuiltinModulePolicy(allow); });
    }
    void setBuiltinModuleAllowList(const std::vector<std::string>& allowed) {
        if (interpreter) interpreter->setBuiltinModuleAllowList(allowed);
        else pendingConfigurators.push_back([allowed](Interpreter& I){ I.setBuiltinModuleAllowList(allowed); });
    }
    void setBuiltinModuleDenyList(const std::vector<std::string>& denied) {
        if (interpreter) interpreter->setBuiltinModuleDenyList(denied);
        else pendingConfigurators.push_back([denied](Interpreter& I){ I.setBuiltinModuleDenyList(denied); });
    }
    bool defineGlobal(const std::string& name, const Value& v) {
        if (interpreter) return interpreter->defineGlobalVar(name, v);
        pendingConfigurators.push_back([name, v](Interpreter& I){ I.defineGlobalVar(name, v); });
        return true;
    }
    bool tryGetGlobal(const std::string& name, Value& out) const { return interpreter ? interpreter->tryGetGlobalVar(name, out) : false; }
    void runFile(const std::string& path);
    void runPrompt();
    bool evalFile(const std::string& path);
    bool evalString(const std::string& code, const std::string& filename = "<eval>");

    // Safety policy helpers (public API)
    // Set all safety-related policies at once
    void setSafetyPolicy(
        bool allowBuiltins,
        const std::vector<std::string>& allowList,
        const std::vector<std::string>& denyList,
        bool allowFileImports,
        bool preferFileOverBuiltin,
        const std::vector<std::string>& searchPaths
    ) {
        if (interpreter) {
            interpreter->setBuiltinModulePolicy(allowBuiltins);
            interpreter->setBuiltinModuleAllowList(allowList);
            interpreter->setBuiltinModuleDenyList(denyList);
            interpreter->setModulePolicy(allowFileImports, preferFileOverBuiltin, searchPaths);
        } else {
            pendingConfigurators.push_back([=](Interpreter& I){
                I.setBuiltinModulePolicy(allowBuiltins);
                I.setBuiltinModuleAllowList(allowList);
                I.setBuiltinModuleDenyList(denyList);
                I.setModulePolicy(allowFileImports, preferFileOverBuiltin, searchPaths);
            });
        }
    }

    // Simple presets: "open", "safe", "locked"
    void setSafetyPreset(const std::string& preset) {
        if (preset == "open") {
            setSafetyPolicy(
                true, /* allowBuiltins */
                {},   /* allowList -> empty means allow all */
                {},
                true, /* allowFileImports */
                true, /* preferFileOverBuiltin */
                {}    /* searchPaths */
            );
        } else if (preset == "safe") {
            // Allow only pure/harmless modules by default
            setSafetyPolicy(
                true,
                std::vector<std::string>{
                    "sys", "time", "rand", "math", "path", "base64"
                },
                std::vector<std::string>{ /* denyList empty when allowList is used */ },
                false, /* disallow file-based imports */
                true,
                {}
            );
        } else if (preset == "locked") {
            // No builtins visible; no file imports
            setSafetyPolicy(
                false,
                {},
                {},
                false,
                true,
                {}
            );
        } else {
            // Default to safe
            setSafetyPreset("safe");
        }
    }

private:
    void ensureInterpreter(bool interactive);
    void applyPendingConfigs() {
        if (!interpreter) return;
        for (auto& f : pendingConfigurators) { f(*interpreter); }
        pendingConfigurators.clear();
    }
    std::vector<std::function<void(Interpreter&)>> pendingConfigurators;
};

