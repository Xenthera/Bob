#include "LineEditor.h"
#include "LineReader.h"
#include <iostream>
#include <algorithm>

LineEditor::LineEditor() 
    : historyIndex(0), maxHistorySize(100) {
}

LineEditor::~LineEditor() {
}

std::string LineEditor::getLine(const std::string& prompt) {
    // Use lineReader for proper line editing and history
    char* line = lineReader(prompt.c_str());
    
    if (line == nullptr) {
        // EOF or Ctrl+C
        return "";
    }
    
    std::string result(line);
    free(line);
    
    // Add to history if not empty
    if (!result.empty()) {
        addToHistory(result);
    }
    
    return result;
}

void LineEditor::addToHistory(const std::string& line) {
    if (line.empty()) return;
    
    // Don't add duplicate consecutive lines
    if (!history.empty() && history.back() == line) return;
    
    // Add to our internal history
    history.push_back(line);
    if (history.size() > maxHistorySize) {
        history.erase(history.begin());
    }
    
    // Add to lineReader history
    lineReaderHistoryAdd(line.c_str());
}

void LineEditor::clearHistory() {
    history.clear();
    historyIndex = 0;
}

void LineEditor::setHistorySize(size_t size) {
    maxHistorySize = size;
    if (history.size() > maxHistorySize) {
        history.erase(history.begin(), history.begin() + (history.size() - maxHistorySize));
    }
}


