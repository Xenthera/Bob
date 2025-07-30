#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "../headers/Lexer.h"
#include "../headers/Interpreter.h"
#include "../headers/helperFunctions/ShortHands.h"

#define VERSION "0.0.1"

class Bob
{
public:
    Lexer lexer;
    sptr(Interpreter) interpreter;

    ~Bob() = default;

public:
    void runFile(const std::string& path);

    void runPrompt();

    void error(int line, const std::string& message);


private:
    bool hadError = false;

private:
    void run(std::string source);

    void report(int line, std::string where, std::string message);
};

