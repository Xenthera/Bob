#include "os.h"
#include "Interpreter.h"
#include "Lexer.h"
#include <vector>
#include <string>
#include <filesystem>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

namespace fs = std::filesystem;

void registerOsModule(Interpreter& interpreter) {
    interpreter.registerModule("os", [](Interpreter::ModuleBuilder& m) {
        // Process
        m.fn("getcwd", [](std::vector<Value>, int, int) -> Value {
            char buf[PATH_MAX];
            if (getcwd(buf, sizeof(buf))) return Value(std::string(buf));
            return NONE_VALUE;
        });
        m.fn("chdir", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 1 || !a[0].isString()) return Value(false);
            int rc = ::chdir(a[0].asString().c_str());
            return Value(rc == 0);
        });
        m.fn("getpid", [](std::vector<Value>, int, int) -> Value {
            return Value(static_cast<double>(getpid()));
        });
        m.fn("getppid", [](std::vector<Value>, int, int) -> Value {
            return Value(static_cast<double>(getppid()));
        });
        m.fn("name", [](std::vector<Value>, int, int) -> Value {
#if defined(_WIN32)
            return Value(std::string("nt"));
#else
            return Value(std::string("posix"));
#endif
        });

        // Filesystem
        m.fn("listdir", [](std::vector<Value> a, int, int) -> Value {
            std::string path = ".";
            if (!a.empty() && a[0].isString()) path = a[0].asString();
            std::vector<Value> out;
            try {
                for (const auto& entry : fs::directory_iterator(path)) {
                    out.push_back(Value(entry.path().filename().string()));
                }
            } catch (...) {}
            return Value(out);
        });
        m.fn("mkdir", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 1 || !a[0].isString()) return Value(false);
            try { return Value(fs::create_directory(a[0].asString())); } catch (...) { return Value(false); }
        });
        m.fn("rmdir", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 1 || !a[0].isString()) return Value(false);
            try { return Value(fs::remove(a[0].asString())); } catch (...) { return Value(false); }
        });
        m.fn("remove", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 1 || !a[0].isString()) return Value(false);
            try { return Value(fs::remove(a[0].asString())); } catch (...) { return Value(false); }
        });
        m.fn("exists", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 1 || !a[0].isString()) return Value(false);
            try { return Value(fs::exists(a[0].asString())); } catch (...) { return Value(false); }
        });
        m.fn("isfile", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 1 || !a[0].isString()) return Value(false);
            try { return Value(fs::is_regular_file(a[0].asString())); } catch (...) { return Value(false); }
        });
        m.fn("isdir", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 1 || !a[0].isString()) return Value(false);
            try { return Value(fs::is_directory(a[0].asString())); } catch (...) { return Value(false); }
        });
        m.fn("rename", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 2 || !a[0].isString() || !a[1].isString()) return Value(false);
            try { fs::rename(a[0].asString(), a[1].asString()); return Value(true); } catch (...) { return Value(false); }
        });

        // Separators
        m.fn("sep", [](std::vector<Value>, int, int) -> Value {
#if defined(_WIN32)
            return Value(std::string("\\"));
#else
            return Value(std::string("/"));
#endif
        });
        m.fn("pathsep", [](std::vector<Value>, int, int) -> Value {
#if defined(_WIN32)
            return Value(std::string(";"));
#else
            return Value(std::string(":"));
#endif
        });
        m.fn("linesep", [](std::vector<Value>, int, int) -> Value {
#if defined(_WIN32)
            return Value(std::string("\r\n"));
#else
            return Value(std::string("\n"));
#endif
        });
    });
}


