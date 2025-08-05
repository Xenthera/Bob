#pragma once

#include <string>
#include <map>
#include <vector>

enum TokenType{
    OPEN_PAREN, CLOSE_PAREN, OPEN_BRACE, CLOSE_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR, PERCENT,

    BIN_OR, BIN_AND, BIN_NOT, BIN_XOR, BIN_SLEFT, BIN_SRIGHT,

    BANG, BANG_EQUAL,
    EQUAL, DOUBLE_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,
    
    // Increment/decrement operators
    PLUS_PLUS, MINUS_MINUS,

    IDENTIFIER, STRING, NUMBER, BOOL,

    AND, OR, TRUE, FALSE, IF, ELSE, FUNCTION, FOR,
    WHILE, VAR, CLASS, SUPER, THIS, NONE, RETURN,

    // Compound assignment operators
    PLUS_EQUAL, MINUS_EQUAL, STAR_EQUAL, SLASH_EQUAL, PERCENT_EQUAL,
    
    // Compound bitwise assignment operators
    BIN_AND_EQUAL, BIN_OR_EQUAL, BIN_XOR_EQUAL, BIN_SLEFT_EQUAL, BIN_SRIGHT_EQUAL,

    END_OF_FILE
};

inline std::string enum_mapping[] = {"OPEN_PAREN", "CLOSE_PAREN", "OPEN_BRACE", "CLOSE_BRACE",
                           "COMMA", "DOT", "MINUS", "PLUS", "SEMICOLON", "SLASH", "STAR", "PERCENT",

                           "BIN_OR", "BIN_AND", "BIN_NOT", "BIN_XOR", "BIN_SLEFT", "BIN_SRIGHT",

                           "BANG", "BANG_EQUAL",
                           "EQUAL", "DOUBLE_EQUAL",
                           "GREATER", "GREATER_EQUAL",
                           "LESS", "LESS_EQUAL",
                           
                           "PLUS_PLUS", "MINUS_MINUS",

                           "IDENTIFIER", "STRING", "NUMBER", "BOOL",

                           "AND", "OR", "TRUE", "FALSE", "IF", "ELSE", "FUNCTION", "FOR",
                           "WHILE", "VAR", "CLASS", "SUPER", "THIS", "NONE", "RETURN",

                           // Compound assignment operators
                           "PLUS_EQUAL", "MINUS_EQUAL", "STAR_EQUAL", "SLASH_EQUAL", "PERCENT_EQUAL",
                           
                           // Compound bitwise assignment operators
                           "BIN_AND_EQUAL", "BIN_OR_EQUAL", "BIN_XOR_EQUAL", "BIN_SLEFT_EQUAL", "BIN_SRIGHT_EQUAL",

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
        {"return", RETURN},
};

struct Token
{
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};


// Forward declaration
class ErrorReporter;

class Lexer{
public:
    Lexer() : errorReporter(nullptr) {}
    
    std::vector<Token> Tokenize(std::string source);
    
    // Set error reporter for enhanced error reporting
    void setErrorReporter(ErrorReporter* reporter) {
        errorReporter = reporter;
    }
    
private:
    int line;
    int column;
    std::vector<char> src;
    ErrorReporter* errorReporter;
    
private:
    bool matchOn(char expected);

    char peekNext();

    void advance(int by = 1);

    std::string parseEscapeCharacters(const std::string &input);
};
