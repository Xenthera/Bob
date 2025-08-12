#include <utility>

#include "bob.h"
#include "Parser.h"

void Bob::ensureInterpreter(bool interactive) {
    if (!interpreter) interpreter = msptr(Interpreter)(interactive);
    applyPendingConfigs();
}

void Bob::runFile(const std::string& path)
{
    ensureInterpreter(false);
    interpreter->addStdLibFunctions();
    if (!evalFile(path)) {
        std::cout << "Execution failed\n";
    }
}

void Bob::runPrompt()
{
    ensureInterpreter(true);
    std::cout << "Bob v" << VERSION << ", 2025\n";
    while(true)
    {
        std::string line;
        std::cout << "\033[0;36m" << "-> " << "\033[0;37m";
        std::getline(std::cin, line);

        if(std::cin.eof())
        {
            break;
        }

        // Reset error state before each REPL command
        errorReporter.resetErrorState();
        interpreter->addStdLibFunctions();
        (void)evalString(line, "REPL");
    }
}

bool Bob::evalFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;
    std::string src((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    errorReporter.loadSource(src, path);
    interpreter->setErrorReporter(&errorReporter);
    try {
        lexer.setErrorReporter(&errorReporter);
        auto tokens = lexer.Tokenize(src);
        Parser p(tokens);
        p.setErrorReporter(&errorReporter);
        auto statements = p.parse();
        interpreter->interpret(statements);
        return true;
    } catch (...) { return false; }
}

bool Bob::evalString(const std::string& code, const std::string& filename) {
    errorReporter.loadSource(code, filename);
    interpreter->setErrorReporter(&errorReporter);
    try {
        lexer.setErrorReporter(&errorReporter);
        auto tokens = lexer.Tokenize(code);
        Parser p(tokens);
        p.setErrorReporter(&errorReporter);
        auto statements = p.parse();
        interpreter->interpret(statements);
        return true;
    } catch (...) { return false; }
}


