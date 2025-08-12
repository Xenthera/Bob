// ModuleDef.h
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "Value.h"

struct Module {
    std::string name;
    std::shared_ptr<std::unordered_map<std::string, Value>> exports;
};


