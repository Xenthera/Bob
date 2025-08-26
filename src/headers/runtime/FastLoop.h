#pragma once
#include "Value.h"
#include "Environment.h"
#include <memory>

// Fast loop implementation for common patterns to avoid interpreter overhead
class FastLoop {
public:
    // Fast path for simple integer loops
    static Value fastIntegerLoop(long long start, long long end, long long step, 
                                std::shared_ptr<Environment> env, 
                                const std::string& varName);
};
