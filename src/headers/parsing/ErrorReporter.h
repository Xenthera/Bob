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

// Centralized error information structure
struct ErrorInfo {
    std::string errorType;
    std::string message;
    std::string fileName;
    int line;
    int column;
    std::string lexeme;
    bool showArrow;
    
    // Default constructor
    ErrorInfo() : line(0), column(0), showArrow(true) {}
    
    // Constructor with parameters
    ErrorInfo(const std::string& type, const std::string& msg, const std::string& file, 
              int l, int c, const std::string& lex = "", bool arrow = true)
        : errorType(type), message(msg), fileName(file), line(l), column(c), 
          lexeme(lex), showArrow(arrow) {}
};

class ErrorReporter {
private:
    std::vector<std::string> sourceLines;
    std::string currentFileName;
    
    // Support nested sources (e.g., eval of external files)
    std::vector<std::vector<std::string>> sourceStack;
    std::vector<std::string> fileNameStack;
    
    // Try/catch state management
    int tryDepth = 0;
    bool errorReported = false;
    ErrorInfo lastError;

public:
    ErrorReporter() = default;
    ~ErrorReporter() = default;

    // Main error reporting interface - works with try/catch
    void reportError(const ErrorInfo& error);
    
    // Convenience overloads for common cases
    void reportError(int line, int column, const std::string& errorType, 
                    const std::string& message, const std::string& lexeme = "", 
                    bool showArrow = true);
    
    // Source management
    void loadSource(const std::string& source, const std::string& fileName);
    void pushSource(const std::string& source, const std::string& fileName);
    void popSource();
    
    // Try/catch integration
    void enterTry() { tryDepth++; }
    void exitTry() { if (tryDepth > 0) tryDepth--; }
    bool isInTry() const { return tryDepth > 0; }
    
    // Error state management
    bool hasError() const { return errorReported; }
    const ErrorInfo& getLastError() const { return lastError; }
    void clearError() { errorReported = false; }
    void resetErrorState() { errorReported = false; tryDepth = 0; lastError = ErrorInfo(); }
    
    const std::string& getCurrentFileName() const { return currentFileName; }

private:
    void displayError(const ErrorInfo& error);
    void displaySourceContext(const ErrorInfo& error);
    std::string colorize(const std::string& text, const std::string& color);
}; 