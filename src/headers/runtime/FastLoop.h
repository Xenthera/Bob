#pragma once
#include "Value.h"
#include "Environment.h"
#include "Statement.h"
#include <memory>

// Fast loop implementation for common patterns to avoid interpreter overhead
class FastLoop {
public:
    // Fast path for simple integer loops
    static Value fastIntegerLoop(long long start, long long end, long long step, 
                                std::shared_ptr<Environment> env, 
                                const std::string& varName);
    
    // Fast path for simple arithmetic loops
    static Value fastArithmeticLoop(const std::string& varName, long long iterations,
                                   std::shared_ptr<Environment> env);
    

};
