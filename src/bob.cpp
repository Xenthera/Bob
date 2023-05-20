#include <iostream>
#include <fstream>
#include <string>
#include "../headers/Lexer.h"

using namespace std;

int main(){

	string TokenTypeMappings[] = {
		"Identifier",
		"Number",
		"Equals",
		"OpenParen",
		"CloseParen",
		"BinaryOperator",
		"TestKeyword"
	};
	Lexer l;

	string path = "source.bob";
	ifstream file = ifstream(path); 
	
	string source = "";

	if(file.is_open()){
		source = string(istreambuf_iterator<char>(file), istreambuf_iterator<char>());
	}
	else
	{
		cout << "File not found" << endl;
	}


	vector<Token> tokens = l.Tokenize(source);
	for(Token t : tokens){
		cout << "Type: " << TokenTypeMappings[t.type] << ", Value: " + t.value << endl;
	}
	return 0;
}
