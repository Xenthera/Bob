#include "terminal.h"
#include "ModuleRegistry.h"
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>

void registerTerminalModule(Interpreter& interpreter) {
    interpreter.registerModule("terminal", [](ModuleRegistry::ModuleBuilder& m) {
        // size(): returns [width, height] of terminal
        m.fn("size", [](std::vector<Value>, int, int) -> Value {
            struct winsize w;
            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
                return Value(std::vector<Value>{Value((long long)w.ws_col), Value((long long)w.ws_row)});
            } else {
                // Fallback to default size if ioctl fails
                return Value(std::vector<Value>{Value((long long)80), Value((long long)24)});
            }
        });
    });
}
