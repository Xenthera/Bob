#include "ModuleManifest.h"
#include <iostream>
#include <fstream>
#include <sstream>

#ifdef _WIN32
    #define LIBRARY_EXTENSION ".dll"
#elif defined(__APPLE__)
    #define LIBRARY_EXTENSION ".dylib"
#else
    #define LIBRARY_EXTENSION ".so"
#endif

ModuleManifest ModuleManifest::parse(const std::string& jsonStr) {
    ModuleManifest manifest;
    
    // Simple JSON parsing for now - we can enhance this later
    // For now, just extract basic fields using string operations
    
    // Extract name
    size_t namePos = jsonStr.find("\"name\"");
    if (namePos != std::string::npos) {
        size_t start = jsonStr.find("\"", namePos + 7);
        size_t end = jsonStr.find("\"", start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            manifest.name = jsonStr.substr(start + 1, end - start - 1);
        }
    }
    
    // Extract version
    size_t versionPos = jsonStr.find("\"version\"");
    if (versionPos != std::string::npos) {
        size_t start = jsonStr.find("\"", versionPos + 10);
        size_t end = jsonStr.find("\"", start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            manifest.version = jsonStr.substr(start + 1, end - start - 1);
        }
    }
    
    // Extract type
    size_t typePos = jsonStr.find("\"type\"");
    if (typePos != std::string::npos) {
        size_t start = jsonStr.find("\"", typePos + 7);
        size_t end = jsonStr.find("\"", start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            manifest.type = jsonStr.substr(start + 1, end - start - 1);
        }
    }
    
    // Extract description
    size_t descPos = jsonStr.find("\"description\"");
    if (descPos != std::string::npos) {
        size_t start = jsonStr.find("\"", descPos + 14);
        size_t end = jsonStr.find("\"", start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            manifest.description = jsonStr.substr(start + 1, end - start - 1);
        }
    }
    
    // Extract system dependencies
    size_t depsPos = jsonStr.find("\"system\"");
    if (depsPos != std::string::npos) {
        size_t start = jsonStr.find("[", depsPos);
        size_t end = jsonStr.find("]", start);
        if (start != std::string::npos && end != std::string::npos) {
            std::string depsStr = jsonStr.substr(start + 1, end - start - 1);
            // Parse array of strings
            size_t pos = 0;
            while ((pos = depsStr.find("\"")) != std::string::npos) {
                size_t endPos = depsStr.find("\"", pos + 1);
                if (endPos != std::string::npos) {
                    std::string dep = depsStr.substr(pos + 1, endPos - pos - 1);
                    manifest.dependencies.system.push_back(dep);
                    depsStr = depsStr.substr(endPos + 1);
                } else {
                    break;
                }
            }
        }
    }
    
    // Extract build information
    size_t buildPos = jsonStr.find("\"build\"");
    if (buildPos != std::string::npos) {
        // Extract build type
        size_t buildTypePos = jsonStr.find("\"type\"", buildPos);
        if (buildTypePos != std::string::npos) {
            size_t start = jsonStr.find("\"", buildTypePos + 7);
            size_t end = jsonStr.find("\"", start + 1);
            if (start != std::string::npos && end != std::string::npos) {
                manifest.build.type = jsonStr.substr(start + 1, end - start - 1);
            }
        }
        
        // Extract source
        size_t sourcePos = jsonStr.find("\"source\"", buildPos);
        if (sourcePos != std::string::npos) {
            size_t start = jsonStr.find("\"", sourcePos + 9);
            size_t end = jsonStr.find("\"", start + 1);
            if (start != std::string::npos && end != std::string::npos) {
                manifest.build.source = jsonStr.substr(start + 1, end - start - 1);
            }
        }
        
        // Extract libraries
        size_t libsPos = jsonStr.find("\"libraries\"", buildPos);
        if (libsPos != std::string::npos) {
            size_t start = jsonStr.find("[", libsPos);
            size_t end = jsonStr.find("]", start);
            if (start != std::string::npos && end != std::string::npos) {
                std::string libsStr = jsonStr.substr(start + 1, end - start - 1);
                // Parse array of strings
                size_t pos = 0;
                while ((pos = libsStr.find("\"")) != std::string::npos) {
                    size_t endPos = libsStr.find("\"", pos + 1);
                    if (endPos != std::string::npos) {
                        std::string lib = libsStr.substr(pos + 1, endPos - pos - 1);
                        manifest.build.libraries.push_back(lib);
                        libsStr = libsStr.substr(endPos + 1);
                    } else {
                        break;
                    }
                }
            }
        }
    }
    
    return manifest;
}

bool ModuleManifest::isValid() const {
    if (name.empty()) {
        std::cerr << "Module manifest error: name is required" << std::endl;
        return false;
    }
    
    if (version.empty()) {
        std::cerr << "Module manifest error: version is required" << std::endl;
        return false;
    }
    
    if (type.empty()) {
        std::cerr << "Module manifest error: type is required" << std::endl;
        return false;
    }
    
    if (type != "cpp" && type != "bob") {
        std::cerr << "Module manifest error: type must be 'cpp' or 'bob'" << std::endl;
        return false;
    }
    
    if (type == "cpp") {
        if (build.source.empty()) {
            std::cerr << "Module manifest error: build.source is required for cpp modules" << std::endl;
            return false;
        }
        
        if (build.type.empty()) {
            std::cerr << "Module manifest error: build.type is required for cpp modules" << std::endl;
            return false;
        }
    }
    
    return true;
}

std::string ModuleManifest::getLibraryExtension() const {
    return LIBRARY_EXTENSION;
}
