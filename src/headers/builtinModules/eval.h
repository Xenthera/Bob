#pragma once

class Interpreter;

// Register the builtin 'eval' module providing eval(code) and evalFile(path)
void registerEvalModule(Interpreter& interpreter);


