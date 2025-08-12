#include "sys.h"
#include "Interpreter.h"
#include "Environment.h"
#include "Lexer.h" // for Token and IDENTIFIER
#include <unistd.h>
#include <limits.h>
#include <cstdlib>
#include <cstring>
#if defined(__APPLE__)
#define DYLD_BOOL DYLD_BOOL_IGNORED
#include <mach-o/dyld.h>
#undef DYLD_BOOL
#endif

void registerSysModule(Interpreter& interpreter) {
    interpreter.registerModule("sys", [](Interpreter::ModuleBuilder& m) {
        Interpreter& I = m.interpreterRef;
        m.fn("memoryUsage", [&I](std::vector<Value>, int l, int c) -> Value {
            try {
                Value fn = I.getEnvironment()->get(Token{IDENTIFIER, "memoryUsage", l, c});
                if (fn.isBuiltinFunction()) return fn.asBuiltinFunction()->func({}, l, c);
            } catch (...) {}
            return NONE_VALUE;
        });
        m.fn("exit", [](std::vector<Value> a, int, int) -> Value {
            int code = 0; if (!a.empty() && a[0].isNumber()) code = static_cast<int>(a[0].asNumber());
            std::exit(code);
            return NONE_VALUE;
        });
        m.fn("cwd", [](std::vector<Value>, int, int) -> Value {
            char buf[PATH_MAX];
            if (getcwd(buf, sizeof(buf))) { return Value(std::string(buf)); }
            return NONE_VALUE;
        });
        m.fn("platform", [](std::vector<Value>, int, int) -> Value {
#if defined(_WIN32)
            return Value(std::string("windows"));
#elif defined(__APPLE__)
            return Value(std::string("macos"));
#elif defined(__linux__)
            return Value(std::string("linux"));
#else
            return Value(std::string("unknown"));
#endif
        });
        m.fn("getenv", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 1 || !a[0].isString()) return NONE_VALUE;
            const char* v = std::getenv(a[0].asString().c_str());
            if (!v) return NONE_VALUE;
            return Value(std::string(v));
        });
        m.fn("pid", [](std::vector<Value>, int, int) -> Value {
            return Value(static_cast<double>(getpid()));
        });
        m.fn("chdir", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 1 || !a[0].isString()) return Value(false);
            const std::string& p = a[0].asString();
            int rc = chdir(p.c_str());
            return Value(rc == 0);
        });
        m.fn("homeDir", [](std::vector<Value>, int, int) -> Value {
            const char* h = std::getenv("HOME");
            if (h) return Value(std::string(h));
            const char* up = std::getenv("USERPROFILE");
            if (up) return Value(std::string(up));
            return NONE_VALUE;
        });
        m.fn("tempDir", [](std::vector<Value>, int, int) -> Value {
            const char* t = std::getenv("TMPDIR");
            if (!t) t = std::getenv("TMP");
            if (!t) t = std::getenv("TEMP");
            if (t) return Value(std::string(t));
            return Value(std::string("/tmp"));
        });
        m.fn("pathSep", [](std::vector<Value>, int, int) -> Value {
#if defined(_WIN32)
            return Value(std::string(";"));
#else
            return Value(std::string(":"));
#endif
        });
        m.fn("dirSep", [](std::vector<Value>, int, int) -> Value {
#if defined(_WIN32)
            return Value(std::string("\\"));
#else
            return Value(std::string("/"));
#endif
        });
        m.fn("execPath", [](std::vector<Value>, int, int) -> Value {
#if defined(__APPLE__)
            uint32_t sz = 0;
            _NSGetExecutablePath(nullptr, &sz);
            std::string buf(sz, '\0');
            if (_NSGetExecutablePath(buf.data(), &sz) == 0) {
                buf.resize(std::strlen(buf.c_str()));
                return Value(buf);
            }
            return NONE_VALUE;
#elif defined(__linux__)
            char path[PATH_MAX];
            ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
            if (len > 0) { path[len] = '\0'; return Value(std::string(path)); }
            return NONE_VALUE;
#else
            return NONE_VALUE;
#endif
        });
        m.fn("env", [](std::vector<Value>, int, int) -> Value {
            std::unordered_map<std::string, Value> out;
#if defined(__APPLE__) || defined(__linux__)
            extern char **environ;
            if (environ) {
                for (char **e = environ; *e != nullptr; ++e) {
                    const char* kv = *e;
                    const char* eq = std::strchr(kv, '=');
                    if (!eq) continue;
                    std::string key(kv, eq - kv);
                    std::string val(eq + 1);
                    out[key] = Value(val);
                }
            }
#endif
            return Value(out);
        });
        m.fn("setenv", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 2 || !a[0].isString() || !a[1].isString()) return Value(false);
#if defined(__APPLE__) || defined(__linux__)
            int rc = ::setenv(a[0].asString().c_str(), a[1].asString().c_str(), 1);
            return Value(rc == 0);
#else
            return Value(false);
#endif
        });
        m.fn("unsetenv", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 1 || !a[0].isString()) return Value(false);
#if defined(__APPLE__) || defined(__linux__)
            int rc = ::unsetenv(a[0].asString().c_str());
            return Value(rc == 0);
#else
            return Value(false);
#endif
        });
    });
}


