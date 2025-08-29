#include "CppModuleLoader.h"
#include "ModuleManifest.h"
#include "ModuleDef.h"
#include "FunctionRegistry.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <dlfcn.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
    #define RTLD_LAZY 0
    #define dlopen(path, flags) LoadLibraryA(path.c_str())
    #define dlsym(handle, symbol) GetProcAddress((HMODULE)handle, symbol)
    #define dlclose(handle) FreeLibrary((HMODULE)handle)
    #define dlerror() "Windows DLL error"
#else
    #include <unistd.h>
    #include <sys/stat.h>
#endif

void CppModuleLoader::installModule(const std::string& zipPath, const std::string& modulesDirectory) {
    try {
        // 1. Extract zip to temporary directory
        std::string tempDir = extractZip(zipPath);
        
        // 2. Parse manifest
        // Try to find manifest.json in the extracted directory
        std::string manifestPath = tempDir + "/manifest.json";
        std::ifstream manifestFile(manifestPath);
        if (!manifestFile.is_open()) {
            // Try looking in subdirectories (common case when zip contains a folder)
            std::filesystem::path tempPath(tempDir);
            for (const auto& entry : std::filesystem::directory_iterator(tempPath)) {
                if (entry.is_directory()) {
                    std::string subManifestPath = entry.path().string() + "/manifest.json";
                    std::ifstream subManifestFile(subManifestPath);
                    if (subManifestFile.is_open()) {
                        manifestPath = subManifestPath;
                        manifestFile = std::move(subManifestFile);
                        break;
                    }
                }
            }
        }
        
        if (!manifestFile.is_open()) {
            throw std::runtime_error("Could not open manifest.json");
        }
        
        std::string manifestContent((std::istreambuf_iterator<char>(manifestFile)), 
                                   std::istreambuf_iterator<char>());
        manifestFile.close();
        
        ModuleManifest manifest = ModuleManifest::parse(manifestContent);
        
        // 3. Validate manifest
        if (!manifest.isValid()) {
            throw std::runtime_error("Invalid module manifest");
        }
        

        
        // 5. Compile module
        std::string libraryPath = compileModule(tempDir, manifest);
        
        // 6. Install to modules directory
        installToModules(manifest.name, libraryPath, modulesDirectory);
        
        // 7. Clean up
        cleanupTemp(tempDir);
        
        std::cout << "Module '" << manifest.name << "' installed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to install module: " + std::string(e.what()));
    }
}

std::shared_ptr<ModuleDef> CppModuleLoader::loadModule(const std::string& name, const std::string& modulesDirectory) {
    std::cout << "DEBUG: loadModule called for " << name << " in " << modulesDirectory << std::endl;
    std::string libraryPath = modulesDirectory + "/" + name + "/lib" + name + getLibraryExtension();
    std::cout << "DEBUG: Looking for library at " << libraryPath << std::endl;
    
    if (!std::filesystem::exists(libraryPath)) {
        std::cout << "DEBUG: Library not found at " << libraryPath << std::endl;
        throw std::runtime_error("Module library not found: " + libraryPath);
    }
    std::cout << "DEBUG: Library found at " << libraryPath << std::endl;
    
    // Load dynamic library
    std::cout << "DEBUG: Loading dynamic library..." << std::endl;
    void* handle = loadDynamicLibrary(libraryPath);
    if (!handle) {
        std::cout << "DEBUG: Failed to load dynamic library" << std::endl;
        throw std::runtime_error("Failed to load module library: " + libraryPath);
    }
    std::cout << "DEBUG: Dynamic library loaded successfully" << std::endl;
    
    // Get module creation function
    std::cout << "DEBUG: Getting createModule function..." << std::endl;
    ModuleDef* module = getCreateModuleFunction(handle);
    if (!module) {
        std::cout << "DEBUG: Failed to get createModule function" << std::endl;
        dlclose(handle);
        throw std::runtime_error("Module does not export 'createModule' function");
    }
    std::cout << "DEBUG: createModule function obtained successfully" << std::endl;
    
    return std::shared_ptr<ModuleDef>(module);
}

bool CppModuleLoader::isModuleInstalled(const std::string& name, const std::string& modulesDirectory) {
    std::string libraryPath = modulesDirectory + "/" + name + "/lib" + name + getLibraryExtension();
    return std::filesystem::exists(libraryPath);
}



