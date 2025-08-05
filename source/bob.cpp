#include <utility>

#include "../headers/bob.h"
#include "../headers/Parser.h"
using namespace std;

void Bob::runFile(const string& path)
{
    this->interpreter = msptr(Interpreter)(false);
    ifstream file = ifstream(path);

    string source;

    if(file.is_open()){
        source = string(istreambuf_iterator<char>(file), istreambuf_iterator<char>());
    }
    else
    {
        cout << "File not found" << endl;
        return;
    }

    // Load source code into error reporter for context
    errorReporter.loadSource(source, path);
    
    // Connect error reporter to interpreter
    interpreter->setErrorReporter(&errorReporter);
    
    this->run(source);
}

void Bob::runPrompt()
{
    this->interpreter = msptr(Interpreter)(true);

    cout << "Bob v" << VERSION << ", 2025" << endl;
    for(;;)
    {
        string line;
        cout << "\033[0;36m" << "-> " << "\033[0;37m";
        std::getline(std::cin, line);

        if(std::cin.eof())
        {
            break;
        }

        // Load source code into error reporter for context
        errorReporter.loadSource(line, "REPL");
        
        // Connect error reporter to interpreter
        interpreter->setErrorReporter(&errorReporter);
        
        this->run(line);
    }
}

void Bob::run(string source)
{
    try {
        // Connect error reporter to lexer
        lexer.setErrorReporter(&errorReporter);
        
        vector<Token> tokens = lexer.Tokenize(std::move(source));
        Parser p(tokens);
        
        // Connect error reporter to parser
        p.setErrorReporter(&errorReporter);
        
        vector<sptr(Stmt)> statements = p.parse();
        interpreter->interpret(statements);
    }
    catch(std::exception &e)
    {
        // Only suppress errors that have already been reported by the error reporter
        if (errorReporter.hasReportedError()) {
            return;
        }
        
        // For errors that weren't reported (like parser errors, undefined variables, etc.)
        // print them normally
        std::cout << "Error: " << e.what() << std::endl;
        return;
    }
    catch(...)
    {
        // Unknown error - report it since it wasn't handled by the interpreter
        errorReporter.reportError(0, 0, "Unknown Error", "An unknown error occurred");
        return;
    }
}


