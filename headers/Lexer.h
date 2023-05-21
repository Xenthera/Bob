#pragma once

#include <string>
#include <map>
#include <vector>

enum TokenType{
    OPEN_PAREN, CLOSE_PAREN, OPEN_BRACE, CLOSE_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

    BINARY_OP,

    BANG, BANG_EQUAL,
    EQUAL, DOUBLE_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    IDENTIFIER, STRING, NUMBER,

    AND, OR, TRUE, FALSE, IF, ELSE, FUNCTION, FOR,
    WHILE, VAR, CLASS, SUPER, THIS, NONE, RETURN,

    END_OF_FILE
};

const std::map<std::string, TokenType> KEYWORDS {
    
};

struct Token
{
    TokenType type;
    std::string lexeme;
    //TODO Object literal;
    int line;
};


class Lexer{
public:
    std::vector<Token> Tokenize(std::string source);
private:
    int line;
private:
    bool matchOn(char expected, std::vector<char>& src);
};
