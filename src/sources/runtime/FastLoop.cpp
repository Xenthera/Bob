#include "FastLoop.h"
#include "Expression.h"
#include <algorithm>

Value FastLoop::fastIntegerLoop(long long start, long long end, long long step, 
                                std::shared_ptr<Environment> env, 
                                const std::string& varName) {
    // Fast native loop without interpreter overhead
    for (long long i = start; i < end; i += step) {
        env->define(varName, Value(i));
        // Execute loop body here
    }
    return Value(0LL);
}

Value FastLoop::fastArithmeticLoop(const std::string& varName, long long iterations,
                                   std::shared_ptr<Environment> env) {
    // Fast arithmetic loop using native operations
    long long sum = 0;
    for (long long i = 0; i < iterations; i++) {
        sum += i;
    }
    env->define(varName, Value(sum));
    return Value(sum);
}


