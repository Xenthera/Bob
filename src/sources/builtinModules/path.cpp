#include "path_module.h"
#include "Interpreter.h"
#include <filesystem>
#include <cctype>

namespace fs = std::filesystem;

static std::string join_impl(const std::vector<Value>& parts){
    if (parts.empty()) return std::string();
    fs::path p;
    for (const auto& v : parts) if (v.isString()) p /= v.asString();
    return p.generic_string();
}

static bool isabs_impl(const std::string& s) {
#if defined(_WIN32)
    if (s.size() >= 2 && (s[0] == '/' || s[0] == '\\')) return true; // root-relative on current drive
    if (s.rfind("\\\\", 0) == 0) return true; // UNC path
    if (s.size() >= 3 && std::isalpha(static_cast<unsigned char>(s[0])) && s[1] == ':' && (s[2] == '/' || s[2] == '\\')) return true; // C:\ or C:/
    return false;
#else
    return !s.empty() && s[0] == '/';
#endif
}

void registerPathModule(Interpreter& interpreter) {
    interpreter.registerModule("path", [](Interpreter::ModuleBuilder& m) {
        m.fn("join", [](std::vector<Value> a, int, int) -> Value {
            return Value(join_impl(a));
        });
        m.fn("dirname", [](std::vector<Value> a, int, int) -> Value {
            if (a.size()!=1 || !a[0].isString()) return NONE_VALUE;
            return Value(fs::path(a[0].asString()).parent_path().generic_string());
        });
        m.fn("basename", [](std::vector<Value> a, int, int) -> Value {
            if (a.size()!=1 || !a[0].isString()) return NONE_VALUE;
            return Value(fs::path(a[0].asString()).filename().generic_string());
        });
        m.fn("splitext", [](std::vector<Value> a, int, int) -> Value {
            if (a.size()!=1 || !a[0].isString()) return NONE_VALUE;
            fs::path p(a[0].asString());
            std::string ext = p.has_extension() ? p.extension().generic_string() : std::string("");
            fs::path basePath = p.has_extension() ? (p.parent_path() / p.stem()) : p;
            return Value(std::vector<Value>{ Value(basePath.generic_string()), Value(ext) });
        });
        m.fn("normalize", [](std::vector<Value> a, int, int) -> Value {
            if (a.size()!=1 || !a[0].isString()) return NONE_VALUE;
            return Value(fs::path(a[0].asString()).lexically_normal().generic_string());
        });
        m.fn("isabs", [](std::vector<Value> a, int, int) -> Value {
            if (a.size()!=1 || !a[0].isString()) return Value(false);
            return Value(isabs_impl(a[0].asString()));
        });
        m.fn("relpath", [](std::vector<Value> a, int, int) -> Value {
            if (a.size()<1 || a.size()>2 || !a[0].isString() || (a.size()==2 && !a[1].isString())) return NONE_VALUE;
            fs::path target(a[0].asString());
            fs::path base = (a.size()==2)? fs::path(a[1].asString()) : fs::current_path();
            return Value(fs::relative(target, base).generic_string());
        });
    });
}


