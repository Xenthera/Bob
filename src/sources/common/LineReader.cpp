#include "LineReader.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#endif

static std::vector<std::string> history;
static size_t historyIndex = 0;

char* lineReader(const char* prompt) {
    // Reset history index for new line
    historyIndex = history.size();
    
    std::cout << prompt;
    std::cout.flush();
    
    std::string line;
    size_t cursorPos = 0;  // Track cursor position
    
#ifdef _WIN32
    // Windows implementation
    int ch;
    while ((ch = _getch()) != '\r' && ch != '\n') {
        if (ch == 224) {
            // Extended key
            ch = _getch();
            if (ch == 72) {
                // Up arrow - history
                if (historyIndex > 0) {
                    historyIndex--;
                    line = history[historyIndex];
                    cursorPos = line.length();
                    // Clear current line and redraw
                    std::cout << "\r\033[K" << prompt << line;
                }
            } else if (ch == 80) {
                // Down arrow - history
                if (historyIndex < history.size()) {
                    historyIndex++;
                    if (historyIndex < history.size()) {
                        line = history[historyIndex];
                    } else {
                        line.clear();
                    }
                    cursorPos = line.length();
                    // Clear current line and redraw
                    std::cout << "\r\033[K" << prompt << line;
                }
            } else if (ch == 75) {
                // Left arrow - move cursor left
                if (cursorPos > 0) {
                    cursorPos--;
                    std::cout << "\b";
                }
            } else if (ch == 77) {
                // Right arrow - move cursor right
                if (cursorPos < line.length()) {
                    cursorPos++;
                    std::cout << "\033[C";
                }
            }
        } else if (ch == 8) {
            // Backspace at cursor position
            if (cursorPos > 0) {
                line.erase(cursorPos - 1, 1);
                cursorPos--;
                std::cout << "\b \b";
                // Redraw trailing characters if we're not at the end
                if (cursorPos < line.length()) {
                    for (size_t i = cursorPos; i < line.length(); i++) {
                        std::cout << line[i];
                    }
                    std::cout << " "; // Clear the last character
                    // Move cursor back to position
                    std::cout << "\033[" << (line.length() - cursorPos + 1) << "D";
                }
            }
        } else if (ch >= 32 && ch <= 126) {
            // Printable character - insert at cursor
            if (cursorPos == line.length()) {
                // At end of line - simple append
                line += static_cast<char>(ch);
                std::cout << static_cast<char>(ch);
                cursorPos++;
            } else {
                // In middle of line - need to redraw
                line.insert(cursorPos, 1, static_cast<char>(ch));
                std::cout << static_cast<char>(ch);
                cursorPos++;
                // Redraw rest of line
                for (size_t i = cursorPos; i < line.length(); i++) {
                    std::cout << line[i];
                }
                // Move cursor back to position
                std::cout << "\033[" << (line.length() - cursorPos) << "D";
            }
        }
    }
    std::cout << std::endl;
#else
    // Unix/Linux/BSD implementation
    struct termios old_termios, new_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_termios);
    
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (ch == 27) {
            // ESC sequence
            ch = getchar();
            if (ch == '[') {
                ch = getchar();
                if (ch == 'A') {
                    // Up arrow - history
                    if (historyIndex > 0) {
                        historyIndex--;
                        line = history[historyIndex];
                        cursorPos = line.length();
                        // Clear current line and redraw
                        std::cout << "\r\033[K" << prompt << line;
                    }
                } else if (ch == 'B') {
                    // Down arrow - history
                    if (historyIndex < history.size()) {
                        historyIndex++;
                        if (historyIndex < history.size()) {
                            line = history[historyIndex];
                        } else {
                            line.clear();
                        }
                        cursorPos = line.length();
                        // Clear current line and redraw
                        std::cout << "\r\033[K" << prompt << line;
                    }
                } else if (ch == 'D') {
                    // Left arrow - move cursor left
                    if (cursorPos > 0) {
                        cursorPos--;
                        std::cout << "\b";
                    }
                } else if (ch == 'C') {
                    // Right arrow - move cursor right
                    if (cursorPos < line.length()) {
                        cursorPos++;
                        std::cout << "\033[C";
                    }
                }
            }
        } else if (ch == 127) {
            // Backspace at cursor position
            if (cursorPos > 0) {
                line.erase(cursorPos - 1, 1);
                cursorPos--;
                std::cout << "\b \b";
                // Redraw trailing characters if we're not at the end
                if (cursorPos < line.length()) {
                    for (size_t i = cursorPos; i < line.length(); i++) {
                        std::cout << line[i];
                    }
                    std::cout << " "; // Clear the last character
                    // Move cursor back to position
                    std::cout << "\033[" << (line.length() - cursorPos + 1) << "D";
                }
            }
        } else if (ch >= 32 && ch <= 126) {
            // Printable character - insert at cursor
            if (cursorPos == line.length()) {
                // At end of line - simple append
                line += static_cast<char>(ch);
                std::cout << static_cast<char>(ch);
                cursorPos++;
            } else {
                // In middle of line - need to redraw
                line.insert(cursorPos, 1, static_cast<char>(ch));
                std::cout << static_cast<char>(ch);
                cursorPos++;
                // Redraw rest of line
                for (size_t i = cursorPos; i < line.length(); i++) {
                    std::cout << line[i];
                }
                // Move cursor back to position
                std::cout << "\033[" << (line.length() - cursorPos) << "D";
            }
        }
    }
    std::cout << std::endl;
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_termios);
#endif
    
    if (std::cin.eof()) {
        return nullptr;
    }
    
    char* result = new char[line.length() + 1];
    strcpy(result, line.c_str());
    return result;
}

int lineReaderHistoryAdd(const char* line) {
    if (line && strlen(line) > 0) {
        history.push_back(line);
        if (history.size() > 100) {
            history.erase(history.begin());
        }
    }
    return 1;
}

