#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "../headers/Lexer.h"
#include "../headers/Interpreter.h"
#include "../headers/helperFunctions/ShortHands.h"
#include "../headers/ErrorReporter.h"

#define VERSION "0.0.2"

class Bob
{
public:
    Lexer lexer;
    sptr(Interpreter) interpreter;
    ErrorReporter errorReporter;

    ~Bob() = default;

public:
    void runFile(const std::string& path);
    void runPrompt();

private:
    void run(std::string source);
};

