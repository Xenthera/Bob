#include "Value.h"

// Global constants for common values (no heap allocation)
const Value NONE_VALUE = Value();
const Value TRUE_VALUE = Value(true);
const Value FALSE_VALUE = Value(false);

// Helper to format module string safely with complete type available in this TU
std::string formatModuleForToString(const std::shared_ptr<Module>& mod) {
    if (mod && !mod->name.empty()) {
        return std::string("<module '") + mod->name + "'>";
    }
    return std::string("<module>");
}
 