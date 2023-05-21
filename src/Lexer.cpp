#include "../headers/Lexer.h"
#include <iostream>
#include <cctype>


using namespace std;

std::vector<Token> Lexer::Tokenize(std::string source){
    std::vector<Token> tokens;
    std::vector<char> src{source.begin(), source.end()};

    while(src.size() > 0)
    {
        char t = src[0];
        if(t == '(')
        {
            tokens.push_back(Token{OPEN_PAREN, std::string(1, t)}); //brace initialization in case you forget
            src.erase(src.begin());
        }
        else if(t == ')')
        {
            tokens.push_back(Token{CLOSE_PAREN, std::string(1, t)});
            src.erase(src.begin());
        }
        else if(t == '{')
        {
            tokens.push_back(Token{OPEN_BRACE, std::string(1, t)});
            src.erase(src.begin());
        }
        else if(t == '}')
        {
            tokens.push_back(Token{CLOSE_BRACE, std::string(1, t)});
            src.erase(src.begin());
        }
        else if(t == ',')
        {
            tokens.push_back(Token{COMMA, std::string(1, t)});
            src.erase(src.begin());
        }
        else if(t == '.')
        {
            tokens.push_back(Token{DOT, std::string(1, t)});
            src.erase(src.begin());
        }
        else if(t == ';')
        {
            tokens.push_back(Token{SEMICOLON, std::string(1, t)});
            src.erase(src.begin());
        }
        else if(t == '+')
        {
            tokens.push_back(Token{PLUS, std::string(1, t)});
            src.erase(src.begin());
        }
        else if(t == '-')
        {
            tokens.push_back(Token{MINUS, std::string(1, t)});
            src.erase(src.begin());
        }
         else if(t == '*')
        {
            tokens.push_back(Token{STAR, std::string(1, t)});
            src.erase(src.begin());
        }
        else if(t == '=')
        {
            std::string token = std::string(1, t);
            src.erase(src.begin());
            bool match = matchOn('=', src);
            token += match ? "=" : "";
            tokens.push_back(Token{match ? DOUBLE_EQUAL : EQUAL, token});
        }
        else if(t == '!')
        {
            std::string token = std::string(1, t);
            src.erase(src.begin());
            bool match = matchOn('=', src);
            token += match ? "=" : "";
            tokens.push_back(Token{match ? BANG_EQUAL : BANG, token});
        }
        else if(t == '<')
        {
            std::string token = std::string(1, t);
            src.erase(src.begin());
            bool match = matchOn('=', src);
            token += match ? "=" : "";
            tokens.push_back(Token{match ? LESS_EQUAL : LESS, token});
        }
        else if(t == '>')
        {
            std::string token = std::string(1, t);
            src.erase(src.begin());
            bool match = matchOn('=', src);
            token += match ? "=" : "";
            tokens.push_back(Token{match ? GREATER_EQUAL : GREATER, token});
        }
        else if(t == '/')
        {
            std::string token = std::string(1, t);
            src.erase(src.begin());
            bool match = matchOn('/', src);
            if(match)
            {
                while(src.size() > 0 && src[0] != '\n')
                {
                    src.erase(src.begin());
                }
            }
            else
            {
                tokens.push_back(Token{SLASH, std::string(1, t)});
            }
        }
        else if(t == '"')
        {
            std::string str = std::string(1, src[0]);
            src.erase(src.begin());
            while(src.size() > 0 && src[0] != '"')
            {
                if(src[0] == '\n') line++;
                str += src[0];
                src.erase(src.begin());
            }
            if(src.size() == 0)
            {
                throw std::runtime_error("Unterminated string at line: " + std::to_string(this->line));
            }
            else if(src[0] == '"')
            {
                str += '"';
                src.erase(src.begin());
                tokens.push_back(Token{STRING, str});
            }
            

        }
        else if(t == '\n')
        {
            line++;
            src.erase(src.begin());
        }
        else
        {
            //Multi char tokens
            if(std::isdigit(t))
            {
                std::string num = "";
                while(src.size() > 0 && std::isdigit(src[0]))
                {
                    num += src[0];
                    src.erase(src.begin());
                }

                if(src.size() > 0 && src[0] == '.')
                {
                    src.erase(src.begin());
                    if(src.size() > 0 && std::isdigit(src[0]))
                    {
                        num += '.';
                        while(src.size() > 0 && std::isdigit(src[0]))
                        {
                            num += src[0];
                            src.erase(src.begin());
                        }
                    }
                    else
                    {
                        throw std::runtime_error("malformed number at: " + std::to_string(this->line));
                    }
                    
                }

                tokens.push_back(Token{NUMBER, num});
            }
            else if(std::isalpha(t))
            {
                std::string ident = "";
                while(src.size() > 0 && std::isalpha(src[0]))
                {
                    ident += src[0];
                    src.erase(src.begin());
                }

                if(KEYWORDS.find(ident) != KEYWORDS.end()) //identifier is a keyword
                {
                    tokens.push_back(Token{KEYWORDS.at(ident), ident});
                }
                else
                {
                    tokens.push_back(Token{IDENTIFIER, ident});
                }

            }
            else if(t == ' ' || t == '\t' || t == '\n')
            {
                    src.erase(src.begin()); //ignore t
            }
            else
            {

                throw std::runtime_error("Unknown Token: '" + std::string(1, t) + "'");
            }

           
        }
        
    }

    return tokens;
}

bool Lexer::matchOn(char expected, std::vector<char> &src)
{
    if(src.size() == 0) return false;
    if(src[0] != expected) return false;
    src.erase(src.begin());
    return true;
}
