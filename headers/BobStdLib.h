#pragma once

#include "Value.h"
#include "Environment.h"
#include <memory>

class Interpreter;
class ErrorReporter;

class BobStdLib {
public:
    static void addToEnvironment(std::shared_ptr<Environment> env, Interpreter& interpreter, ErrorReporter* errorReporter = nullptr);
}; 