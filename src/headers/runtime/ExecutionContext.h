#pragma once
#include "Value.h"

struct ExecutionContext {
    bool isFunctionBody = false;
    bool hasReturn = false;
    Value returnValue = NONE_VALUE;
    bool shouldBreak = false;
    bool shouldContinue = false;
};
