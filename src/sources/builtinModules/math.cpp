#include "math_module.h"
#include "Interpreter.h"
#include <cmath>

static Value unary_math(std::vector<Value> a, double(*fn)(double)){
    if (a.size() != 1 || !a[0].isNumeric()) return NONE_VALUE;
    double value = a[0].isInteger() ? static_cast<double>(a[0].asInteger()) : a[0].asNumber();
    return Value(fn(value));
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
            if (a.size() != 1 || !a[0].isNumeric()) return NONE_VALUE;
            double value = a[0].isInteger() ? static_cast<double>(a[0].asInteger()) : a[0].asNumber();
            return Value(std::round(value));
        });
        m.fn("abs", [](std::vector<Value> a, int, int)->Value{
            if (a.size() != 1 || !a[0].isNumeric()) return NONE_VALUE;
            double value = a[0].isInteger() ? static_cast<double>(a[0].asInteger()) : a[0].asNumber();
            return Value(std::fabs(value));
        });
        m.fn("pow", [](std::vector<Value> a, int, int)->Value{
            if (a.size() != 2 || !a[0].isNumeric() || !a[1].isNumeric()) return NONE_VALUE;
            double base = a[0].isInteger() ? static_cast<double>(a[0].asInteger()) : a[0].asNumber();
            double exp = a[1].isInteger() ? static_cast<double>(a[1].asInteger()) : a[1].asNumber();
            return Value(std::pow(base, exp));
        });
        m.fn("min", [](std::vector<Value> a, int, int)->Value{
            if (a.empty()) return NONE_VALUE;
            double mval = a[0].isNumeric() ? 
                (a[0].isInteger() ? static_cast<double>(a[0].asInteger()) : a[0].asNumber()) : 0.0;
            for(size_t i=1;i<a.size();++i){ 
                if (a[i].isNumeric()) {
                    double val = a[i].isInteger() ? static_cast<double>(a[i].asInteger()) : a[i].asNumber();
                    mval = std::min(mval, val); 
                }
            }
            return Value(mval);
        });
        m.fn("max", [](std::vector<Value> a, int, int)->Value{
            if (a.empty()) return NONE_VALUE;
            double mval = a[0].isNumeric() ? 
                (a[0].isInteger() ? static_cast<double>(a[0].asInteger()) : a[0].asNumber()) : 0.0;
            for(size_t i=1;i<a.size();++i){ 
                if (a[i].isNumeric()) {
                    double val = a[i].isInteger() ? static_cast<double>(a[i].asInteger()) : a[i].asNumber();
                    mval = std::max(mval, val); 
                }
            }
            return Value(mval);
        });
        m.val("pi", Value(3.14159265358979323846));
        m.val("e", Value(2.71828182845904523536));
    });
}


