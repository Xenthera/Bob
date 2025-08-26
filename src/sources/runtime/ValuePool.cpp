#include "ValuePool.h"
#include "Value.h"

std::array<std::unique_ptr<Value>, ValuePool::POOL_SIZE> ValuePool::integerPool;
bool ValuePool::initialized = false;

void ValuePool::initialize() {
    if (initialized) return;
    
    for (int i = 0; i < POOL_SIZE; i++) {
        integerPool[i] = std::make_unique<Value>(static_cast<long long>(i - 128));
    }
    initialized = true;
}

Value* ValuePool::getInteger(long long value) {
    if (!initialized) initialize();
    
    if (value >= -128 && value <= 127) {
        return integerPool[value + 128].get();
    }
    return nullptr; // Not in pool
}

void ValuePool::cleanup() {
    for (auto& ptr : integerPool) {
        ptr.reset();
    }
    initialized = false;
}
