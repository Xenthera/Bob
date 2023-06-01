#include "../headers/Lexer.h"
#include "../headers/helperFunctions/HelperFunctions.h"
#include <cctype>
#include <stdexcept>

using namespace std;

std::vector<Token> Lexer::Tokenize(std::string source){
    std::vector<Token> tokens;
    src = std::vector<char>{source.begin(), source.end()};
    line = 0;

    while(!src.empty())
    {
        char t = src[0];
        if(t == '(')
        {
            tokens.push_back(Token{OPEN_PAREN, std::string(1, t), line}); //brace initialization in case you forget
            advance();
        }
        else if(t == ')')
        {
            tokens.push_back(Token{CLOSE_PAREN, std::string(1, t), line});
            advance();
        }
        else if(t == '{')
        {
            tokens.push_back(Token{OPEN_BRACE, std::string(1, t), line});
            advance();
        }
        else if(t == '}')
        {
            tokens.push_back(Token{CLOSE_BRACE, std::string(1, t), line});
            advance();
        }
        else if(t == ',')
        {
            tokens.push_back(Token{COMMA, std::string(1, t), line});
            advance();
        }
        else if(t == '.')
        {
            tokens.push_back(Token{DOT, std::string(1, t), line});
            advance();
        }
        else if(t == ';')
        {
            tokens.push_back(Token{SEMICOLON, std::string(1, t), line});
            advance();
        }
        else if(t == '+')
        {
            tokens.push_back(Token{PLUS, std::string(1, t), line});
            advance();
        }
        else if(t == '-')
        {
            tokens.push_back(Token{MINUS, std::string(1, t), line});
            advance();
        }
        else if(t == '*')
        {
            tokens.push_back(Token{STAR, std::string(1, t), line});
            advance();
        }
        else if(t == '%')
        {
            tokens.push_back(Token{PERCENT, std::string(1, t), line});
            advance();
        }
        else if(t == '~')
        {
            tokens.push_back(Token{BIN_NOT, std::string(1, t), line});
            advance();
        }
        else if(t == '=')
        {
            std::string token = std::string(1, t);
            advance();
            bool match = matchOn('=');
            token += match ? "=" : "";
            tokens.push_back(Token{match ? DOUBLE_EQUAL : EQUAL, token, line});
        }
        else if(t == '!')
        {
            std::string token = std::string(1, t);
            advance();
            bool match = matchOn('=');
            token += match ? "=" : "";
            tokens.push_back(Token{match ? BANG_EQUAL : BANG, token, line});
        }
        else if(t == '<')
        {
            std::string token = std::string(1, t);
            advance();
            if(matchOn('='))
            {
                tokens.push_back(Token{LESS_EQUAL, "<=", line});
            }
            else if(matchOn('<'))
            {
                tokens.push_back(Token{BIN_SLEFT, "<<", line});
            }
            else
            {
                tokens.push_back(Token{LESS, token, line});
            }

        }
        else if(t == '>')
        {
            std::string token = std::string(1, t);
            advance();
            bool match = matchOn('=');
            token += match ? "=" : "";
            tokens.push_back(Token{match ? GREATER_EQUAL : GREATER, token, line});
        }
        else if(t == '&')
        {
            std::string token = std::string(1, t);
            advance();
            bool match = matchOn('&');
            token += match ? "&" : "";
            if(match) tokens.push_back(Token{AND, token, line});
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
                tokens.push_back(Token{SLASH, std::string(1, t), line});
            }
        }
        else if(t == '"')
        {
            std::string str;
            advance();
            while(!src.empty() && src[0] != '"')
            {

                if(src[0] == '\n') line++;
                str += src[0];
                advance();
            }
            if(src.empty())
            {
                throw std::runtime_error("LEXER: Unterminated string at line: " + std::to_string(this->line));
            }
            else if(src[0] == '"')
            {

                advance();
                tokens.push_back(Token{STRING, str, line});
            }
            

        }
        else if(t == '\n')
        {
            line++;
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

                tokens.push_back(Token{NUMBER, num, line});
            }
            else if(std::isalpha(t))
            {
                std::string ident;
                while(!src.empty() && std::isalpha(src[0]))
                {
                    ident += src[0];
                    advance();
                }

                if(KEYWORDS.find(ident) != KEYWORDS.end()) //identifier is a keyword
                {
                    tokens.push_back(Token{KEYWORDS.at(ident), ident, line});
                }
                else
                {
                    tokens.push_back(Token{IDENTIFIER, ident, line});
                }

            }
            else if(t == ' ' || t == '\t')
            {
                    advance();
            }
            else
            {

                throw std::runtime_error("LEXER: Unknown Token: '" + std::string(1, t) + "'");
            }

           
        }
        
    }
    tokens.push_back({END_OF_FILE, "eof", line});
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
        src.erase(src.begin());
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


