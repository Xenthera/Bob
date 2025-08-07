#pragma once
#include <string>

// Common error message utilities
namespace ErrorUtils {
    // Generate consistent operator error messages with single quotes
    inline std::string makeOperatorError(const std::string& op, const std::string& leftType, const std::string& rightType) {
        return "'" + op + "' is not supported between '" + leftType + "' and '" + rightType + "'";
    }
} 