#include "time_module.h"
#include "Interpreter.h"
#include "Environment.h"
#include <chrono>
#include <thread>

void registerTimeModule(Interpreter& interpreter) {
    interpreter.registerModule("time", [](Interpreter::ModuleBuilder& m) {
        m.fn("now", [](std::vector<Value>, int, int) -> Value {
            using namespace std::chrono;
            auto now = system_clock::now().time_since_epoch();
            auto us = duration_cast<microseconds>(now).count();
            return Value(static_cast<double>(us));
        });
        m.fn("monotonic", [](std::vector<Value>, int, int) -> Value {
            using namespace std::chrono;
            auto now = steady_clock::now().time_since_epoch();
            auto us = duration_cast<microseconds>(now).count();
            return Value(static_cast<double>(us));
        });
        m.fn("sleep", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 1) {
                return NONE_VALUE;
            }
            if (!a[0].isNumeric()) {
                return NONE_VALUE;
            }
            double seconds;
            if (a[0].isInteger()) {
                seconds = static_cast<double>(a[0].asInteger());
            } else {
                seconds = a[0].asNumber();
            }
            if (seconds < 0) {
                return NONE_VALUE;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(seconds * 1000)));
            return NONE_VALUE;
        });
    });
}


