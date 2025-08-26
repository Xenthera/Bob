#include "ErrorReporter.h"
#include <algorithm>
#include <iomanip>
#include <string>

void ErrorReporter::loadSource(const std::string& source, const std::string& fileName) {
    currentFileName = fileName;
    sourceLines.clear();
    
    std::istringstream iss(source);
    std::string line;
    while (std::getline(iss, line)) {
        sourceLines.push_back(line);
    }
}

void ErrorReporter::pushSource(const std::string& source, const std::string& fileName) {
    // Save current
    sourceStack.push_back(sourceLines);
    fileNameStack.push_back(currentFileName);
    // Load new
    loadSource(source, fileName);
}

void ErrorReporter::popSource() {
    if (!sourceStack.empty()) {
        sourceLines = sourceStack.back();
        sourceStack.pop_back();
    } else {
        sourceLines.clear();
    }
    if (!fileNameStack.empty()) {
        currentFileName = fileNameStack.back();
        fileNameStack.pop_back();
    } else {
        currentFileName.clear();
    }
}

void ErrorReporter::reportError(const ErrorInfo& error) {
    // Store the error information
    lastError = error;
    errorReported = true;
    
    // If we're inside a try block, don't display the error yet
    // It will be handled by the catch block
    if (isInTry()) {
        return;
    }
    
    // Display the error immediately
    displayError(error);
    std::cout.flush(); // Ensure output is flushed before any exception is thrown
}

void ErrorReporter::reportError(int line, int column, const std::string& errorType, 
                               const std::string& message, const std::string& lexeme, bool showArrow) {
    ErrorInfo error(errorType, message, currentFileName, line, column, lexeme, showArrow);
    reportError(error);
}

void ErrorReporter::displayError(const ErrorInfo& error) {
    std::cout << "\n";
    displaySourceContext(error);
    std::cout << colorize("Error: ", Colors::RED) << colorize(error.errorType, Colors::BOLD) << "\n";
    std::cout << colorize("Message: ", Colors::BOLD) << error.message << "\n\n";
}

void ErrorReporter::displaySourceContext(const ErrorInfo& error) {
    if (sourceLines.empty()) {
        return; // No source available
    }
    
    // Calculate line range to display (4 lines before, 2 lines after)
    int startLine = std::max(1, error.line - 4);
    int endLine = std::min(static_cast<int>(sourceLines.size()), error.line + 2);
    
    // Calculate max width for formatting
    int maxWidth = 80; // Default minimum width
    for (int i = startLine; i <= endLine; i++) {
        if (i > 0 && i <= static_cast<int>(sourceLines.size())) {
            int lineWidth = static_cast<int>(sourceLines[i-1].length()) + 8;
            maxWidth = std::max(maxWidth, lineWidth);
        }
    }
    
    // Add space for error message
    int errorLineWidth = 8 + error.column + 1 + static_cast<int>(error.message.length());
    maxWidth = std::max(maxWidth, errorLineWidth);
    maxWidth = std::min(maxWidth, 120); // Cap at reasonable width
    
    std::cout << colorize("Source Code Context:", Colors::BOLD) << "\n";
    std::cout << colorize("┌" + std::string(maxWidth, '-') + "┐", Colors::BLUE) << "\n";
    
    for (int i = startLine; i <= endLine; i++) {
        std::string lineNum = std::to_string(i);
        std::string linePrefix = "  " + std::string(4 - lineNum.length(), ' ') + lineNum + " | ";
        
        if (i > 0 && i <= static_cast<int>(sourceLines.size())) {
            if (i == error.line) {
                // Error line - highlight it
                std::string sourceLine = sourceLines[i-1];
                std::string fullLine = colorize(linePrefix, Colors::RED) + colorize(sourceLine, Colors::YELLOW);
                std::cout << fullLine << "\n";
                
                // Draw arrow pointing to the error column
                if (error.showArrow) {
                    std::string arrowLine = colorize("       | ", Colors::RED);
                    int safeColumn = std::max(1, std::min(error.column, static_cast<int>(sourceLine.length() + 1)));
                    arrowLine += std::string(safeColumn - 1, ' ') + colorize("^", Colors::RED) + 
                                colorize(" " + error.message, Colors::RED);
                    std::cout << arrowLine << "\n";
                }
            } else {
                // Normal line
                std::string sourceLine = sourceLines[i - 1];
                std::string fullLine = colorize(linePrefix, Colors::BLUE) + sourceLine;              
                std::cout << fullLine << "\n";
            }
        } else {
            // Empty line
            std::string fullLine = linePrefix;
            std::cout << colorize(fullLine, Colors::BLUE) << "\n";
        }
    }
    
    std::cout << colorize("└" + std::string(maxWidth, '-') + "┘", Colors::BLUE) << "\n";
}

std::string ErrorReporter::colorize(const std::string& text, const std::string& color) {
    try {
        return color + text + Colors::RESET;
    } catch (...) {
        return text;
    }
}
