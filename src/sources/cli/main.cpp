//

//
#include "bob.h"
#include "Interpreter.h"

int main(int argc, char* argv[]){
    Bob bobLang;
    // Example: host can register a custom module via Bob bridge (applied on first use)
    bobLang.registerModule("demo", [](ModuleRegistry::ModuleBuilder& m) {
        m.fn("hello", [](std::vector<Value> a, int, int) -> Value {
            std::string who = (a.size() >= 1 && a[0].isString()) ? a[0].asString() : std::string("world");
            return Value(std::string("hello ") + who);
        });
        m.val("meaning", Value(42.0));
    });
    //bobLang.setBuiltinModuleDenyList({"sys"});

    if(argc > 1) {
        bobLang.runFile(argv[1]);
    } else {
        // For REPL, use interactive mode
        bobLang.runPrompt();
    }

    return 0;
}
