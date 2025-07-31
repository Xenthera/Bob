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

    this->run(source);
}

void Bob::runPrompt()
{
    this->interpreter = msptr(Interpreter)(true);

    cout << "Bob v" << VERSION << ", 2023" << endl;
    for(;;)
    {
        string line;
        cout << "\033[0;36m" << "-> " << "\033[0;37m";
        std::getline(std::cin, line);

        if(std::cin.eof())
        {
            break;
        }

        this->run(line);
        hadError = false;
    }
}

void Bob::error(int line, const string& message)
{

}

void Bob::run(string source)
{
    try {
        vector<Token> tokens = lexer.Tokenize(std::move(source));
        Parser p(tokens);
        vector<sptr(Stmt)> statements = p.parse();
        interpreter->interpret(statements);
    }
    catch(std::exception &e)
    {
        cout << "ERROR OCCURRED: " << e.what() << endl;
        return;
    }
    catch(...)
    {
        cout << "UNKNOWN ERROR OCCURRED" << endl;
        return;
    }



}

void Bob::report(int line, string where, string message)
{
    hadError = true;
}


