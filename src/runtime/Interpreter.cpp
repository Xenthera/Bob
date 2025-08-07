#include "Interpreter.h"
#include "Evaluator.h"
#include "Executor.h"
#include "BobStdLib.h"
#include <iostream>

Interpreter::Interpreter(bool isInteractive) 
    : isInteractive(isInteractive), errorReporter(nullptr) {
    evaluator = std::make_unique<Evaluator>(this);
    executor = std::make_unique<Executor>(this, evaluator.get());
    environment = std::make_shared<Environment>();
}

Interpreter::~Interpreter() = default;

void Interpreter::interpret(std::vector<std::shared_ptr<Stmt>> statements) {
    executor->interpret(statements);
}

void Interpreter::execute(const std::shared_ptr<Stmt>& statement, ExecutionContext* context) {
    statement->accept(executor.get(), context);
}

void Interpreter::executeBlock(std::vector<std::shared_ptr<Stmt>> statements, std::shared_ptr<Environment> env, ExecutionContext* context) {
    executor->executeBlock(statements, env, context);
}

Value Interpreter::evaluate(const std::shared_ptr<Expr>& expr) {
    Value result = expr->accept(evaluator.get());
    if (inThunkExecution) {
        return result;
    }
    return runTrampoline(result);
}

Value Interpreter::runTrampoline(Value initialResult) {
    Value current = initialResult;
    while (current.isThunk()) {
        current = current.asThunk()->execute();
    }
    return current;
}

bool Interpreter::isTruthy(Value object) {
    return diagnostics.isTruthy(object);
}

bool Interpreter::isEqual(Value a, Value b) {
    return diagnostics.isEqual(a, b);
}

std::string Interpreter::stringify(Value object) {
    return diagnostics.stringify(object);
}

void Interpreter::addStdLibFunctions() {
    BobStdLib::addToEnvironment(environment, *this, errorReporter);
}

void Interpreter::addBuiltinFunction(std::shared_ptr<BuiltinFunction> func) {
    builtinFunctions.push_back(func);
}

void Interpreter::addThunk(std::shared_ptr<Thunk> thunk) {
    thunks.push_back(thunk);
}

void Interpreter::addFunction(std::shared_ptr<Function> function) {
    functions.push_back(function);
}

void Interpreter::setErrorReporter(ErrorReporter* reporter) {
    errorReporter = reporter;
    if (environment) {
        environment->setErrorReporter(reporter);
    }
    addStdLibFunctions();
}

bool Interpreter::isInteractiveMode() const {
    return isInteractive;
}

std::shared_ptr<Environment> Interpreter::getEnvironment() {
    return environment;
}

void Interpreter::setEnvironment(std::shared_ptr<Environment> env) {
    environment = env;
}

void Interpreter::reportError(int line, int column, const std::string& errorType, const std::string& message, const std::string& lexeme) {
    if (errorReporter) {
        errorReporter->reportError(line, column, errorType, message, lexeme);
    }
}
