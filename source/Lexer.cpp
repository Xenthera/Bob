#include "../headers/Lexer.h"
#include "../headers/ErrorReporter.h"
#include "../headers/helperFunctions/HelperFunctions.h"
#include <cctype>
#include <stdexcept>

using namespace std;

std::vector<Token> Lexer::Tokenize(std::string source){
    std::vector<Token> tokens;
    src = std::vector<char>{source.begin(), source.end()};
    line = 1;
    column = 1;

    while(!src.empty())
    {
        char t = src[0];
        if(t == '(')
        {
            tokens.push_back(Token{OPEN_PAREN, std::string(1, t), line, column}); //brace initialization in case you forget
            advance();
        }
        else if(t == ')')
        {
            tokens.push_back(Token{CLOSE_PAREN, std::string(1, t), line, column});
            advance();
        }
        else if(t == '{')
        {
            tokens.push_back(Token{OPEN_BRACE, std::string(1, t), line, column});
            advance();
        }
        else if(t == '}')
        {
            tokens.push_back(Token{CLOSE_BRACE, std::string(1, t), line, column});
            advance();
        }
        else if(t == '[')
        {
            tokens.push_back(Token{OPEN_BRACKET, std::string(1, t), line, column});
            advance();
        }
        else if(t == ']')
        {
            tokens.push_back(Token{CLOSE_BRACKET, std::string(1, t), line, column});
            advance();
        }
        else if(t == ',')
        {
            tokens.push_back(Token{COMMA, std::string(1, t), line, column});
            advance();
        }
        else if(t == '.')
        {
            tokens.push_back(Token{DOT, std::string(1, t), line, column});
            advance();
        }
        else if(t == ';')
        {
            tokens.push_back(Token{SEMICOLON, std::string(1, t), line, column});
            advance();
        }
        else if(t == '+')
        {
            std::string token = std::string(1, t);
            advance();
            bool match = matchOn('+');
            if(match) {
                token += '+';
                tokens.push_back(Token{PLUS_PLUS, token, line, column - 1});
            } else {
                match = matchOn('=');
                if(match) {
                    token += '=';
                    tokens.push_back(Token{PLUS_EQUAL, token, line, column - 1});
                } else {
                    tokens.push_back(Token{PLUS, token, line, column - 1});
                }
            }
        }
        else if(t == '-')
        {
            std::string token = std::string(1, t);
            advance();
            bool match = matchOn('-');
            if(match) {
                token += '-';
                tokens.push_back(Token{MINUS_MINUS, token, line, column - 1});
            } else {
                match = matchOn('=');
                if(match) {
                    token += '=';
                    tokens.push_back(Token{MINUS_EQUAL, token, line, column - 1});
                } else {
                    tokens.push_back(Token{MINUS, token, line, column - 1});
                }
            }
        }
        else if(t == '*')
        {
            std::string token = std::string(1, t);
            advance();
            bool match = matchOn('=');
            if(match) {
                token += '=';
                tokens.push_back(Token{STAR_EQUAL, token, line, column - 1});
            } else {
                tokens.push_back(Token{STAR, token, line, column - 1});
            }
        }
        else if(t == '%')
        {
            std::string token = std::string(1, t);
            advance();
            bool match = matchOn('=');
            if(match) {
                token += '=';
                tokens.push_back(Token{PERCENT_EQUAL, token, line, column - 1});
            } else {
                tokens.push_back(Token{PERCENT, token, line, column - 1});
            }
        }
        else if(t == '~')
        {
            tokens.push_back(Token{BIN_NOT, std::string(1, t), line, column - 1});
            advance();
        }
        else if(t == '?')
        {
            tokens.push_back(Token{QUESTION, std::string(1, t), line, column});
            advance();
        }
        else if(t == ':')
        {
            tokens.push_back(Token{COLON, std::string(1, t), line, column});
            advance();
        }
        else if(t == '=')
        {
            std::string token = std::string(1, t);
            advance();
            bool match = matchOn('=');
            token += match ? "=" : "";
            tokens.push_back(Token{match ? DOUBLE_EQUAL : EQUAL, token, line, column - static_cast<int>(token.length())});
        }
        else if(t == '!')
        {
            std::string token = std::string(1, t);
            advance();
            bool match = matchOn('=');
            token += match ? "=" : "";
            tokens.push_back(Token{match ? BANG_EQUAL : BANG, token, line, column - static_cast<int>(token.length())});
        }
        else if(t == '<')
        {
            std::string token = std::string(1, t);
            advance();
            if(matchOn('='))
            {
                tokens.push_back(Token{LESS_EQUAL, "<=", line, column - 1});
            }
            else if(matchOn('<'))
            {
                bool equalMatch = matchOn('=');
                if(equalMatch) {
                    tokens.push_back(Token{BIN_SLEFT_EQUAL, "<<=", line, column - 1});
                } else {
                    tokens.push_back(Token{BIN_SLEFT, "<<", line, column - 1});
                }
            }
            else
            {
                tokens.push_back(Token{LESS, token, line, column - static_cast<int>(token.length())});
            }

        }
        else if(t == '>')
        {
            std::string token = std::string(1, t);
            advance();
            if(matchOn('='))
            {
                tokens.push_back(Token{GREATER_EQUAL, ">=", line, column - 1});
            }
            else if(matchOn('>'))
            {
                bool equalMatch = matchOn('=');
                if(equalMatch) {
                    tokens.push_back(Token{BIN_SRIGHT_EQUAL, ">>=", line, column - 1});
                } else {
                    tokens.push_back(Token{BIN_SRIGHT, ">>", line, column - 1});
                }
            }
            else
            {
                tokens.push_back(Token{GREATER, token, line, column - static_cast<int>(token.length())});
            }
        }
        else if(t == '&')
        {
            std::string token = std::string(1, t);
            advance();
            bool match = matchOn('&');
            if(match) {
                token += '&';
                tokens.push_back(Token{AND, token, line, column - 1});
            } else {
                bool equalMatch = matchOn('=');
                if(equalMatch) {
                    token += '=';
                    tokens.push_back(Token{BIN_AND_EQUAL, token, line, column - 1});
                } else {
                    tokens.push_back(Token{BIN_AND, token, line, column - 1});
                }
            }
        }
        else if(t == '|')
        {
            std::string token = std::string(1, t);
            advance();
            bool match = matchOn('|');
            if(match) {
                token += '|';
                tokens.push_back(Token{OR, token, line, column - 1});
            } else {
                bool equalMatch = matchOn('=');
                if(equalMatch) {
                    token += '=';
                    tokens.push_back(Token{BIN_OR_EQUAL, token, line, column - 1});
                } else {
                    tokens.push_back(Token{BIN_OR, token, line, column - 1});
                }
            }
        }
        else if(t == '^')
        {
            std::string token = std::string(1, t);
            advance();
            bool match = matchOn('=');
            if(match) {
                tokens.push_back(Token{BIN_XOR_EQUAL, "^=", line, column - 1});
            } else {
                tokens.push_back(Token{BIN_XOR, "^", line, column - 1});
            }
        }
        else if(t == '/')
        {
            std::string token = std::string(1, t);
            advance();
            bool match = matchOn('/');
            if(match)
            {
                while(!src.empty() && src[0] != '\n')
                {
                    advance();
                }
            }
            else
            {
                bool starMatch = matchOn('*');
                if(starMatch)
                {
                    // Multi-line comment /* ... */
                    while(!src.empty())
                    {
                        if(src[0] == '*' && !src.empty() && src.size() > 1 && src[1] == '/')
                        {
                            advance(2);  // Skip */
                            break;
                        }
                        advance();
                    }
                }
                else
                {
                    bool equalMatch = matchOn('=');
                    if(equalMatch) {
                        tokens.push_back(Token{SLASH_EQUAL, "/=", line, column - 1});
                    } else {
                        tokens.push_back(Token{SLASH, "/", line, column - 1});
                    }
                }
            }
        }
        else if(t == '"')
        {
            bool last_was_escape = false;
            std::string str;
            int startColumn = column;
            advance();

            while(!src.empty())
            {
                std::string next_c =  std::string(1, src[0]);
                if(next_c == "\"") break;
                if(next_c == "\\")
                {
                    advance();
                    next_c = "\\" + std::string(1, src[0]);
                }

                str += next_c;
                advance();
            }

            if(src.empty())
            {
                throw std::runtime_error("LEXER: Unterminated string at line: " + std::to_string(this->line));
            }
            else if(src[0] == '"')
            {

                advance();


                std::string escaped_str = parseEscapeCharacters(str);
                tokens.push_back(Token{STRING, escaped_str, line, startColumn});
            }
            

        }
        else if(t == '\n')
        {
            advance();
        }
        else
        {
            bool isNotation = false;
            bool notationInvalidated = false;
            char notationChar;
            //Multi char tokens
            if(std::isdigit(t))
            {
                std::string num;
                int startColumn = column;

                if(src[0] != '0') notationInvalidated = true;

                while(!src.empty())
                {
                    if(std::isdigit(src[0]))
                    {
                        if(src[0] == '0' && !notationInvalidated)
                        {
                            if(peekNext() == 'b' || peekNext() == 'x') {
                                num += "0";
                                num += peekNext();
                                notationChar = peekNext();
                                advance(2);
                                isNotation = true;
                                break;
                            }
                        }

                        num += src[0];
                        advance();

                    }
                    else
                    {
                        break;
                    }

                }
                if(!isNotation) {
                    if (!src.empty() && src[0] == '.') {
                        advance();
                        if (!src.empty() && std::isdigit(src[0])) {
                            num += '.';
                            while (!src.empty() && std::isdigit(src[0])) {
                                num += src[0];
                                advance();
                            }
                        } else {
                            throw std::runtime_error("LEXER: malformed number at: " + std::to_string(this->line));
                        }

                    }
                }
                else
                {
                    if(!src.empty() && (src[0]))
                    {
                        if(notationChar == 'b') {
                            while (!src.empty() && (src[0] == '0' || src[0] == '1')) {
                                num += src[0];
                                advance();
                            }
                        }
                        else if(notationChar == 'x')
                        {
                            while (!src.empty() && std::isxdigit(src[0])) {

                                num += src[0];
                                advance();
                            }
                        }
                    }
                    else
                    {
                        throw std::runtime_error("LEXER: malformed notation at: " + std::to_string(this->line));
                    }
                }

                tokens.push_back(Token{NUMBER, num, line, startColumn});
            }
            else if(std::isalpha(t))
            {
                std::string ident;
                int startColumn = column;
                while(!src.empty() && (std::isalpha(src[0]) || std::isdigit(src[0]) || src[0] == '_'))
                {
                    ident += src[0];
                    advance();
                }

                if(KEYWORDS.find(ident) != KEYWORDS.end()) //identifier is a keyword
                {
                    tokens.push_back(Token{KEYWORDS.at(ident), ident, line, startColumn});
                }
                else
                {
                    tokens.push_back(Token{IDENTIFIER, ident, line, startColumn});
                }

            }
            else if(t == ' ' || t == '\t')
            {
                    advance();
            }
            else
            {
                if (errorReporter) {
                    errorReporter->reportError(line, column, "Lexer Error",
                        "Unknown token '" + std::string(1, t) + "'", "");
                }
                throw std::runtime_error("LEXER: Unknown Token: '" + std::string(1, t) + "'");
            }

           
        }
        
    }
    tokens.push_back({END_OF_FILE, "eof", line, column});
    return tokens;
}

