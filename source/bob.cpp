#include "../headers/bob.h"
using namespace std;

void Bob::runFile(string path)
{
    ifstream file = ifstream(path);

    string source = "";

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
    cout << "Bob v" << VERSION << ", 2023" << endl;
    for(;;)
    {
        string line;
        cout << "-> ";
        std::getline(std::cin, line);

        if(std::cin.eof())
        {
            break;
        }

        this->run(line);
        hadError = false;
    }
}

void Bob::error(int line, string message)
{

}

void Bob::run(string source)
{
    vector<Token> tokens = lexer.Tokenize(source);

    for(Token t : tokens){
        cout << "{type: " << t.type << ", value: " << t.lexeme << "}" << endl;
    }
}

void Bob::report(int line, string where, string message)
{
    hadError = true;
}


