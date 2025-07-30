#include "../headers/StdLib.h"
#include "../headers/Interpreter.h"
#include <chrono>

void StdLib::addToEnvironment(sptr(Environment) env, Interpreter* interpreter) {
    // Create a built-in print function
    auto printFunc = std::make_shared<BuiltinFunction>("print", 
        [interpreter](std::vector<std::shared_ptr<Object>> args) -> std::shared_ptr<Object> {
            if (args.size() != 1) {
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            // Use the interpreter's stringify function
            std::cout << interpreter->stringify(args[0]) << std::endl;
            return std::make_shared<None>();
        });
    env->define("print", printFunc);

    // Create a built-in assert function
    auto assertFunc = std::make_shared<BuiltinFunction>("assert",
        [interpreter](std::vector<std::shared_ptr<Object>> args) -> std::shared_ptr<Object> {
            if (args.size() != 1 && args.size() != 2) {
                throw std::runtime_error("Expected 1 or 2 arguments but got " + std::to_string(args.size()) + ".");
            }
            
            // Check if the argument is a boolean and is true
            if (auto boolObj = std::dynamic_pointer_cast<Boolean>(args[0])) {
                if (!boolObj->value) {
                    std::string message = "Assertion failed: condition is false";
                    if (args.size() == 2) {
                        if (auto strObj = std::dynamic_pointer_cast<String>(args[1])) {
                            message += " - " + strObj->value;
                        }
                    }
                    throw std::runtime_error(message);
                }
            } else {
                throw std::runtime_error("Assertion failed: expected boolean condition, got " + interpreter->stringify(args[0]));
            }
            
            return std::make_shared<None>();
        });
    env->define("assert", assertFunc);

    // Create a built-in time function (returns microseconds since Unix epoch)
    auto timeFunc = std::make_shared<BuiltinFunction>("time",
        [](std::vector<std::shared_ptr<Object>> args) -> std::shared_ptr<Object> {
            if (args.size() != 0) {
                throw std::runtime_error("Expected 0 arguments but got " + std::to_string(args.size()) + ".");
            }
            
            auto now = std::chrono::high_resolution_clock::now();
            auto duration = now.time_since_epoch();
            auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
            
            return std::make_shared<Number>(microseconds);
        });
    env->define("time", timeFunc);
} 