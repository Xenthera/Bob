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

inline std::string enum_mapping[] = {"OPEN_PAREN", "CLOSE_PAREN", "OPEN_BRACE", "CLOSE_BRACE",
                           "COMMA", "DOT", "MINUS", "PLUS", "SEMICOLON", "SLASH", "STAR",

                           "BINARY_OP",

                           "BANG", "BANG_EQUAL",
                           "EQUAL", "DOUBLE_EQUAL",
                           "GREATER", "GREATER_EQUAL",
                           "LESS", "LESS_EQUAL",

                           "IDENTIFIER", "STRING", "NUMBER",

                           "AND", "OR", "TRUE", "FALSE", "IF", "ELSE", "FUNCTION", "FOR",
                           "WHILE", "VAR", "CLASS", "SUPER", "THIS", "NONE", "RETURN",

                           "END_OF_FILE"};

const std::map<std::string, TokenType> KEYWORDS {
        {"and", AND},
        {"or", OR},
        {"true", TRUE},
        {"false", FALSE},
        {"if", IF},
        {"else", ELSE},
        {"func", FUNCTION},
        {"for", FOR},
        {"while", WHILE},
        {"var", VAR},
        {"class", CLASS},
        {"super", SUPER},
        {"this", THIS},
        {"none", NONE},
        {"return", RETURN}
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
    std::vector<char> src;
private:
    bool matchOn(char expected);

    char peekNext();

    void advance(int by = 1);
};
