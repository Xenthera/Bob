#pragma once

#include <string>
#include <vector>
#include <map>

struct ModuleManifest {
    std::string name;
    std::string version;
    std::string type;  // "cpp" or "bob"
    std::string description;
    std::string author;
    std::string license;
    
    struct Dependencies {
        std::vector<std::string> system;  // System libraries like "raylib", "cjson"
    } dependencies;
    
    struct Build {
        std::string type;  // "cmake" or "simple"
        std::string source;  // Path to source file relative to module root
        std::vector<std::string> libraries;  // Libraries to link against
        std::string compiler;  // Compiler to use (optional)
        std::vector<std::string> flags;  // Compiler flags (optional)
    } build;
    
    // Parse manifest from JSON string
    static ModuleManifest parse(const std::string& jsonStr);
    
    // Validate manifest
    bool isValid() const;
    
    // Get platform-specific library extension
    std::string getLibraryExtension() const;
};
