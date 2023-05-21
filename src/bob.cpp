#include <iostream>
#include <fstream>
#include <string>
#include "../headers/Lexer.h"

#define VERSION "0.0.1"

using namespace std;

class Bob
{
public:
	Lexer lexer;

public:
	void runFile(string path)
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

	void runPrompt()
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
		}
	}
	

private:
	bool hadError = false;

private:
	void run(string source)
	{
		vector<Token> tokens = lexer.Tokenize(source);

		for(Token t : tokens){
			cout << "{type: " << t.type << ", value: " << t.value << "}" << endl;
		}
	}
};
int main(){

	// string TokenTypeMappings[] = {
	// 	"Identifier",
	// 	"Number",
	// 	"Equals",
	// 	"OpenParen",
	// 	"CloseParen",
	// 	"BinaryOperator",
	// 	"TestKeyword"
	// };
	
	Bob bobLang;

	//bobLang.runFile("source.bob");
	bobLang.runPrompt();


	return 0;
}
