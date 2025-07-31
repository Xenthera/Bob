#include "../headers/StdLib.h"
#include "../headers/Interpreter.h"
#include <chrono>

void StdLib::addToEnvironment(std::shared_ptr<Environment> env, Interpreter& interpreter) {
    // Create a built-in toString function
    auto toStringFunc = std::make_shared<BuiltinFunction>("toString",
        [&interpreter](std::vector<Value> args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            
            return Value(interpreter.stringify(args[0]));
        });
    env->define("toString", Value(toStringFunc.get()));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(toStringFunc);

    // Create a built-in print function
    auto printFunc = std::make_shared<BuiltinFunction>("print", 
        [&interpreter](std::vector<Value> args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            // Use the interpreter's stringify function
            std::cout << interpreter.stringify(args[0]) << std::endl;
            return NONE_VALUE;
        });
    env->define("print", Value(printFunc.get()));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(printFunc);

    // Create a built-in assert function
    auto assertFunc = std::make_shared<BuiltinFunction>("assert",
        [](std::vector<Value> args) -> Value {
            if (args.size() != 1 && args.size() != 2) {
                throw std::runtime_error("Expected 1 or 2 arguments but got " + std::to_string(args.size()) + ".");
            }
            
            // Simple truthy check without calling interpreter.isTruthy
            bool isTruthy = false;
            if (args[0].isBoolean()) {
                isTruthy = args[0].asBoolean();
            } else if (args[0].isNone()) {
                isTruthy = false;
            } else {
                isTruthy = true; // Numbers, strings, functions are truthy
            }
            
            if (!isTruthy) {
                std::string message = "Assertion failed: condition is false";
                if (args.size() == 2) {
                    if (args[1].isString()) {
                        message += " - " + std::string(args[1].asString());
                    }
                }
                throw std::runtime_error(message);
            }
            
            return NONE_VALUE;
        });
    env->define("assert", Value(assertFunc.get()));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(assertFunc);

    // Create a built-in time function (returns microseconds since Unix epoch)
    auto timeFunc = std::make_shared<BuiltinFunction>("time",
        [](std::vector<Value> args) -> Value {
            if (args.size() != 0) {
                throw std::runtime_error("Expected 0 arguments but got " + std::to_string(args.size()) + ".");
            }
            
            auto now = std::chrono::high_resolution_clock::now();
            auto duration = now.time_since_epoch();
            auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
            
            return Value(static_cast<double>(microseconds));
        });
    env->define("time", Value(timeFunc.get()));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(timeFunc);

    // Create a built-in input function
    auto inputFunc = std::make_shared<BuiltinFunction>("input",
        [&interpreter](std::vector<Value> args) -> Value {
            if (args.size() > 1) {
                throw std::runtime_error("Expected 0 or 1 arguments but got " + std::to_string(args.size()) + ".");
            }
            
            // Optional prompt
            if (args.size() == 1) {
                std::cout << interpreter.stringify(args[0]);
            }
            
            // Get user input
            std::string userInput;
            std::getline(std::cin, userInput);
            
            return Value(userInput);
        });
    env->define("input", Value(inputFunc.get()));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(inputFunc);

    // Create a built-in type function
    auto typeFunc = std::make_shared<BuiltinFunction>("type",
        [](std::vector<Value> args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            
            std::string typeName;
            if (args[0].isNumber()) {
                typeName = "number";
            } else if (args[0].isString()) {
                typeName = "string";
            } else if (args[0].isBoolean()) {
                typeName = "boolean";
            } else if (args[0].isNone()) {
                typeName = "none";
            } else if (args[0].isFunction()) {
                typeName = "function";
            } else if (args[0].isBuiltinFunction()) {
                typeName = "builtin_function";
            } else {
                typeName = "unknown";
            }
            
            return Value(typeName);
        });
    env->define("type", Value(typeFunc.get()));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(typeFunc);

    // Create a built-in toNumber function for string-to-number conversion
    auto toNumberFunc = std::make_shared<BuiltinFunction>("toNumber",
        [](std::vector<Value> args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            
            if (!args[0].isString()) {
                throw std::runtime_error("toNumber() expects a string argument.");
            }
            
            std::string str = args[0].asString();
            
            // Remove leading/trailing whitespace
            str.erase(0, str.find_first_not_of(" \t\n\r"));
            str.erase(str.find_last_not_of(" \t\n\r") + 1);
            
            if (str.empty()) {
                throw std::runtime_error("Cannot convert empty string to number.");
            }
            
            try {
                double value = std::stod(str);
                return Value(value);
            } catch (const std::invalid_argument&) {
                throw std::runtime_error("Cannot convert '" + str + "' to number.");
            } catch (const std::out_of_range&) {
                throw std::runtime_error("Number '" + str + "' is out of range.");
            }
        });
    env->define("toNumber", Value(toNumberFunc.get()));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(toNumberFunc);
} 