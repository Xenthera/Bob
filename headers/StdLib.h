#pragma once

#include "Value.h"
#include "Environment.h"
#include <memory>

class Interpreter;

class StdLib {
public:
    static void addToEnvironment(std::shared_ptr<Environment> env, Interpreter& interpreter);
}; 