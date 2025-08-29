#pragma once

#include <string>
#include <memory>
#include <vector>
#include "ModuleManifest.h"
#include "TypeWrapper.h"

class ModuleDef;

class CppModuleLoader {
public:
    // Install a C++ module from a zip file
    static void installModule(const std::string& zipPath, const std::string& modulesDirectory);
    
    // Load a C++ module by name
    static std::shared_ptr<ModuleDef> loadModule(const std::string& name, const std::string& modulesDirectory);
    
    // Check if a C++ module is installed
    static bool isModuleInstalled(const std::string& name, const std::string& modulesDirectory);
    


    
    // Get current platform
    static std::string getCurrentPlatform();
    
    // Get platform-specific library extension
    static std::string getLibraryExtension();

private:
    // Extract zip file to temporary directory
    static std::string extractZip(const std::string& zipPath);
    
    // Compile module using CMake
    static std::string compileModule(const std::string& tempDir, const ModuleManifest& manifest);
    
    // Install compiled library to modules directory
    static void installToModules(const std::string& name, const std::string& libraryPath, 
                                const std::string& modulesDirectory);
    
    // Clean up temporary files
    static void cleanupTemp(const std::string& tempDir);
    
    // Load dynamic library
    static void* loadDynamicLibrary(const std::string& libraryPath);
    
    // Get module creation function from dynamic library
    static ModuleDef* getCreateModuleFunction(void* handle);
};
