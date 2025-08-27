#pragma once

#include <string>
#include <map>
#include <vector>

enum TokenType{
    OPEN_PAREN, CLOSE_PAREN, OPEN_BRACE, CLOSE_BRACE,
    OPEN_BRACKET, CLOSE_BRACKET,  // Array brackets
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR, PERCENT,

    BIN_OR, BIN_AND, BIN_NOT, BIN_XOR, BIN_SLEFT, BIN_SRIGHT,

    BANG, BANG_EQUAL,
    EQUAL, DOUBLE_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,
    
    // Ternary operator
    QUESTION, COLON,
    
    // Increment/decrement operators
    PLUS_PLUS, MINUS_MINUS,

    IDENTIFIER, STRING, NUMBER, INTEGER, BIGINT, KW_BOOL,

    AND, OR, TRUE, FALSE, IF, ELSE, FUNCTION, FOR, FOREACH,
    WHILE, DO, VAR, CLASS, EXTENDS, EXTENSION, SUPER, THIS, NONE, RETURN, BREAK, CONTINUE,
    IMPORT, FROM, AS,
    TRY, CATCH, FINALLY, THROW,

    // Compound assignment operators
    PLUS_EQUAL, MINUS_EQUAL, STAR_EQUAL, SLASH_EQUAL, PERCENT_EQUAL,
    
    // Compound bitwise assignment operators
    BIN_AND_EQUAL, BIN_OR_EQUAL, BIN_XOR_EQUAL, BIN_SLEFT_EQUAL, BIN_SRIGHT_EQUAL,

    END_OF_FILE
};

inline std::string enum_mapping[] = {"OPEN_PAREN", "CLOSE_PAREN", "OPEN_BRACE", "CLOSE_BRACE",
                           "OPEN_BRACKET", "CLOSE_BRACKET",  // Array brackets
                           "COMMA", "DOT", "MINUS", "PLUS", "SEMICOLON", "SLASH", "STAR", "PERCENT",

                           "BIN_OR", "BIN_AND", "BIN_NOT", "BIN_XOR", "BIN_SLEFT", "BIN_SRIGHT",

                           "BANG", "BANG_EQUAL",
                           "EQUAL", "DOUBLE_EQUAL",
                           "GREATER", "GREATER_EQUAL",
                           "LESS", "LESS_EQUAL",
                           
                           "QUESTION", "COLON",
                           
                           "PLUS_PLUS", "MINUS_MINUS",

                           "IDENTIFIER", "STRING", "NUMBER", "INTEGER", "BIGINT", "KW_BOOL",

                           "AND", "OR", "TRUE", "FALSE", "IF", "ELSE", "FUNCTION", "FOR", "FOREACH",
                           "WHILE", "DO", "VAR", "CLASS", "EXTENDS", "EXTENSION", "SUPER", "THIS", "NONE", "RETURN", "BREAK", "CONTINUE",
                           "IMPORT", "FROM", "AS",
                           "TRY", "CATCH", "FINALLY", "THROW",

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
        {"foreach", FOREACH},
        {"while", WHILE},
        {"do", DO},
        {"var", VAR},
        {"class", CLASS},
        {"extends", EXTENDS},
        {"extension", EXTENSION},
        {"super", SUPER},
        {"this", THIS},
        {"none", NONE},
        {"return", RETURN},
        {"break", BREAK},
        {"continue", CONTINUE},
        {"import", IMPORT},
        {"from", FROM},
        {"as", AS},
        {"try", TRY},
        {"catch", CATCH},
        {"finally", FINALLY},
        {"throw", THROW},
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
