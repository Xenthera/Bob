#include "eval.h"
#include "Interpreter.h"
#include "ErrorReporter.h"
#include "Lexer.h"
#include "Parser.h"
#include <fstream>
#include <sstream>

void registerEvalModule(Interpreter& interpreter) {
    interpreter.registerModule("eval", [](Interpreter::ModuleBuilder& m) {
        ErrorReporter* er = m.interpreterRef.getErrorReporter();
        m.fn("eval", [er, &I = m.interpreterRef](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1 || !args[0].isString()) {
                if (er) er->reportError(line, column, "Invalid Arguments", "eval expects exactly 1 string argument", "eval");
                throw std::runtime_error("eval expects exactly 1 string argument");
            }
            std::string code = args[0].asString();
            std::string evalName = "<eval>";
            try {
                if (er) er->pushSource(code, evalName);
                Lexer lx; if (er) lx.setErrorReporter(er);
                auto toks = lx.Tokenize(code);
                Parser p(toks); if (er) p.setErrorReporter(er);
                auto stmts = p.parse();
                I.interpret(stmts);
                return NONE_VALUE;
            } catch (...) {
                if (er) er->popSource();
                throw;
            }
            if (er) er->popSource();
        });

        m.fn("evalFile", [er, &I = m.interpreterRef](std::vector<Value> args, int line, int column) -> Value {
            if (args.size() != 1 || !args[0].isString()) {
                if (er) er->reportError(line, column, "Invalid Arguments", "evalFile expects exactly 1 string argument (path)", "evalFile");
                throw std::runtime_error("evalFile expects exactly 1 string argument (path)");
            }
            std::string filename = args[0].asString();
            std::ifstream f(filename);
            if (!f.is_open()) {
                if (er) er->reportError(line, column, "StdLib Error", "Could not open file: " + filename, "");
                throw std::runtime_error("Could not open file: " + filename);
            }
            std::stringstream buf; buf << f.rdbuf(); f.close();
            std::string code = buf.str();
            try {
                if (er) er->pushSource(code, filename);
                Lexer lx; if (er) lx.setErrorReporter(er);
                auto toks = lx.Tokenize(code);
                Parser p(toks); if (er) p.setErrorReporter(er);
                auto stmts = p.parse();
                I.interpret(stmts);
                return NONE_VALUE;
            } catch (...) {
                if (er) er->popSource();
                throw;
            }
            if (er) er->popSource();
        });
    });
}


