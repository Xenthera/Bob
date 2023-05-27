#include "../headers/bob.h"
#include "../headers/Parser.h"
#include "../headers/ASTPrinter.h"
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
    try {
        vector<Token> tokens = lexer.Tokenize(source);
        Parser p(tokens);
        shared_ptr<Expr<shared_ptr<Object>>> expr = p.parse();


        ASTPrinter printer;

        cout << dynamic_pointer_cast<String>(printer.print(expr.get()))->value << endl;


        for(Token t : tokens){
            //cout << "{type: " << t.type << ", value: " << t.lexeme << "}" << endl;
        }


    }
    catch(std::exception &e)
    {
        cout << "ERROR OCCURRED: " << e.what() << endl;
        return;
    }

    cout << "Current object count: " << Object::count << endl;

}

void Bob::report(int line, string where, string message)
{
    hadError = true;
}


