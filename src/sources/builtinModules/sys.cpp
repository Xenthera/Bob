#include "sys.h"
#include "Interpreter.h"
#include "Environment.h"
#include "Lexer.h" // for Token and IDENTIFIER
#include <unistd.h>
#include <limits.h>
#include <cstdlib>
#include <cstring>
#include <vector>

void registerSysModule(Interpreter& interpreter) {
    interpreter.registerModule("sys", [](Interpreter::ModuleBuilder& m) {
        Interpreter& I = m.interpreterRef;
        m.fn("platform", [](std::vector<Value>, int, int) -> Value {
#if defined(_WIN32)
            return Value(std::string("win32"));
#elif defined(__APPLE__)
            return Value(std::string("darwin"));
#elif defined(__linux__)
            return Value(std::string("linux"));
#else
            return Value(std::string("unknown"));
#endif
        });
        m.fn("version", [](std::vector<Value>, int, int) -> Value { return Value(std::string("0.0.3")); });
        // argv(): array of strings
        m.fn("argv", [&I](std::vector<Value>, int, int) -> Value {
            std::vector<Value> out;
            for (const auto& s : I.getArgv()) out.push_back(Value(s));
            return Value(out);
        });
        // executable(): absolute path to the running binary (host-provided)
        m.fn("executable", [&I](std::vector<Value>, int, int) -> Value { return Value(I.getExecutablePath()); });
        // modules(): read-only snapshot of module cache
        m.fn("modules", [&I](std::vector<Value>, int, int) -> Value {
            Value dictVal = Value(std::unordered_map<std::string, Value>{});
            auto snapshot = I.getModuleCacheSnapshot();
            return Value(snapshot);
        });
        m.fn("exit", [](std::vector<Value> a, int, int) -> Value {
            int code = 0; if (!a.empty() && a[0].isNumber()) code = static_cast<int>(a[0].asNumber());
            std::exit(code);
            return NONE_VALUE;
        });
        // env/cwd/pid moved to os; keep sys minimal
    });
}


