//

//
#include "bob.h"
#include "Interpreter.h"

int main(int argc, char* argv[]){
    Bob bobLang;
    // Enable open preset (all builtins, file imports allowed)
    bobLang.setSafetyPreset("open");
   

    if(argc > 1) {
        // Seed argv/executable for sys module
        std::vector<std::string> args; for (int i = 2; i < argc; ++i) args.emplace_back(argv[i]);
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
