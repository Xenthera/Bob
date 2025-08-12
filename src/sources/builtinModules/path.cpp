#include "path_module.h"
#include "Interpreter.h"
#include <filesystem>

namespace fs = std::filesystem;

static std::string join_impl(const std::vector<Value>& parts){
    if (parts.empty()) return std::string();
    fs::path p;
    for (const auto& v : parts) if (v.isString()) p /= v.asString();
    return p.generic_string();
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
            return Value(std::vector<Value>{ Value(p.replace_extension("").generic_string()), Value(p.extension().generic_string()) });
        });
        m.fn("normalize", [](std::vector<Value> a, int, int) -> Value {
            if (a.size()!=1 || !a[0].isString()) return NONE_VALUE;
            return Value(fs::path(a[0].asString()).lexically_normal().generic_string());
        });
        m.fn("isabs", [](std::vector<Value> a, int, int) -> Value {
            if (a.size()!=1 || !a[0].isString()) return Value(false);
            return Value(fs::path(a[0].asString()).is_absolute());
        });
        m.fn("relpath", [](std::vector<Value> a, int, int) -> Value {
            if (a.size()<1 || a.size()>2 || !a[0].isString() || (a.size()==2 && !a[1].isString())) return NONE_VALUE;
            fs::path target(a[0].asString());
            fs::path base = (a.size()==2)? fs::path(a[1].asString()) : fs::current_path();
            return Value(fs::relative(target, base).generic_string());
        });
    });
}


