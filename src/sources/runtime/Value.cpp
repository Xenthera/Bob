#include "Value.h"

// Global constants for common values (no heap allocation)
const Value NONE_VALUE = Value();
const Value TRUE_VALUE = Value(true);
const Value FALSE_VALUE = Value(false);
const Value ZERO_VALUE = Value(0.0);
const Value ONE_VALUE = Value(1.0); 