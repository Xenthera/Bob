#include "../headers/ErrorReporter.h"
#include <algorithm>
#include <iomanip>
#include <string>

// Helper function to find operator in source line
int findOperatorInLine(const std::string& sourceLine, const std::string& operator_) {
    size_t pos = 0;
    while ((pos = sourceLine.find(operator_, pos)) != std::string::npos) {
        // Check if this operator is not inside a string
        bool inString = false;
        for (size_t i = 0; i < pos; i++) {
            if (sourceLine[i] == '"' && (i == 0 || sourceLine[i-1] != '\\')) {
                inString = !inString;
            }
        }
        
        if (!inString) {
            // Check if this is a standalone operator, not part of a larger operator
            bool isStandalone = true;
            
            // Check character before the operator
            if (pos > 0) {
                char before = sourceLine[pos - 1];
                if (before == '&' || before == '|' || before == '=' || before == '<' || before == '>') {
                    isStandalone = false;
                }
            }
            
            // Check character after the operator
            if (pos + operator_.length() < sourceLine.length()) {
                char after = sourceLine[pos + operator_.length()];
                if (after == '&' || after == '|' || after == '=' || after == '<' || after == '>') {
                    isStandalone = false;
                }
            }
            
            if (isStandalone) {
                return static_cast<int>(pos + 1); // Convert to 1-based column
            }
        }
        pos += 1; // Move to next position to continue searching
    }
    
    return 1; // Default to column 1 if not found
}

void ErrorReporter::loadSource(const std::string& source, const std::string& fileName) {
    currentFileName = fileName;
    sourceLines.clear();
    
    std::istringstream iss(source);
    std::string line;
    while (std::getline(iss, line)) {
        sourceLines.push_back(line);
    }
}

void ErrorReporter::reportError(int line, int column, const std::string& errorType, const std::string& message, const std::string& operator_, bool showArrow) {
    hadError = true;
    displaySourceContext(line, column, errorType, message, operator_, showArrow);
    std::cout.flush(); // Ensure output is flushed before any exception is thrown
}

void ErrorReporter::reportErrorWithContext(const ErrorContext& context) {
    hadError = true;
    
    std::cout << "\n";
    std::cout << colorize("╔══════════════════════════════════════════════════════════════╗", Colors::RED) << "\n";
    std::cout << colorize("║                        ERROR REPORT                          ║", Colors::RED) << "\n";
    std::cout << colorize("╚══════════════════════════════════════════════════════════════╝", Colors::RED) << "\n\n";
    
    std::cout << colorize("Error Type: ", Colors::BOLD) << colorize(context.errorType, Colors::RED) << "\n";
    std::cout << colorize("Message: ", Colors::BOLD) << colorize(context.message, Colors::WHITE) << "\n";
    
    if (!context.fileName.empty()) {
        std::cout << colorize("File: ", Colors::BOLD) << colorize(context.fileName, Colors::CYAN) << "\n";
    }
    
    std::cout << colorize("Location: ", Colors::BOLD) << colorize("Line " + std::to_string(context.line) + 
                                                                 ", Column " + std::to_string(context.column), Colors::YELLOW) << "\n\n";
    
    displaySourceContext(context.line, context.column, context.errorType, context.message);
    
    if (!context.callStack.empty()) {
        displayCallStack(context.callStack);
    }
    
    std::cout << "\n";
}

void ErrorReporter::pushCallStack(const std::string& functionName) {
    // This would be called when entering a function
    // Implementation depends on integration with the interpreter
}

void ErrorReporter::popCallStack() {
    // This would be called when exiting a function
    // Implementation depends on integration with the interpreter
}

