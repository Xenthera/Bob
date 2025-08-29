#include "ModuleRegistry.h"
#include "CppModuleLoader.h"
#include <filesystem>

bool ModuleRegistry::isInstalledModule(const std::string& name) const {
    return CppModuleLoader::isModuleInstalled(name, modulesDirectory);
}

std::shared_ptr<ModuleDef> ModuleRegistry::loadInstalledModule(const std::string& name) {
    return CppModuleLoader::loadModule(name, modulesDirectory);
}
