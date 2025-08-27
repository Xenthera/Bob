#pragma once

#include <string>
#include <vector>
#include <memory>

#ifdef _WIN32
// Windows doesn't have termios
#else
#include <termios.h>
#endif

class LineEditor {
public:
    LineEditor();
    ~LineEditor();
    
    // Get a line of input with history support
    std::string getLine(const std::string& prompt);
    
    // Add a line to history
    void addToHistory(const std::string& line);
    
    // Clear history
    void clearHistory();
    
    // Set history size limit
    void setHistorySize(size_t size);
    
    // Get history as a vector (for debugging/testing)
    const std::vector<std::string>& getHistory() const { return history; }

private:
    std::vector<std::string> history;
    size_t historyIndex;
    size_t maxHistorySize;
};
