#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

// ANSI color codes for terminal output
namespace Colors {
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string WHITE = "\033[37m";
    const std::string BOLD = "\033[1m";
    const std::string RESET = "\033[0m";
}

struct ErrorContext {
    std::string errorType;
    std::string message;
    std::string fileName;
    int line;
    int column;
    std::vector<std::string> callStack;
};

class ErrorReporter {
private:
    std::vector<std::string> sourceLines;
    std::string currentFileName;
    std::vector<std::string> callStack;
    bool hadError = false;
    // Support nested sources (e.g., eval of external files)
    std::vector<std::vector<std::string>> sourceStack;
    std::vector<std::string> fileNameStack;

public:
    ErrorReporter() = default;
    ~ErrorReporter() = default;

    // Load source code for context
    void loadSource(const std::string& source, const std::string& fileName);

    // Report errors with line and column information
    void reportError(int line, int column, const std::string& errorType, const std::string& message, const std::string& operator_ = "", bool showArrow = true);
    
    // Check if an error has been reported
    bool hasReportedError() const { return hadError; }

    // Reset error state (call this between REPL commands)
    void resetErrorState() { hadError = false; }

    // Report errors with full context
    void reportErrorWithContext(const ErrorContext& context);

    // Call stack management
    void pushCallStack(const std::string& functionName);
    void popCallStack();

    // Source push/pop for eval
    void pushSource(const std::string& source, const std::string& fileName);
    void popSource();
    const std::string& getCurrentFileName() const { return currentFileName; }

private:
    void displaySourceContext(int line, int column, const std::string& errorType, const std::string& message, const std::string& operator_ = "", bool showArrow = true);
    void displayCallStack(const std::vector<std::string>& callStack);
    std::string getLineWithArrow(int line, int column);
    std::string colorize(const std::string& text, const std::string& color);
}; 