#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "Lexer.h"
#include "Interpreter.h"
#include "helperFunctions/ShortHands.h"
#include "ErrorReporter.h"

#define VERSION "0.0.3"

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

