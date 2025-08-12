#include "sys.h"
#include "Interpreter.h"
#include "Environment.h"
#include "Lexer.h" // for Token and IDENTIFIER
#include <unistd.h>
#include <limits.h>
#include <cstdlib>
#include <cstring>
#include <vector>

// Platform-specific includes for memoryUsage()
#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>
#elif defined(__linux__)
#include <fstream>
#include <sstream>
#elif defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#endif

void registerSysModule(Interpreter& interpreter) {
    interpreter.registerModule("sys", [](Interpreter::ModuleBuilder& m) {
        Interpreter& I = m.interpreterRef;
        m.fn("platform", [](std::vector<Value>, int, int) -> Value {
#if defined(_WIN32)
            return Value(std::string("win32"));
#elif defined(__APPLE__)
            return Value(std::string("darwin"));
#elif defined(__linux__)
            return Value(std::string("linux"));
#else
            return Value(std::string("unknown"));
#endif
        });
        m.fn("version", [](std::vector<Value>, int, int) -> Value { return Value(std::string("0.0.3")); });
        // argv(): array of strings
        m.fn("argv", [&I](std::vector<Value>, int, int) -> Value {
            std::vector<Value> out;
            for (const auto& s : I.getArgv()) out.push_back(Value(s));
            return Value(out);
        });
        // executable(): absolute path to the running binary (host-provided)
        m.fn("executable", [&I](std::vector<Value>, int, int) -> Value { return Value(I.getExecutablePath()); });
        // modules(): read-only snapshot of module cache
        m.fn("modules", [&I](std::vector<Value>, int, int) -> Value {
            Value dictVal = Value(std::unordered_map<std::string, Value>{});
            auto snapshot = I.getModuleCacheSnapshot();
            return Value(snapshot);
        });
        // memoryUsage(): process RSS in MB (best effort per-platform)
        m.fn("memoryUsage", [](std::vector<Value> a, int, int) -> Value {
            if (!a.empty()) return NONE_VALUE;
            size_t memoryBytes = 0;
#if defined(__APPLE__) && defined(__MACH__)
            // macOS
            struct mach_task_basic_info info;
            mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
            if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &infoCount) == KERN_SUCCESS) {
                memoryBytes = info.resident_size;
            }
#elif defined(__linux__)
            // Linux
            std::ifstream statusFile("/proc/self/status");
            std::string line;
            while (std::getline(statusFile, line)) {
                if (line.substr(0, 6) == "VmRSS:") {
                    std::istringstream iss(line);
                    std::string label, value, unit;
                    iss >> label >> value >> unit;
                    memoryBytes = std::stoull(value) * 1024; // KB -> bytes
                    break;
                }
            }
#elif defined(_WIN32)
            // Windows
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
                memoryBytes = pmc.WorkingSetSize;
            }
#endif
            double memoryMB = static_cast<double>(memoryBytes) / (1024.0 * 1024.0);
            return Value(memoryMB);
        });
        m.fn("exit", [](std::vector<Value> a, int, int) -> Value {
            int code = 0; if (!a.empty() && a[0].isNumber()) code = static_cast<int>(a[0].asNumber());
            std::exit(code);
            return NONE_VALUE;
        });
        // env/cwd/pid moved to os; keep sys minimal
    });
}


