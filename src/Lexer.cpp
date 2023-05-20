#include "../headers/Lexer.h"
#include <iostream>
#include <cctype>

std::vector<Token> Lexer::Tokenize(std::string source){
    std::vector<Token> tokens;
    std::vector<char> src{source.begin(), source.end()};

    while(src.size() > 0)
    {
        char t = src[0];
        if(t == '(')
        {
            tokens.push_back(Token{OpenParenthesis, std::string(1, t)}); //brace initialization in case you forget
            src.erase(src.begin());
        }
        else if(t == ')')
        {
            tokens.push_back(Token{CloseParenthesis, std::string(1, t)});
            src.erase(src.begin());
        }
        else if(t == '+' || t == '-' || t == '*' || t == '/')
        {
            tokens.push_back(Token{BinaryOperator, std::string(1, t)});
            src.erase(src.begin());
        }
        else if(t == '=')
        {
            tokens.push_back(Token{Equals, std::string(1, t)});
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

                tokens.push_back(Token{Number, num});
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
                    tokens.push_back(Token{Identifier, ident});
                }

            }
            else if(t == ' ' || t == '\t' || t == '\n')
            {
                    src.erase(src.begin());
            }
            else
            {
                throw std::runtime_error("Unknown Token: '" + std::string(1, t) + "'");
            }
        }
        
    }

    return tokens;
}
