// BobModuleSDK.h - Single header for Bob module development
#pragma once

// Standard includes
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <utility>

// GMP includes (bundled with Bob)
#include "gmp.h"
#include "gmpxx.h"

// Core Bob types and interfaces
#include "ModuleDef.h"
#include "Value.h"
#include "TypeWrapper.h"
#include "ModuleRegistry.h"
#include "GMPWrapper.h"
#include "ValuePool.h"
#include "RuntimeDiagnostics.h"
#include "FunctionRegistry.h"
#include "ClassRegistry.h"
#include "ExtensionRegistry.h"
#include "Interpreter.h"

// Error handling utilities
#include "helperFunctions/ErrorUtils.h"
