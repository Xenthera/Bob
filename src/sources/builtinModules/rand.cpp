#include "rand.h"
#include "Interpreter.h"
#include <random>

void registerRandModule(Interpreter& interpreter) {
    interpreter.registerModule("rand", [](Interpreter::ModuleBuilder& m) {
        static std::mt19937_64 rng{std::random_device{}()};
        m.fn("seed", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() == 1 && a[0].isNumber()) {
                rng.seed(static_cast<uint64_t>(a[0].asNumber()));
            }
            return NONE_VALUE;
        });
        m.fn("random", [](std::vector<Value>, int, int) -> Value {
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            return Value(dist(rng));
        });
        m.fn("randint", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 2 || !a[0].isNumber() || !a[1].isNumber()) return NONE_VALUE;
            long long lo = static_cast<long long>(a[0].asNumber());
            long long hi = static_cast<long long>(a[1].asNumber());
            if (hi < lo) std::swap(lo, hi);
            std::uniform_int_distribution<long long> dist(lo, hi);
            return Value(static_cast<double>(dist(rng)));
        });
        m.fn("choice", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 1 || !a[0].isArray() || a[0].asArray().empty()) return NONE_VALUE;
            const auto& arr = a[0].asArray();
            std::uniform_int_distribution<size_t> dist(0, arr.size() - 1);
            return arr[dist(rng)];
        });
    });
}


