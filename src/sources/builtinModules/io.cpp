#include <fstream>
#include <sstream>
#include "io.h"
#include "Interpreter.h"
#include "ErrorReporter.h"

void registerIoModule(Interpreter& interpreter) {
    interpreter.registerModule("io", [](Interpreter::ModuleBuilder& m) {
        ErrorReporter* er = m.interpreterRef.getErrorReporter();

        m.fn("readFile", [er](std::vector<Value> a, int line, int col) -> Value {
            if (a.empty() || !a[0].isString() || a.size() > 2 || (a.size() == 2 && !a[1].isString())) {
                if (er) er->reportError(line, col, "Invalid Arguments", "readFile(path[, mode]) expects 1-2 args (strings)", "readFile");
                throw std::runtime_error("readFile(path[, mode]) expects 1-2 string args");
            }
            std::string mode = (a.size() == 2) ? a[1].asString() : std::string("r");
            std::ios_base::openmode om = std::ios::in;
            if (mode.find('b') != std::string::npos) om |= std::ios::binary;
            std::ifstream f(a[0].asString(), om);
            if (!f.is_open()) {
                if (er) er->reportError(line, col, "StdLib Error", "Could not open file", a[0].asString());
                throw std::runtime_error("Could not open file");
            }
            std::stringstream buf; buf << f.rdbuf(); f.close();
            return Value(buf.str());
        });

        m.fn("writeFile", [er](std::vector<Value> a, int line, int col) -> Value {
            if (a.size() < 2 || a.size() > 3 || !a[0].isString() || !a[1].isString() || (a.size() == 3 && !a[2].isString())) {
                if (er) er->reportError(line, col, "Invalid Arguments", "writeFile(path, data[, mode]) expects 2-3 args (strings)", "writeFile");
                throw std::runtime_error("writeFile(path, data[, mode]) expects 2-3 string args");
            }
            std::string mode = (a.size() == 3) ? a[2].asString() : std::string("w");
            std::ios_base::openmode om = std::ios::out;
            if (mode.find('b') != std::string::npos) om |= std::ios::binary;
            if (mode.find('a') != std::string::npos) om |= std::ios::app; else om |= std::ios::trunc;
            std::ofstream f(a[0].asString(), om);
            if (!f.is_open()) {
                if (er) er->reportError(line, col, "StdLib Error", "Could not create file", a[0].asString());
                throw std::runtime_error("Could not create file");
            }
            f << a[1].asString(); f.close();
            return NONE_VALUE;
        });

        m.fn("readLines", [er](std::vector<Value> a, int line, int col) -> Value {
            if (a.size() != 1 || !a[0].isString()) {
                if (er) er->reportError(line, col, "Invalid Arguments", "readLines(path) expects 1 string arg", "readLines");
                throw std::runtime_error("readLines(path) expects 1 string arg");
            }
            std::ifstream f(a[0].asString());
            if (!f.is_open()) {
                if (er) er->reportError(line, col, "StdLib Error", "Could not open file", a[0].asString());
                throw std::runtime_error("Could not open file");
            }
            std::vector<Value> lines; std::string s;
            while (std::getline(f, s)) lines.emplace_back(s);
            f.close();
            return Value(lines);
        });

        m.fn("exists", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 1 || !a[0].isString()) return Value(false);
            std::ifstream f(a[0].asString()); bool ok = f.good(); f.close();
            return Value(ok);
        });

        // input remains a global in stdlib; not provided here
    });
}


