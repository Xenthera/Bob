#pragma once
#include <array>
#include <memory>

// Value pool for common integer values to reduce allocations
class ValuePool {
private:
    static constexpr int POOL_SIZE = 256; // Cache values from -128 to 127
    static std::array<std::unique_ptr<struct Value>, POOL_SIZE> integerPool;
    static bool initialized;

public:
    static void initialize();
    static struct Value* getInteger(long long value);
    static void cleanup();
};
