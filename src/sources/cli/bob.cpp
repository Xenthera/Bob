#include <utility>

#include "bob.h"
#include "Parser.h"

void Bob::runFile(const std::string& path)
{
    this->interpreter = msptr(Interpreter)(false);
    std::ifstream file = std::ifstream(path);

    std::string source;

    if(file.is_open()){
        source = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
    else
    {
        std::cout << "File not found\n";
        return;
    }

    // Load source code into error reporter for context
    errorReporter.loadSource(source, path);
    
    interpreter->setErrorReporter(&errorReporter);
    interpreter->addStdLibFunctions();
    
    this->run(source);
}

void Bob::runPrompt()
{
    this->interpreter = msptr(Interpreter)(true);

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
        
        // Load source code into error reporter for context
        errorReporter.loadSource(line, "REPL");
        
        // Connect error reporter to interpreter
        interpreter->setErrorReporter(&errorReporter);
        interpreter->addStdLibFunctions();
        
        this->run(line);
    }
}

void Bob::run(std::string source)
{
    try {
        // Connect error reporter to lexer
        lexer.setErrorReporter(&errorReporter);
        
        std::vector<Token> tokens = lexer.Tokenize(std::move(source));
        Parser p(tokens);
        
        // Connect error reporter to parser
        p.setErrorReporter(&errorReporter);
        
        std::vector<sptr(Stmt)> statements = p.parse();
        interpreter->interpret(statements);
    }
    catch(std::exception &e)
    {
        // Only suppress errors that have already been reported inline/top-level
        if (errorReporter.hasReportedError() || (interpreter && (interpreter->hasReportedError() || interpreter->hasInlineErrorReported()))) {
            if (interpreter) interpreter->clearInlineErrorReported();
            return;
        }
        
        // For errors that weren't reported (like parser errors, undefined variables, etc.)
        // print them normally
        std::cout << "Error: " << e.what() << '\n';
        return;
    }
    catch(const std::exception& e)
    {
        // Unknown error - report it since it wasn't handled by the interpreter
        errorReporter.reportError(0, 0, "Unknown Error", "An unknown error occurred: " + std::string(e.what()));
        return;
    }
}


