#pragma once

#include <string>
#include <map>

enum TokenType{
    Identifier,
    Number,
    Equals,
    OpenParenthesis,
    CloseParenthesis,
    BinaryOperator,
    Test
};

const std::map<std::string, TokenType> KEYWORDS {
    {"test", Test}
};

struct Token
{
    TokenType type;
    std::string value;
};


class Lexer{
public:
    std::vector<Token> Tokenize(std::string source);

};
