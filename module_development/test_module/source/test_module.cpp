#include "BobModuleSDK.h"
#include <iostream>
#include <string>
#include <vector>

class TestModule : public ModuleDef {
private:
    std::string name;

public:
    TestModule() : name("test_module") {}

    std::string getName() const override {
        return name;
    }

    void registerModule(class Interpreter& interpreter) override {
        interpreter.registerModule("test_module", [this](Interpreter::ModuleBuilder& m) {
            // Simple test function
            m.fn("hello", [](std::vector<Value> args, int line, int column) -> Value {
                return Value("Hello from test module!");
            });
            
            // Simple test value
            m.val("version", Value("1.0.0"));
        });
    }
};

extern "C" ModuleDef* createModule() {
    return new TestModule();
}
