#include "math_module.h"
#include "Interpreter.h"
#include <cmath>

static Value unary_math(std::vector<Value> a, double(*fn)(double)){
    if (a.size() != 1 || !a[0].isNumber()) return NONE_VALUE;
    return Value(fn(a[0].asNumber()));
}

void registerMathModule(Interpreter& interpreter) {
    interpreter.registerModule("math", [](Interpreter::ModuleBuilder& m) {
        m.fn("sin", [](std::vector<Value> a, int, int)->Value{ return unary_math(a, std::sin); });
        m.fn("cos", [](std::vector<Value> a, int, int)->Value{ return unary_math(a, std::cos); });
        m.fn("tan", [](std::vector<Value> a, int, int)->Value{ return unary_math(a, std::tan); });
        m.fn("asin", [](std::vector<Value> a, int, int)->Value{ return unary_math(a, std::asin); });
        m.fn("acos", [](std::vector<Value> a, int, int)->Value{ return unary_math(a, std::acos); });
        m.fn("atan", [](std::vector<Value> a, int, int)->Value{ return unary_math(a, std::atan); });
        m.fn("sinh", [](std::vector<Value> a, int, int)->Value{ return unary_math(a, std::sinh); });
        m.fn("cosh", [](std::vector<Value> a, int, int)->Value{ return unary_math(a, std::cosh); });
        m.fn("tanh", [](std::vector<Value> a, int, int)->Value{ return unary_math(a, std::tanh); });
        m.fn("exp", [](std::vector<Value> a, int, int)->Value{ return unary_math(a, std::exp); });
        m.fn("log", [](std::vector<Value> a, int, int)->Value{ return unary_math(a, std::log); });
        m.fn("log10", [](std::vector<Value> a, int, int)->Value{ return unary_math(a, std::log10); });
        m.fn("sqrt", [](std::vector<Value> a, int, int)->Value{ return unary_math(a, std::sqrt); });
        m.fn("ceil", [](std::vector<Value> a, int, int)->Value{ return unary_math(a, std::ceil); });
        m.fn("floor", [](std::vector<Value> a, int, int)->Value{ return unary_math(a, std::floor); });
        m.fn("round", [](std::vector<Value> a, int, int)->Value{
            if (a.size() != 1 || !a[0].isNumber()) return NONE_VALUE;
            return Value(std::round(a[0].asNumber()));
        });
        m.fn("abs", [](std::vector<Value> a, int, int)->Value{
            if (a.size() != 1 || !a[0].isNumber()) return NONE_VALUE;
            return Value(std::fabs(a[0].asNumber()));
        });
        m.fn("pow", [](std::vector<Value> a, int, int)->Value{
            if (a.size() != 2 || !a[0].isNumber() || !a[1].isNumber()) return NONE_VALUE;
            return Value(std::pow(a[0].asNumber(), a[1].asNumber()));
        });
        m.fn("min", [](std::vector<Value> a, int, int)->Value{
            if (a.empty()) return NONE_VALUE;
            double mval = a[0].isNumber()? a[0].asNumber() : 0.0;
            for(size_t i=1;i<a.size();++i){ if (a[i].isNumber()) mval = std::min(mval, a[i].asNumber()); }
            return Value(mval);
        });
        m.fn("max", [](std::vector<Value> a, int, int)->Value{
            if (a.empty()) return NONE_VALUE;
            double mval = a[0].isNumber()? a[0].asNumber() : 0.0;
            for(size_t i=1;i<a.size();++i){ if (a[i].isNumber()) mval = std::max(mval, a[i].asNumber()); }
            return Value(mval);
        });
        m.val("pi", Value(3.14159265358979323846));
        m.val("e", Value(2.71828182845904523536));
    });
}


