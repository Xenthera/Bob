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

    // Create a built-in time function (returns strictly increasing microseconds)
    auto timeFunc = std::make_shared<BuiltinFunction>("time",
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 0) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 0 arguments but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 0 arguments but got " + std::to_string(args.size()) + ".");
            }
            
            static long long lastReturnedMicros = 0;
            auto now = std::chrono::high_resolution_clock::now();
            auto duration = now.time_since_epoch();
            long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
            if (microseconds <= lastReturnedMicros) {
                microseconds = lastReturnedMicros + 1;
            }
            lastReturnedMicros = microseconds;
            return Value(static_cast<double>(microseconds));
        });
    env->define("time", Value(timeFunc));
    
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
    env->define("input", Value(inputFunc));
    
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
    env->define("exit", Value(exitFunc));
    
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
    env->define("sleep", Value(sleepFunc));
    
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
    env->define("random", Value(randomFunc));
    
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
            std::string evalName = "<eval>";
            
            try {
                // Push eval source for correct error context
                if (errorReporter) {
                    errorReporter->pushSource(code, evalName);
                }
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
            } catch (...) {
                if (errorReporter) {
                    errorReporter->popSource();
                }
                throw;
            }
            if (errorReporter) {
                errorReporter->popSource();
            }
        });
    env->define("eval", Value(evalFunc));
    
    // Store the shared_ptr in the interpreter to keep it alive
    interpreter.addBuiltinFunction(evalFunc);

    

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
    env->define("readFile", Value(readFileFunc));
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
    env->define("writeFile", Value(writeFileFunc));
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
    env->define("readLines", Value(readLinesFunc));
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
    env->define("fileExists", Value(fileExistsFunc));
    interpreter.addBuiltinFunction(fileExistsFunc);

    // Create a built-in memoryUsage function (platform-specific, best effort)
    auto memoryUsageFunc = std::make_shared<BuiltinFunction>("memoryUsage",
        [errorReporter](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 0) {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "StdLib Error", 
                        "Expected 0 arguments but got " + std::to_string(args.size()) + ".", "", true);
                }
                throw std::runtime_error("Expected 0 arguments but got " + std::to_string(args.size()) + ".");
            }
            
            // Platform-specific memory usage detection
            size_t memoryBytes = 0;
            
#if defined(__APPLE__) && defined(__MACH__)
            // macOS
            struct mach_task_basic_info info;
            mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
            if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &infoCount) == KERN_SUCCESS) {
                memoryBytes = info.resident_size;
            }
#elif defined(__linux__)
            // Linux - read from /proc/self/status
            std::ifstream statusFile("/proc/self/status");
            std::string line;
            while (std::getline(statusFile, line)) {
                if (line.substr(0, 6) == "VmRSS:") {
                    std::istringstream iss(line);
                    std::string label, value, unit;
                    iss >> label >> value >> unit;
                    memoryBytes = std::stoull(value) * 1024; // Convert KB to bytes
                    break;
                }
            }
#elif defined(_WIN32)
            // Windows
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
                memoryBytes = pmc.WorkingSetSize;
            }
#endif
            
            // Return memory usage in MB for readability
            double memoryMB = static_cast<double>(memoryBytes) / (1024.0 * 1024.0);
            return Value(memoryMB);
        });
    env->define("memoryUsage", Value(memoryUsageFunc));
    interpreter.addBuiltinFunction(memoryUsageFunc);

} 