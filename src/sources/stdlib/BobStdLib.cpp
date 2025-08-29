#include "BobStdLib.h"
#include "Interpreter.h"
#include "ErrorReporter.h"
#include "Lexer.h"
#include "Parser.h"
#include "GMPWrapper.h"
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
    interpreter.getFunctionRegistry().addBuiltinFunction(toStringFunc);

    // Create a built-in print function
    auto printFunc = std::make_shared<BuiltinFunction>("print", 
        [&interpreter, errorReporter](std::vector<Value> args, int line, int column) -> Value {
            // Handle multiple arguments by joining them with spaces
            for (size_t i = 0; i < args.size(); ++i) {
                if (i > 0) std::cout << " ";
                std::cout << interpreter.stringify(args[i]);
            }
            std::cout << '\n';
            return NONE_VALUE;
        });
    env->define("print", Value(printFunc));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.getFunctionRegistry().addBuiltinFunction(printFunc);

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
    interpreter.getFunctionRegistry().addBuiltinFunction(printRawFunc);

    

    // Create a built-in assert function
    auto assertFunc = std::make_shared<BuiltinFunction>("assert",
        [&interpreter, errorReporter](std::vector<Value> args, int line, int column) -> Value {
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
                // Only report the error if we're not in a try block
                if (!interpreter.isInTry()) {
                    interpreter.reportError(line, column, "StdLib Error", message, "");
                }
                throw std::runtime_error(message);
            }
            
            return NONE_VALUE;
        });
    env->define("assert", Value(assertFunc));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.getFunctionRegistry().addBuiltinFunction(assertFunc);

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
    interpreter.getFunctionRegistry().addBuiltinFunction(inputFunc);

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
            if (args[0].isNumeric()) {
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
                        if (interpreter.getFunctionRegistry().lookupFunction(bf->name, arity)) {
                            hasUserFunction = true;
                            break;
                        }
                    }
                    if (hasUserFunction) {
                        typeName = "function";
                    } else {
                        // Check if this is a class constructor (builtin function with class name)
                        // Class constructors are typically named like "ClassName" or "module::ClassName"
                        if (bf->name.find("::") != std::string::npos || 
                            (bf->name[0] >= 'A' && bf->name[0] <= 'Z')) {
                            typeName = "class: " + bf->name;
                        } else {
                            typeName = "builtin_function";
                        }
                    }
                } else {
                    typeName = "builtin_function";
                }
            } else if (args[0].isArray()) {
                typeName = "array";
            } else if (args[0].isDict()) {
                // Check if this is a class instance
                const auto& dict = args[0].asDict();
                auto classIt = dict.find("__class");
                if (classIt != dict.end() && classIt->second.isString()) {
                    typeName = "object: " + classIt->second.asString();
                } else {
                    typeName = "dict";
                }
            } else if (args[0].isModule()) {
                typeName = "module";
            } else {
                typeName = "unknown";
            }
            
            return Value(typeName);
        });
    env->define("type", Value(typeFunc));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.getFunctionRegistry().addBuiltinFunction(typeFunc);

    // Create a built-in typeRaw function (debug version that shows actual internal types)
    auto typeRawFunc = std::make_shared<BuiltinFunction>("typeRaw",
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
            } else if (args[0].isInteger()) {
                typeName = "integer";
            } else if (args[0].isBigInt()) {
                typeName = "bigint";
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
            } else if (args[0].isModule()) {
                typeName = "module";
            } else {
                typeName = "unknown";
            }
            
            return Value(typeName);
        });
    env->define("typeRaw", Value(typeRawFunc));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.getFunctionRegistry().addBuiltinFunction(typeRawFunc);

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
            
            // Remove any invisible characters that might cause conversion to fail
            str.erase(std::remove_if(str.begin(), str.end(), [](char c) {
                return c < 32 && c != '\t' && c != '\n' && c != '\r';
            }), str.end());
            
            // Also remove any non-printable characters
            str.erase(std::remove_if(str.begin(), str.end(), [](char c) {
                return !std::isprint(c) && c != '\t' && c != '\n' && c != '\r';
            }), str.end());
            
            // Handle empty string
            if (str.empty()) {
                return NONE_VALUE;  // Return none for empty string
            }
            
            // Trim whitespace
            str.erase(0, str.find_first_not_of(" \t\n\r"));
            str.erase(str.find_last_not_of(" \t\n\r") + 1);
            
            // Handle special cases first
            if (str == "0" || str == "0.0" || str == "-0" || str == "-0.0") {
                return Value(0.0);
            }
            
            // Check if it's a valid number format (integer or decimal)
            bool isValidNumber = true;
            bool hasDecimal = false;
            bool hasExponent = false;
            bool hasSign = false;
            
            for (size_t i = 0; i < str.length(); i++) {
                char c = str[i];
                if (c == '+' || c == '-') {
                    if (i == 0) {
                        hasSign = true;
                    } else if (str[i-1] == 'e' || str[i-1] == 'E') {
                        // Exponent sign is valid
                    } else {
                        isValidNumber = false;
                        break;
                    }
                } else if (c == '.') {
                    if (hasDecimal || hasExponent) {
                        isValidNumber = false;
                        break;
                    }
                    hasDecimal = true;
                } else if (c == 'e' || c == 'E') {
                    if (hasExponent) {
                        isValidNumber = false;
                        break;
                    }
                    hasExponent = true;
                } else if (c < '0' || c > '9') {
                    isValidNumber = false;
                    break;
                }
            }
            
            if (!isValidNumber) {
                return NONE_VALUE;  // Return none for invalid format
            }
            
            // Try to convert as a regular number first
            try {
                double value = std::stod(str);
                
                // Check if this is an integer that's too large for double precision
                if (!hasDecimal && !hasExponent) {
                    // For integers, check if they're too large
                    const double MAX_SAFE_INTEGER = 9007199254740991.0; // 2^53 - 1
                    if (value > MAX_SAFE_INTEGER || value < -MAX_SAFE_INTEGER) {
                        try {
                            GMPWrapper::BigInt bigint = GMPWrapper::BigInt::fromString(str);
                            return Value(bigint);
                        } catch (...) {
                            // If bigint creation fails, fall back to double
                        }
                    }
                }
                
                return Value(value);
            } catch (const std::invalid_argument&) {
                // If std::stod fails, try as bigint for integers
                if (!hasDecimal && !hasExponent) {
                    try {
                        GMPWrapper::BigInt bigint = GMPWrapper::BigInt::fromString(str);
                        return Value(bigint);
                    } catch (...) {
                        // If bigint creation fails, return none
                    }
                }
                return NONE_VALUE;  // Return none for invalid conversion
            } catch (const std::out_of_range&) {
                // If std::stod fails due to range, try as bigint for integers
                if (!hasDecimal && !hasExponent) {
                    try {
                        GMPWrapper::BigInt bigint = GMPWrapper::BigInt::fromString(str);
                        return Value(bigint);
                    } catch (...) {
                        // If bigint creation fails, return none
                    }
                }
                return NONE_VALUE;  // Return none for out of range
            }
        });
    env->define("toNumber", Value(toNumberFunc));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.getFunctionRegistry().addBuiltinFunction(toNumberFunc);

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
            
            if (!args[0].isNumeric()) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "toInt() can only be used on numbers", "", true);
                }
                throw std::runtime_error("toInt() can only be used on numbers");
            }
            
            // Convert to integer by truncating (same as | 0)
            if (args[0].isInteger()) {
                return args[0]; // Already an integer
            } else if (args[0].isBigInt()) {
                // For BigInt, check if it fits in long long range
                const GMPWrapper::BigInt& bigint = args[0].asBigInt();
                if (bigint.fitsInLongLong()) {
                    return Value(bigint.toLongLong());
                } else {
                    // If it doesn't fit, throw an error instead of truncating
                    throw std::runtime_error("BigInt value too large to convert to integer");
                }
            } else {
                // For numbers (doubles), truncate to long long
                double value = args[0].asNumber();
                return Value(static_cast<long long>(value));
            }
        });
    env->define("toInt", Value(toIntFunc));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.getFunctionRegistry().addBuiltinFunction(toIntFunc);

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
    interpreter.getFunctionRegistry().addBuiltinFunction(toBooleanFunc);

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
    interpreter.getFunctionRegistry().addBuiltinFunction(dirFunc);

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
    interpreter.getFunctionRegistry().addBuiltinFunction(functionsFunc);

    auto valuesFunc = std::make_shared<BuiltinFunction>("values",
        [](std::vector<Value> args, int, int) -> Value {
            if (args.size() != 1) return Value(std::vector<Value>{});
            Value obj = args[0];
            std::vector<Value> out;
            auto pushIfValue = [&out](const std::pair<const std::string, Value>& kv){
                if (!kv.second.isFunction() && !kv.second.isBuiltinFunction()) out.push_back(Value(kv.first));
            };
            if (obj.isModule()) {
                auto* mod = obj.asModule();
                if (mod && mod->exports) {
                    for (const auto& kv : *mod->exports) pushIfValue(kv);
                }
            } else if (obj.isDict()) {
                const auto& d = obj.asDict();
                for (const auto& kv : d) pushIfValue(kv);
            }
            return Value(out);
        });
    env->define("values", Value(valuesFunc));
    interpreter.getFunctionRegistry().addBuiltinFunction(valuesFunc);

    // random moved to rand module

    // (eval and evalFile moved to eval module)

    

    // (file I/O moved to io module)

    // memoryUsage moved to sys module

    // Array utility functions
    auto rangeFunc = std::make_shared<BuiltinFunction>("range",
        [](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() < 1 || args.size() > 3) {
                throw std::runtime_error("range() expects 1-3 arguments: range(end) or range(start, end) or range(start, end, step)");
            }
            
            long long start = 0, end, step = 1;
            
            if (args.size() == 1) {
                // range(end)
                if (!args[0].isNumeric()) {
                    throw std::runtime_error("range() argument must be numeric");
                }
                end = args[0].isInteger() ? args[0].asInteger() : static_cast<long long>(args[0].asNumber());
            } else if (args.size() == 2) {
                // range(start, end)
                if (!args[0].isNumeric() || !args[1].isNumeric()) {
                    throw std::runtime_error("range() arguments must be numeric");
                }
                start = args[0].isInteger() ? args[0].asInteger() : static_cast<long long>(args[0].asNumber());
                end = args[1].isInteger() ? args[1].asInteger() : static_cast<long long>(args[1].asNumber());
            } else {
                // range(start, end, step)
                if (!args[0].isNumeric() || !args[1].isNumeric() || !args[2].isNumeric()) {
                    throw std::runtime_error("range() arguments must be numeric");
                }
                start = args[0].isInteger() ? args[0].asInteger() : static_cast<long long>(args[0].asNumber());
                end = args[1].isInteger() ? args[1].asInteger() : static_cast<long long>(args[1].asNumber());
                step = args[2].isInteger() ? args[2].asInteger() : static_cast<long long>(args[2].asNumber());
            }
            
            if (step == 0) {
                throw std::runtime_error("range() step cannot be zero");
            }
            
            std::vector<Value> result;
            if (step > 0) {
                for (long long i = start; i < end; i += step) {
                    result.push_back(Value(i));
                }
            } else {
                for (long long i = start; i > end; i += step) {
                    result.push_back(Value(i));
                }
            }
            
            return Value(result);
        });
    env->define("range", Value(rangeFunc));
    interpreter.getFunctionRegistry().addBuiltinFunction(rangeFunc);



} 