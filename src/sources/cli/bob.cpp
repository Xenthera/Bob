#include <utility>

#include "bob.h"
#include "Parser.h"
#include "LineEditor.h"
#include "CppModuleLoader.h"

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
    
    LineEditor editor;
    editor.setHistorySize(100);
    
    while(true)
    {
        std::string line = editor.getLine("\033[0;36m-> \033[0;37m");

        if(std::cin.eof())
        {
            break;
        }
        
        // Skip empty lines but don't quit
        if(line.empty())
        {
            continue;
        }



        // Handle special REPL commands
        if (line == "history" || line == "history;") {
            std::cout << "Command History:\n";
            const auto& hist = editor.getHistory();
            for (size_t i = 0; i < hist.size(); ++i) {
                std::cout << "  " << (i + 1) << ": " << hist[i] << "\n";
            }
            continue;
        } else if (line == "clear" || line == "clear;") {
            editor.clearHistory();
            std::cout << "History cleared.\n";
            continue;
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
    ensureInterpreter(false);
    interpreter->addStdLibFunctions();
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

void Bob::installModule(const std::string& modulePath) {
    // Ensure interpreter is initialized
    ensureInterpreter(false);
    
    // Set up modules directory relative to current working directory
    std::string modulesDirectory = "./bob-modules/";
    interpreter->getModuleRegistry().setModulesDirectory(modulesDirectory);
    
    // Install the module
    CppModuleLoader::installModule(modulePath, modulesDirectory);
}


