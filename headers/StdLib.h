#pragma once

#include "TypeWrapper.h"
#include "Environment.h"
#include <functional>

class Interpreter;  // Forward declaration

class StdLib {
public:
    static void addToEnvironment(sptr(Environment) env, Interpreter* interpreter);
}; 