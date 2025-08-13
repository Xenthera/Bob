#pragma once

#include <iostream>
#include <vector>
#include <bitset>
#include <cctype>

inline std::vector<std::string> splitString(const std::string& input, const std::string& delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    size_t start = 0;
    size_t end = input.find(delimiter);

    while (end != std::string::npos) {
        token = input.substr(start, end - start);
        tokens.push_back(token);
        start = end + 1;
        end = input.find(delimiter, start);
    }

    // Add the last token (after the last delimiter)
    token = input.substr(start, end);
    tokens.push_back(token);

    return tokens;
}

inline std::string trim(const std::string& str) {
    // Find the first non-whitespace character
    size_t start = str.find_first_not_of(" \t\n\r");

    // If the string is all whitespace, return an empty string
    if (start == std::string::npos) {
        return "";
    }

    // Find the last non-whitespace character
    size_t end = str.find_last_not_of(" \t\n\r");

    // Extract the trimmed substring
    return str.substr(start, end - start + 1);
}

inline std::string replaceSubstring(const std::string& str, const std::string& findSubstring, const std::string& replacement) {
    std::string result = str;
    size_t startPos = result.find(findSubstring);

    while (startPos != std::string::npos) {
        result.replace(startPos, findSubstring.length(), replacement);
        startPos = result.find(findSubstring, startPos + replacement.length());
    }

    return result;
}

inline bool isHexDigit(char c) {
    return (std::isdigit(c) || (std::isxdigit(c) && std::islower(c)));
}

inline unsigned long long binaryStringToLong(const std::string& binaryString) {
    std::string binaryDigits = binaryString.substr(2);  // Remove the '0b' prefix
    unsigned long long result = 0;
    for (char ch : binaryDigits) {
        result <<= 1;
        result += (ch - '0');
    }
    return result;
}