bool Lexer::matchOn(char expected)
{
    if(src.empty()) return false;
    if(src[0] != expected) return false;
    advance();
    return true;
}

void Lexer::advance(int by)
{
    for (int i = 0; i < by; ++i) {
        if (!src.empty()) {
            char c = src[0];
            src.erase(src.begin());
            
            // Update column and line counters
            if (c == '\n') {
                line++;
                column = 1;
            } else if (c == '\r') {
                // Handle \r\n sequence
                if (!src.empty() && src[0] == '\n') {
                    src.erase(src.begin());
                    line++;
                    column = 1;
                } else {
                    column++;
                }
            } else {
                column++;
            }
        }
    }
}

char Lexer::peekNext()
{
    if(src.size() > 1)
    {
        return src[1];
    }

    return '\0';
}

std::string Lexer::parseEscapeCharacters(const std::string& input) {
    std::string output;
    bool escapeMode = false;
    size_t i = 0;

    while (i < input.length()) {
        char c = input[i];
        
        if (escapeMode) {
            switch (c) {
                case 'n':
                    output += '\n';
                    break;
                case 't':
                    output += '\t';
                    break;
                case '"':
                    output += '\"';
                    break;
                case '\\':
                    output += '\\';
                    break;
                case '0':
                    output += '\0';
                    break;
                case 'r':
                    output += '\r';
                    break;
                case 'a':
                    output += '\a';
                    break;
                case 'b':
                    output += '\b';
                    break;
                case 'f':
                    output += '\f';
                    break;
                case 'v':
                    output += '\v';
                    break;
                case 'e':
                    // ANSI escape sequence
                    output += '\033';
                    break;
                default:
                    throw runtime_error("Invalid escape character: " + std::string(1, c));
            }
            escapeMode = false;
        } else if (c == '\\') {
            escapeMode = true;
        } else {
            output += c;
        }
        i++;
    }

    return output;
}


