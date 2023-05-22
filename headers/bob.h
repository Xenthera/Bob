#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "../headers/Lexer.h"

#define VERSION "0.0.1"

class Bob
{
public:
    Lexer lexer;

public:
    void runFile(std::string path);

    void runPrompt();

    void error(int line, std::string message);


private:
    bool hadError = false;

private:
    void run(std::string source);

    void report(int line, std::string where, std::string message);
};