std::string CppModuleLoader::getCurrentPlatform() {
    #ifdef _WIN32
        return "windows-x64";
    #elif defined(__APPLE__)
        return "macos-x64";
    #else
        return "linux-x64";
    #endif
}

std::string CppModuleLoader::getLibraryExtension() {
    #ifdef _WIN32
        return ".dll";
    #elif defined(__APPLE__)
        return ".dylib";
    #else
        return ".so";
    #endif
}

std::string CppModuleLoader::extractZip(const std::string& zipPath) {
    // For now, we'll use system unzip command
    // In a real implementation, you'd want a proper zip library
    
    std::string tempDir = "/tmp/bob_module_" + std::to_string(getpid());
    
    // Create temp directory
    if (mkdir(tempDir.c_str(), 0755) != 0) {
        throw std::runtime_error("Failed to create temp directory: " + tempDir);
    }
    
    // Extract zip
    std::string cmd = "unzip -q " + zipPath + " -d " + tempDir;
    int result = system(cmd.c_str());
    
    if (result != 0) {
        cleanupTemp(tempDir);
        throw std::runtime_error("Failed to extract zip file");
    }
    
    return tempDir;
}

std::string CppModuleLoader::compileModule(const std::string& tempDir, const ModuleManifest& manifest) {
    if (manifest.build.type != "cmake") {
        throw std::runtime_error("Only CMake build system is supported");
    }
    
    // Check if CMake is available
    if (system("cmake --version") != 0) {
        throw std::runtime_error("CMake not found. Please install CMake first.");
    }
    
    // Create build directory
    std::string buildDir = tempDir + "/build";
    if (mkdir(buildDir.c_str(), 0755) != 0) {
        throw std::runtime_error("Failed to create build directory");
    }
    
    // Configure with CMake (headers are bundled with module)
    // Find the actual module directory (might be in a subdirectory)
    std::string moduleDir = tempDir;
    std::filesystem::path tempPath(tempDir);
    for (const auto& entry : std::filesystem::directory_iterator(tempPath)) {
        if (entry.is_directory()) {
            std::string subManifestPath = entry.path().string() + "/manifest.json";
            if (std::filesystem::exists(subManifestPath)) {
                moduleDir = entry.path().string();
                break;
            }
        }
    }
    
    std::string configureCmd = "cmake -B " + buildDir + " -S " + moduleDir;
    int result = system(configureCmd.c_str());
    if (result != 0) {
        throw std::runtime_error("CMake configuration failed");
    }
    
    // Build with CMake
    std::string buildCmd = "cmake --build " + buildDir;
    result = system(buildCmd.c_str());
    if (result != 0) {
        throw std::runtime_error("CMake build failed");
    }
    
    // Find the built library
    std::vector<std::string> possiblePaths = {
        buildDir + "/lib" + manifest.name + getLibraryExtension(),
        buildDir + "/" + manifest.name + getLibraryExtension(),
        buildDir + "/Debug/lib" + manifest.name + getLibraryExtension(),
        buildDir + "/Release/lib" + manifest.name + getLibraryExtension()
    };
    
    for (const auto& path : possiblePaths) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    
    throw std::runtime_error("Built library not found in " + buildDir);
}

void CppModuleLoader::installToModules(const std::string& name, const std::string& libraryPath, 
                                      const std::string& modulesDirectory) {
    std::string moduleDir = modulesDirectory + "/" + name + "/";
    
    // Create module directory
    std::filesystem::create_directories(moduleDir);
    
    // Copy library
    std::string targetPath = moduleDir + "lib" + name + getLibraryExtension();
    std::filesystem::copy_file(libraryPath, targetPath, std::filesystem::copy_options::overwrite_existing);
    
    // Copy manifest
    std::string manifestPath = moduleDir + "module.json";
    // We'll need to reconstruct the manifest or copy it from temp dir
}

void CppModuleLoader::cleanupTemp(const std::string& tempDir) {
    std::filesystem::remove_all(tempDir);
}

void* CppModuleLoader::loadDynamicLibrary(const std::string& libraryPath) {
    return dlopen(libraryPath.c_str(), RTLD_LAZY);
}

ModuleDef* CppModuleLoader::getCreateModuleFunction(void* handle) {
    auto createModule = (ModuleDef*(*)())dlsym(handle, "createModule");
    if (!createModule) {
        return nullptr;
    }
    return createModule();
}
