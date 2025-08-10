#pragma once

#include "Value.h"
#include "Lexer.h"

// Utility to compute the result of a compound assignment (e.g., +=, -=, etc.)
// Leaves error reporting to callers; throws std::runtime_error on unknown operator
inline Value computeCompoundAssignment(const Value& currentValue, TokenType opType, const Value& rhs) {
    switch (opType) {
        case PLUS_EQUAL:
            return currentValue + rhs;
        case MINUS_EQUAL:
            return currentValue - rhs;
        case STAR_EQUAL:
            return currentValue * rhs;
        case SLASH_EQUAL:
            return currentValue / rhs;
        case PERCENT_EQUAL:
            return currentValue % rhs;
        case BIN_AND_EQUAL:
            return currentValue & rhs;
        case BIN_OR_EQUAL:
            return currentValue | rhs;
        case BIN_XOR_EQUAL:
            return currentValue ^ rhs;
        case BIN_SLEFT_EQUAL:
            return currentValue << rhs;
        case BIN_SRIGHT_EQUAL:
            return currentValue >> rhs;
        default:
            throw std::runtime_error("Unknown compound assignment operator");
    }
}


