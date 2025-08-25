#include "BobStdLib.h"
#include "Interpreter.h"
#include "ErrorReporter.h"
#include "Lexer.h"
#include "Parser.h"
#include <chrono>
#include <thread>
#include <ctime>
#include <fstream>
#include <sstream>

// Platform-specific includes for memory usage
#if defined(__APPLE__) && defined(__MACH__)
    #include <mach/mach.h>
#elif defined(__linux__)
    // Uses /proc/self/status, no extra includes needed
#elif defined(_WIN32)
    #include <windows.h>
    #include <psapi.h>
#endif

void BobStdLib::addToEnvironment(std::shared_ptr<Environment> env, Interpreter& interpreter, ErrorReporter* errorReporter) {
    // Create a built-in toString function
    auto toStringFunc = std::make_shared<BuiltinFunction>("toString",
        [&interpreter, errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 1 argument but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            
            return Value(interpreter.stringify(args[0]));
        });
    env->define("toString", Value(toStringFunc));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(toStringFunc);

    // Create a built-in print function
    auto printFunc = std::make_shared<BuiltinFunction>("print", 
        [&interpreter, errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 1 argument but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            // Use the interpreter's stringify function
            std::cout << interpreter.stringify(args[0]) << '\n';
            return NONE_VALUE;
        });
    env->define("print", Value(printFunc));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(printFunc);

    // Create a built-in printRaw function (no newline, for ANSI escape sequences)
    auto printRawFunc = std::make_shared<BuiltinFunction>("printRaw", 
        [&interpreter, errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 1 argument but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            // Print without newline and flush immediately for ANSI escape sequences
            std::cout << interpreter.stringify(args[0]) << std::flush;
            return NONE_VALUE;
        });
    env->define("printRaw", Value(printRawFunc));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(printRawFunc);

    

    // Create a built-in assert function
    auto assertFunc = std::make_shared<BuiltinFunction>("assert",
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1 && args.size() != 2) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 1 or 2 arguments but got " + std::to_string(args.size()) + ".", "", true);
                }
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
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", message, "", true);
                }
                throw std::runtime_error(message);
            }
            
            return NONE_VALUE;
        });
    env->define("assert", Value(assertFunc));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(assertFunc);

    // time-related globals moved into builtin time module

    // Create a built-in input function
    auto inputFunc = std::make_shared<BuiltinFunction>("input",
        [&interpreter, errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() > 1) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 0 or 1 arguments but got " + std::to_string(args.size()) + ".", "", true);
                }
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
    env->define("input", Value(inputFunc));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(inputFunc);

    // Create a built-in type function
    auto typeFunc = std::make_shared<BuiltinFunction>("type",
        [&interpreter, errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 1 argument but got " + std::to_string(args.size()) + ".", "", true);
                }
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
                // Check if this is a dispatcher for a user-defined function
                auto bf = args[0].asBuiltinFunction();
                if (bf && !bf->name.empty()) {
                    // Check if there are any user-defined functions with this name
                    // This indicates it's a dispatcher, not a true builtin
                    bool hasUserFunction = false;
                    for (size_t arity = 0; arity < 256; ++arity) { // Reasonable limit
                        if (interpreter.lookupFunction(bf->name, arity)) {
                            hasUserFunction = true;
                            break;
                        }
                    }
                    if (hasUserFunction) {
                        typeName = "function";
                    } else {
                        typeName = "builtin_function";
                    }
                } else {
                    typeName = "builtin_function";
                }
            } else if (args[0].isArray()) {
                typeName = "array";
            } else if (args[0].isDict()) {
                typeName = "dict";
            } else if (args[0].isModule()) {
                typeName = "module";
            } else {
                typeName = "unknown";
            }
            
            return Value(typeName);
        });
    env->define("type", Value(typeFunc));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(typeFunc);

    // Create a built-in toNumber function for string-to-number conversion
    auto toNumberFunc = std::make_shared<BuiltinFunction>("toNumber",
        [](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1) {
                return NONE_VALUE;  // Return none for wrong argument count
            }
            
            if (!args[0].isString()) {
                return NONE_VALUE;  // Return none for wrong type
            }
            
            std::string str = args[0].asString();
            
            // Remove leading/trailing whitespace
            str.erase(0, str.find_first_not_of(" \t\n\r"));
            str.erase(str.find_last_not_of(" \t\n\r") + 1);
            
            if (str.empty()) {
                return NONE_VALUE;  // Return none for empty string
            }
            
            try {
                double value = std::stod(str);
                return Value(value);
            } catch (const std::invalid_argument&) {
                return NONE_VALUE;  // Return none for invalid conversion
            } catch (const std::out_of_range&) {
                return NONE_VALUE;  // Return none for out of range
            }
        });
    env->define("toNumber", Value(toNumberFunc));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(toNumberFunc);

    // Create a built-in toInt function for float-to-integer conversion
    auto toIntFunc = std::make_shared<BuiltinFunction>("toInt",
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 1 argument but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            
            if (!args[0].isNumber()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "toInt() can only be used on numbers", "", true);
                }
                throw std::runtime_error("toInt() can only be used on numbers");
            }
            
            // Convert to integer by truncating (same as | 0)
            double value = args[0].asNumber();
            return Value(static_cast<double>(static_cast<long long>(value)));
        });
    env->define("toInt", Value(toIntFunc));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(toIntFunc);

    // Create a built-in toBoolean function for explicit boolean conversion
    auto toBooleanFunc = std::make_shared<BuiltinFunction>("toBoolean",
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 1 argument but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            
            // Use the same logic as isTruthy() for consistency
            Value value = args[0];
            
            if (value.isNone()) {
                return Value(false);
            }
            
            if (value.isBoolean()) {
                return value;  // Already a boolean
            }
            
            if (value.isNumber()) {
                return Value(value.asNumber() != 0.0);
            }
            
            if (value.isString()) {
                return Value(!value.asString().empty());
            }
            
            // For any other type (functions, etc.), consider them truthy
            return Value(true);
        });
    env->define("toBoolean", Value(toBooleanFunc));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(toBooleanFunc);

    // exit moved to sys module

    // sleep moved into builtin time module

    // Introspection: dir(obj) and functions(obj)
    auto dirFunc = std::make_shared<BuiltinFunction>("dir",
        [](std::vector<Value> args, int, int) -> Value {
            if (args.size() != 1) return Value(std::vector<Value>{});
            Value obj = args[0];
            std::vector<Value> out;
            if (obj.isModule()) {
                auto* mod = obj.asModule();
                if (mod && mod->exports) {
                    for (const auto& kv : *mod->exports) out.push_back(Value(kv.first));
                }
            } else if (obj.isDict()) {
                const auto& d = obj.asDict();
                for (const auto& kv : d) out.push_back(Value(kv.first));
            }
            return Value(out);
        });
    env->define("dir", Value(dirFunc));
    interpreter.addBuiltinFunction(dirFunc);

    auto functionsFunc = std::make_shared<BuiltinFunction>("functions",
        [](std::vector<Value> args, int, int) -> Value {
            if (args.size() != 1) return Value(std::vector<Value>{});
            Value obj = args[0];
            std::vector<Value> out;
            auto pushIfFn = [&out](const std::pair<const std::string, Value>& kv){
                if (kv.second.isFunction() || kv.second.isBuiltinFunction()) out.push_back(Value(kv.first));
            };
            if (obj.isModule()) {
                auto* mod = obj.asModule();
                if (mod && mod->exports) {
                    for (const auto& kv : *mod->exports) pushIfFn(kv);
                }
            } else if (obj.isDict()) {
                const auto& d = obj.asDict();
                for (const auto& kv : d) pushIfFn(kv);
            }
            return Value(out);
        });
    env->define("functions", Value(functionsFunc));
    interpreter.addBuiltinFunction(functionsFunc);

    // random moved to rand module

    // (eval and evalFile moved to eval module)

    

    // (file I/O moved to io module)

    // memoryUsage moved to sys module

} 