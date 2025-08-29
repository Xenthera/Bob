//

//
#include "bob.h"
#include "Interpreter.h"

int main(int argc, char* argv[]){
    Bob bobLang;
    // Enable open preset (all builtins, file imports allowed)
    bobLang.setSafetyPreset("open");
    
    // Collect all command-line arguments early
    std::vector<std::string> allArgs;
    for (int i = 1; i < argc; ++i) {
        allArgs.push_back(argv[i]);
    }

    if(argc > 1) {
        std::string firstArg = argv[1];
        
        // Handle --version flag
        if (firstArg == "--version") {
            std::cout << "Bob v0.0.3 (2025)" << std::endl;
            std::cout << "A fast, embeddable programming language" << std::endl;
            return 0;
        }
        
        // Handle --help flag
        if (firstArg == "--help" || firstArg == "-h") {
            std::cout << "Usage: bob [options] [script] [arguments...]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --version     Show version information" << std::endl;
            std::cout << "  --help, -h    Show this help message" << std::endl;
            std::cout << "  -e <code>     Execute code string" << std::endl;
            std::cout << "  -i            Force interactive mode" << std::endl;
            return 0;
        }
        
        // Handle -e flag for executing code
        if (firstArg == "-e" && argc > 2) {
            std::vector<std::string> args;
            for (int i = 3; i < argc; ++i) {
                args.push_back(argv[i]);
            }
            bobLang.registerModule("__configure_sys_argv__", [args, execPath = std::string(argv[0])](ModuleRegistry::ModuleBuilder& m){
                m.interpreterRef.setArgv(args, execPath);
            });
            bobLang.evalString(argv[2]);
            return 0;
        }
        
        // Handle install command
        if (firstArg == "install" && argc > 2) {
            std::string modulePath = argv[2];
            try {
                bobLang.installModule(modulePath);
                return 0;
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                return 1;
            }
        }
        
        // Handle -i flag for interactive mode
        if (firstArg == "-i") {
            std::vector<std::string> args;
            for (int i = 2; i < argc; ++i) {
                args.push_back(argv[i]);
            }
            bobLang.registerModule("__configure_sys_argv__", [args, execPath = std::string(argv[0])](ModuleRegistry::ModuleBuilder& m){
                m.interpreterRef.setArgv(args, execPath);
            });
            bobLang.runPrompt();
            return 0;
        }
        
        // Regular script execution
        std::vector<std::string> args;
        for (int i = 2; i < argc; ++i) {
            args.push_back(argv[i]);
        }
        bobLang.registerModule("__configure_sys_argv__", [args, execPath = std::string(argv[0])](ModuleRegistry::ModuleBuilder& m){
            m.interpreterRef.setArgv(args, execPath);
        });
        bobLang.runFile(argv[1]);
    } else {
        // For REPL, use interactive mode and seed empty argv
        std::vector<std::string> args;
        bobLang.registerModule("__configure_sys_argv__", [args, execPath = std::string(argv[0])](ModuleRegistry::ModuleBuilder& m){
            m.interpreterRef.setArgv(args, execPath);
        });
        bobLang.runPrompt();
    }

    return 0;
}
