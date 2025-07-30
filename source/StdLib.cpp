#include "../headers/StdLib.h"
#include "../headers/Interpreter.h"
#include <chrono>

void StdLib::addToEnvironment(sptr(Environment) env, Interpreter* interpreter) {
    // Create a built-in toString function
    auto toStringFunc = std::make_shared<BuiltinFunction>("toString",
        [interpreter](std::vector<std::shared_ptr<Object>> args) -> std::shared_ptr<Object> {
            if (args.size() != 1) {
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            
            return std::make_shared<String>(interpreter->stringify(args[0]));
        });
    env->define("toString", toStringFunc);

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

    // Create a built-in input function
    auto inputFunc = std::make_shared<BuiltinFunction>("input",
        [interpreter](std::vector<std::shared_ptr<Object>> args) -> std::shared_ptr<Object> {
            if (args.size() > 1) {
                throw std::runtime_error("Expected 0 or 1 arguments but got " + std::to_string(args.size()) + ".");
            }
            
            // Optional prompt
            if (args.size() == 1) {
                std::cout << interpreter->stringify(args[0]);
            }
            
            // Get user input
            std::string userInput;
            std::getline(std::cin, userInput);
            
            return std::make_shared<String>(userInput);
        });
    env->define("input", inputFunc);

    // Create a built-in type function
    auto typeFunc = std::make_shared<BuiltinFunction>("type",
        [](std::vector<std::shared_ptr<Object>> args) -> std::shared_ptr<Object> {
            if (args.size() != 1) {
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            
            std::string typeName;
            if (std::dynamic_pointer_cast<Number>(args[0])) {
                typeName = "number";
            } else if (std::dynamic_pointer_cast<String>(args[0])) {
                typeName = "string";
            } else if (std::dynamic_pointer_cast<Boolean>(args[0])) {
                typeName = "boolean";
            } else if (std::dynamic_pointer_cast<None>(args[0])) {
                typeName = "none";
            } else if (std::dynamic_pointer_cast<Function>(args[0])) {
                typeName = "function";
            } else if (std::dynamic_pointer_cast<BuiltinFunction>(args[0])) {
                typeName = "builtin_function";
            } else {
                typeName = "unknown";
            }
            
            return std::make_shared<String>(typeName);
        });
    env->define("type", typeFunc);

    // Create a built-in toNumber function for string-to-number conversion
    auto toNumberFunc = std::make_shared<BuiltinFunction>("toNumber",
        [](std::vector<std::shared_ptr<Object>> args) -> std::shared_ptr<Object> {
            if (args.size() != 1) {
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            
            auto strObj = std::dynamic_pointer_cast<String>(args[0]);
            if (!strObj) {
                throw std::runtime_error("toNumber() expects a string argument.");
            }
            
            std::string str = strObj->value;
            
            // Remove leading/trailing whitespace
            str.erase(0, str.find_first_not_of(" \t\n\r"));
            str.erase(str.find_last_not_of(" \t\n\r") + 1);
            
            if (str.empty()) {
                throw std::runtime_error("Cannot convert empty string to number.");
            }
            
            try {
                double value = std::stod(str);
                return std::make_shared<Number>(value);
            } catch (const std::invalid_argument&) {
                throw std::runtime_error("Cannot convert '" + str + "' to number.");
            } catch (const std::out_of_range&) {
                throw std::runtime_error("Number '" + str + "' is out of range.");
            }
        });
    env->define("toNumber", toNumberFunc);
} 