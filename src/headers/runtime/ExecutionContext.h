#pragma once
#include "Value.h"

struct ExecutionContext {
    bool isFunctionBody = false;
    bool hasReturn = false;
    Value returnValue = NONE_VALUE;
    bool shouldBreak = false;
    bool shouldContinue = false;
    bool hasThrow = false;
    Value thrownValue = NONE_VALUE;
    int throwLine = 0;
    int throwColumn = 0;
};
