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
    env->define("toString", Value(toStringFunc.get()));
    
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
    env->define("print", Value(printFunc.get()));
    
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
    env->define("printRaw", Value(printRawFunc.get()));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(printRawFunc);

    // Create a built-in len function for arrays and strings
    auto lenFunc = std::make_shared<BuiltinFunction>("len", 
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 1 argument but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            
            if (args[0].isArray()) {
                return Value(static_cast<double>(args[0].asArray().size()));
            } else if (args[0].isString()) {
                return Value(static_cast<double>(args[0].asString().length()));
            } else if (args[0].isDict()) {
                return Value(static_cast<double>(args[0].asDict().size()));
            } else {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "len() can only be used on arrays, strings, and dictionaries", "", true);
                }
                throw std::runtime_error("len() can only be used on arrays, strings, and dictionaries");
            }
        });
    env->define("len", Value(lenFunc.get()));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(lenFunc);

    // Create a built-in push function for arrays
    auto pushFunc = std::make_shared<BuiltinFunction>("push", 
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() < 2) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected at least 2 arguments but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected at least 2 arguments but got " + std::to_string(args.size()) + ".");
            }
            
            if (!args[0].isArray()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "First argument to push() must be an array", "", true);
                }
                throw std::runtime_error("First argument to push() must be an array");
            }
            
            // Get the array and modify it in place
            std::vector<Value>& arr = args[0].asArray();
            
            // Add all arguments except the first one (which is the array)
            for (size_t i = 1; i < args.size(); i++) {
                arr.push_back(args[i]);
            }
            
            return args[0]; // Return the modified array
        });
    env->define("push", Value(pushFunc.get()));
    interpreter.addBuiltinFunction(pushFunc);

    // Create a built-in pop function for arrays
    auto popFunc = std::make_shared<BuiltinFunction>("pop", 
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 1 argument but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            
            if (!args[0].isArray()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "pop() can only be used on arrays", "", true);
                }
                throw std::runtime_error("pop() can only be used on arrays");
            }
            
            std::vector<Value>& arr = args[0].asArray();
            if (arr.empty()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Cannot pop from empty array", "", true);
                }
                throw std::runtime_error("Cannot pop from empty array");
            }
            
            // Get the last element and remove it from the array
            Value lastElement = arr.back();
            arr.pop_back();
            
            return lastElement; // Return the popped element
        });
    env->define("pop", Value(popFunc.get()));
    interpreter.addBuiltinFunction(popFunc);

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
    env->define("assert", Value(assertFunc.get()));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(assertFunc);

    // Create a built-in time function (returns microseconds since Unix epoch)
    auto timeFunc = std::make_shared<BuiltinFunction>("time",
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 0) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 0 arguments but got " + std::to_string(args.size()) + ".", "", true);
                }
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
    env->define("input", Value(inputFunc.get()));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(inputFunc);

    // Create a built-in type function
    auto typeFunc = std::make_shared<BuiltinFunction>("type",
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
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
                typeName = "builtin_function";
            } else if (args[0].isArray()) {
                typeName = "array";
            } else if (args[0].isDict()) {
                typeName = "dict";
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
    env->define("toNumber", Value(toNumberFunc.get()));
    
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
    env->define("toInt", Value(toIntFunc.get()));
    
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
    env->define("toBoolean", Value(toBooleanFunc.get()));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(toBooleanFunc);

    // Create a built-in exit function to terminate the program
    auto exitFunc = std::make_shared<BuiltinFunction>("exit",
        [](std::vector<Value> args, int line, int column) -> Value {
            int exitCode = 0;  // Default exit code
            
            if (args.size() > 0) {
                if (args[0].isNumber()) {
                    exitCode = static_cast<int>(args[0].asNumber());
                }
                // If not a number, just use default exit code 0
            }
            
            std::exit(exitCode);
        });
    env->define("exit", Value(exitFunc.get()));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(exitFunc);

    // Create a built-in sleep function for animations and timing
    auto sleepFunc = std::make_shared<BuiltinFunction>("sleep",
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
                        "sleep() argument must be a number", "", true);
                }
                throw std::runtime_error("sleep() argument must be a number");
            }
            
            double seconds = args[0].asNumber();
            if (seconds < 0) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "sleep() argument cannot be negative", "", true);
                }
                throw std::runtime_error("sleep() argument cannot be negative");
            }
            
            // Convert to milliseconds and sleep
            int milliseconds = static_cast<int>(seconds * 1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
            
            return NONE_VALUE;
        });
    env->define("sleep", Value(sleepFunc.get()));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(sleepFunc);

    // Create a built-in random function
    auto randomFunc = std::make_shared<BuiltinFunction>("random",
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 0) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 0 arguments but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 0 arguments but got " + std::to_string(args.size()) + ".");
            }
            
            // Seed the random number generator if not already done
            static bool seeded = false;
            if (!seeded) {
                srand(static_cast<unsigned int>(time(nullptr)));
                seeded = true;
            }
            
            return Value(static_cast<double>(rand()) / RAND_MAX);
        });
    env->define("random", Value(randomFunc.get()));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(randomFunc);

    // Create a built-in eval function (like Python's eval)
    auto evalFunc = std::make_shared<BuiltinFunction>("eval",
        [&interpreter, errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "Invalid Arguments", 
                        "eval expects exactly 1 argument (string)", "eval");
                }
                throw std::runtime_error("eval expects exactly 1 argument");
            }
            
            if (!args[0].isString()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "Invalid Type", 
                        "eval argument must be a string", "eval");
                }
                throw std::runtime_error("eval argument must be a string");
            }
            
            std::string code = args[0].asString();
            
            try {
                // Create a new lexer for the code string
                Lexer lexer;
                lexer.setErrorReporter(errorReporter);
                std::vector<Token> tokens = lexer.Tokenize(code);
                
                // Create a new parser
                Parser parser(tokens);
                parser.setErrorReporter(errorReporter);
                std::vector<std::shared_ptr<Stmt>> statements = parser.parse();
                
                // Execute the statements in the current environment
                // Note: This runs in the current scope, so variables are shared
                interpreter.interpret(statements);
                
                // For now, return NONE_VALUE since we don't have a way to get the last expression value
                // In a more sophisticated implementation, we'd track the last expression result
                return NONE_VALUE;
                
            } catch (const std::exception& e) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "Eval Error", 
                        "Failed to evaluate code: " + std::string(e.what()), code);
                }
                throw std::runtime_error("eval failed: " + std::string(e.what()));
            }
        });
    env->define("eval", Value(evalFunc.get()));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(evalFunc);

    // Create a built-in keys function for dictionaries
    auto keysFunc = std::make_shared<BuiltinFunction>("keys",
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 1 argument but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            
            if (!args[0].isDict()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "keys() can only be used on dictionaries", "", true);
                }
                throw std::runtime_error("keys() can only be used on dictionaries");
            }
            
            const std::unordered_map<std::string, Value>& dict = args[0].asDict();
            std::vector<Value> keys;
            
            for (const auto& pair : dict) {
                keys.push_back(Value(pair.first));
            }
            
            return Value(keys);
        });
    env->define("keys", Value(keysFunc.get()));
    interpreter.addBuiltinFunction(keysFunc);

    // Create a built-in values function for dictionaries
    auto valuesFunc = std::make_shared<BuiltinFunction>("values",
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 1 argument but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            
            if (!args[0].isDict()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "values() can only be used on dictionaries", "", true);
                }
                throw std::runtime_error("values() can only be used on dictionaries");
            }
            
            const std::unordered_map<std::string, Value>& dict = args[0].asDict();
            std::vector<Value> values;
            
            for (const auto& pair : dict) {
                values.push_back(pair.second);
            }
            
            return Value(values);
        });
    env->define("values", Value(valuesFunc.get()));
    interpreter.addBuiltinFunction(valuesFunc);

    // Create a built-in has function for dictionaries
    auto hasFunc = std::make_shared<BuiltinFunction>("has",
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 2) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 2 arguments but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 2 arguments but got " + std::to_string(args.size()) + ".");
            }
            
            if (!args[0].isDict()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "First argument to has() must be a dictionary", "", true);
                }
                throw std::runtime_error("First argument to has() must be a dictionary");
            }
            
            if (!args[1].isString()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Second argument to has() must be a string", "", true);
                }
                throw std::runtime_error("Second argument to has() must be a string");
            }
            
            const std::unordered_map<std::string, Value>& dict = args[0].asDict();
            std::string key = args[1].asString();
            
            return Value(dict.find(key) != dict.end());
        });
    env->define("has", Value(hasFunc.get()));
    interpreter.addBuiltinFunction(hasFunc);

    // Create a built-in readFile function
    auto readFileFunc = std::make_shared<BuiltinFunction>("readFile",
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 1 argument but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            
            if (!args[0].isString()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "readFile() argument must be a string", "", true);
                }
                throw std::runtime_error("readFile() argument must be a string");
            }
            
            std::string filename = args[0].asString();
            std::ifstream file(filename);
            
            if (!file.is_open()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Could not open file: " + filename, "", true);
                }
                throw std::runtime_error("Could not open file: " + filename);
            }
            
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();
            
            return Value(buffer.str());
        });
    env->define("readFile", Value(readFileFunc.get()));
    interpreter.addBuiltinFunction(readFileFunc);

    // Create a built-in writeFile function
    auto writeFileFunc = std::make_shared<BuiltinFunction>("writeFile",
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 2) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 2 arguments but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 2 arguments but got " + std::to_string(args.size()) + ".");
            }
            
            if (!args[0].isString()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "First argument to writeFile() must be a string", "", true);
                }
                throw std::runtime_error("First argument to writeFile() must be a string");
            }
            
            if (!args[1].isString()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Second argument to writeFile() must be a string", "", true);
                }
                throw std::runtime_error("Second argument to writeFile() must be a string");
            }
            
            std::string filename = args[0].asString();
            std::string content = args[1].asString();
            
            std::ofstream file(filename);
            if (!file.is_open()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Could not create file: " + filename, "", true);
                }
                throw std::runtime_error("Could not create file: " + filename);
            }
            
            file << content;
            file.close();
            
            return NONE_VALUE;
        });
    env->define("writeFile", Value(writeFileFunc.get()));
    interpreter.addBuiltinFunction(writeFileFunc);

    // Create a built-in readLines function
    auto readLinesFunc = std::make_shared<BuiltinFunction>("readLines",
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 1 argument but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            
            if (!args[0].isString()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "readLines() argument must be a string", "", true);
                }
                throw std::runtime_error("readLines() argument must be a string");
            }
            
            std::string filename = args[0].asString();
            std::ifstream file(filename);
            
            if (!file.is_open()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Could not open file: " + filename, "", true);
                }
                throw std::runtime_error("Could not open file: " + filename);
            }
            
            std::vector<Value> lines;
            std::string line_content;
            
            while (std::getline(file, line_content)) {
                lines.push_back(Value(line_content));
            }
            
            file.close();
            return Value(lines);
        });
    env->define("readLines", Value(readLinesFunc.get()));
    interpreter.addBuiltinFunction(readLinesFunc);

    // Create a built-in fileExists function
    auto fileExistsFunc = std::make_shared<BuiltinFunction>("fileExists",
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 1 argument but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 1 argument but got " + std::to_string(args.size()) + ".");
            }
            
            if (!args[0].isString()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "fileExists() argument must be a string", "", true);
                }
                throw std::runtime_error("fileExists() argument must be a string");
            }
            
            std::string filename = args[0].asString();
            std::ifstream file(filename);
            bool exists = file.good();
            file.close();
            
            return Value(exists);
        });
    env->define("fileExists", Value(fileExistsFunc.get()));
    interpreter.addBuiltinFunction(fileExistsFunc);

} 