void ErrorReporter::displaySourceContext(int line, int column, const std::string& errorType, const std::string& message, const std::string& operator_, bool showArrow) {
    if (sourceLines.empty()) {
        std::cout << colorize("Error: ", Colors::RED) << colorize(errorType, Colors::BOLD) << "\n";
        std::cout << colorize("Message: ", Colors::BOLD) << message << "\n";
        std::cout << colorize("Location: ", Colors::BOLD) << "Line " << line << ", Column " << column << "\n";
        return;
    }
    
    static const int ERROR_DISPLAY_MAX_WIDTH = 65;
    int maxWidth = ERROR_DISPLAY_MAX_WIDTH;
    int startLine = std::max(1, line - 4);
    int endLine = std::min(static_cast<int>(sourceLines.size()), line + 2);
    
    for (int i = startLine; i <= endLine; i++) {
        if (i > 0 && i <= static_cast<int>(sourceLines.size())) {
            int lineWidth = static_cast<int>(sourceLines[i-1].length()) + 8;
            maxWidth = std::max(maxWidth, lineWidth);
        }
    }
    
    int errorLineWidth = 8 + column + 1 + static_cast<int>(message.length());
    maxWidth = std::max(maxWidth, errorLineWidth);
    maxWidth = std::max(maxWidth, ERROR_DISPLAY_MAX_WIDTH);
    
    std::cout << colorize("Source Code Context:", Colors::BOLD) << "\n";
    std::cout << colorize("┌" + std::string(maxWidth, '-') + "┐", Colors::BLUE) << "\n";
    
    for (int i = startLine; i <= endLine; i++) {
        std::string lineNum = std::to_string(i);
        std::string linePrefix = "  " + std::string(4 - lineNum.length(), ' ') + lineNum + " | ";
        
        if (i > 0 && i <= static_cast<int>(sourceLines.size())) {
            if (i == line) {
                std::string sourceLine = sourceLines[i-1];
                std::string fullLine = colorize(linePrefix, Colors::RED) + colorize(sourceLine, Colors::YELLOW);
                
                std::cout << fullLine << "\n";
                
                // Draw arrow only if showArrow is true
                if (showArrow) {
                    std::string arrowLine = colorize("       | ", Colors::RED);
                    int safeColumn = std::max(1, std::min(column, static_cast<int>(sourceLine.length() + 1)));
                    arrowLine += std::string(safeColumn - 1, ' ') + colorize("^", Colors::RED) + colorize(" " + message, Colors::RED);
                    
                    std::cout << arrowLine << "\n";
                }
            } else {
                std::string sourceLine = sourceLines[i - 1];
                std::string fullLine = colorize(linePrefix, Colors::BLUE) + sourceLine;              
                std::cout << fullLine << "\n";
            }
        } else {
            std::string fullLine = linePrefix;
            std::cout << colorize(fullLine, Colors::BLUE) << "\n";
        }
    }
    
    std::cout << colorize("└" + std::string(maxWidth, '-') + "┘", Colors::BLUE) << "\n";
    std::cout << colorize("Error: ", Colors::RED) << colorize(errorType, Colors::BOLD) << "\n";
    std::cout << colorize("Message: ", Colors::BOLD) << message << "\n\n";
}

void ErrorReporter::displayCallStack(const std::vector<std::string>& callStack) {
    if (callStack.empty()) return;
    
    static const int CALL_STACK_MAX_WIDTH = 65;
    int maxWidth = CALL_STACK_MAX_WIDTH;
    for (const auto& func : callStack) {
        int funcWidth = static_cast<int>(func.length()) + 6;
        maxWidth = std::max(maxWidth, funcWidth);
    }
    
    std::cout << colorize("Call Stack:", Colors::BOLD) << "\n";
    std::cout << colorize("┌" + std::string(maxWidth - 2, '-') + "┐", Colors::MAGENTA) << "\n";
    
    for (size_t i = 0; i < callStack.size(); i++) {
        std::string prefix = "│ " + std::to_string(i + 1) + ". ";
        std::string line = prefix + callStack[i];
        
        int currentWidth = static_cast<int>(line.length());
        if (currentWidth < maxWidth - 1) {
            line += std::string(maxWidth - 1 - currentWidth, ' ') + "│";
        } else {
            line += " │";
        }
        
        std::cout << colorize(line, Colors::MAGENTA) << "\n";
    }
    
    std::cout << colorize("└" + std::string(maxWidth - 2, '-') + "┘", Colors::MAGENTA) << "\n\n";
}

std::string ErrorReporter::getLineWithArrow(int line, int column) {
    if (line < 1 || line > static_cast<int>(sourceLines.size())) {
        return "";
    }
    
    std::string sourceLine = sourceLines[line - 1];
    std::string arrow = std::string(column - 1, ' ') + "^";
    return sourceLine + "\n" + arrow;
}

std::string ErrorReporter::colorize(const std::string& text, const std::string& color) {
    try {
        return color + text + Colors::RESET;
    } catch (...) {
        return text;
    }
}
