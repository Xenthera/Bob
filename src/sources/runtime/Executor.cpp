#include "Executor.h"
#include "Evaluator.h"
#include "Interpreter.h"
#include <iostream>

Executor::Executor(Interpreter* interpreter, Evaluator* evaluator) 
    : interpreter(interpreter), evaluator(evaluator) {}

Executor::~Executor() {}

void Executor::interpret(const std::vector<std::shared_ptr<Stmt>>& statements) {
    for (const auto& statement : statements) {
        execute(statement, nullptr);
    }
}

void Executor::execute(const std::shared_ptr<Stmt>& statement, ExecutionContext* context) {
    statement->accept(this, context);
}

void Executor::executeBlock(const std::vector<std::shared_ptr<Stmt>>& statements, std::shared_ptr<Environment> env, ExecutionContext* context) {
    std::shared_ptr<Environment> previous = interpreter->getEnvironment();
    interpreter->setEnvironment(env);

    for (const auto& statement : statements) {
        execute(statement, context);
        if (context && (context->hasReturn || context->shouldBreak || context->shouldContinue)) {
            interpreter->setEnvironment(previous);
            return;
        }
    }
    interpreter->setEnvironment(previous);
}

void Executor::visitBlockStmt(const std::shared_ptr<BlockStmt>& statement, ExecutionContext* context) {
    auto newEnv = std::make_shared<Environment>(interpreter->getEnvironment());
    executeBlock(statement->statements, newEnv, context);
}

void Executor::visitExpressionStmt(const std::shared_ptr<ExpressionStmt>& statement, ExecutionContext* context) {
    Value value = statement->expression->accept(evaluator);

    if (interpreter->isInteractiveMode())
        std::cout << "\u001b[38;5;8m[" << interpreter->stringify(value) << "]\u001b[38;5;15m\n";
}

void Executor::visitVarStmt(const std::shared_ptr<VarStmt>& statement, ExecutionContext* context) {
    Value value = NONE_VALUE;
    if (statement->initializer != nullptr) {
        value = statement->initializer->accept(evaluator);
    }
    interpreter->getEnvironment()->define(statement->name.lexeme, value);
}

void Executor::visitFunctionStmt(const std::shared_ptr<FunctionStmt>& statement, ExecutionContext* context) {
    std::vector<std::string> paramNames;
    for (const Token& param : statement->params) {
        paramNames.push_back(param.lexeme);
    }
    
    auto function = std::make_shared<Function>(statement->name.lexeme, 
                                   paramNames, 
                                   statement->body, 
                                   interpreter->getEnvironment());
    interpreter->addFunction(function);
    interpreter->getEnvironment()->define(statement->name.lexeme, Value(function));
}

void Executor::visitReturnStmt(const std::shared_ptr<ReturnStmt>& statement, ExecutionContext* context) {
    Value value = NONE_VALUE;
    if (statement->value != nullptr) {
        value = statement->value->accept(evaluator);
    }
    
    if (context && context->isFunctionBody) {
        context->hasReturn = true;
        context->returnValue = value;
    }
}

void Executor::visitIfStmt(const std::shared_ptr<IfStmt>& statement, ExecutionContext* context) {
    if (interpreter->isTruthy(statement->condition->accept(evaluator))) {
        execute(statement->thenBranch, context);
    } else if (statement->elseBranch != nullptr) {
        execute(statement->elseBranch, context);
    }
}

void Executor::visitWhileStmt(const std::shared_ptr<WhileStmt>& statement, ExecutionContext* context) {
    ExecutionContext loopContext;
    if (context) {
        loopContext.isFunctionBody = context->isFunctionBody;
    }
    
    while (interpreter->isTruthy(statement->condition->accept(evaluator))) {
        execute(statement->body, &loopContext);
        
        if (loopContext.hasReturn) {
            if (context) {
                context->hasReturn = true;
                context->returnValue = loopContext.returnValue;
            }
            break;
        }
        
        if (loopContext.shouldBreak) {
            break;
        }
        
        if (loopContext.shouldContinue) {
            loopContext.shouldContinue = false;
            continue;
        }
    }
}

void Executor::visitDoWhileStmt(const std::shared_ptr<DoWhileStmt>& statement, ExecutionContext* context) {
    ExecutionContext loopContext;
    if (context) {
        loopContext.isFunctionBody = context->isFunctionBody;
    }
    
    do {
        execute(statement->body, &loopContext);
        
        if (loopContext.hasReturn) {
            if (context) {
                context->hasReturn = true;
                context->returnValue = loopContext.returnValue;
            }
            break;
        }
        
        if (loopContext.shouldBreak) {
            break;
        }
        
        if (loopContext.shouldContinue) {
            loopContext.shouldContinue = false;
            continue;
        }
    } while (interpreter->isTruthy(statement->condition->accept(evaluator)));
}

void Executor::visitForStmt(const std::shared_ptr<ForStmt>& statement, ExecutionContext* context) {
    if (statement->initializer != nullptr) {
        execute(statement->initializer, context);
    }
    
    ExecutionContext loopContext;
    if (context) {
        loopContext.isFunctionBody = context->isFunctionBody;
    }
    
    while (statement->condition == nullptr || interpreter->isTruthy(statement->condition->accept(evaluator))) {
        execute(statement->body, &loopContext);
        
        if (loopContext.hasReturn) {
            if (context) {
                context->hasReturn = true;
                context->returnValue = loopContext.returnValue;
            }
            break;
        }
        
        if (loopContext.shouldBreak) {
            break;
        }
        
        if (loopContext.shouldContinue) {
            loopContext.shouldContinue = false;
            if (statement->increment != nullptr) {
                statement->increment->accept(evaluator);
            }
            continue;
        }
        
        if (statement->increment != nullptr) {
            statement->increment->accept(evaluator);
        }
    }
}

void Executor::visitBreakStmt(const std::shared_ptr<BreakStmt>& statement, ExecutionContext* context) {
    if (context) {
        context->shouldBreak = true;
    }
}

void Executor::visitContinueStmt(const std::shared_ptr<ContinueStmt>& statement, ExecutionContext* context) {
    if (context) {
        context->shouldContinue = true;
    }
}

void Executor::visitAssignStmt(const std::shared_ptr<AssignStmt>& statement, ExecutionContext* context) {
    try {
        Value value = statement->value->accept(evaluator);
        
        if (statement->op.type == EQUAL) {
            try {
                // Assign first to release references held by the old value
                interpreter->getEnvironment()->assign(statement->name, value);
                
                // Clean up on any reassignment, regardless of old/new type
                interpreter->forceCleanup();
            } catch (const std::exception& e) {
                std::cerr << "Error during assignment: " << e.what() << std::endl;
                throw; // Re-throw to see the full stack trace
            }
        } else {
        // Handle compound assignment operators
        Value currentValue = interpreter->getEnvironment()->get(statement->name);
        Value newValue;
        
        switch (statement->op.type) {
            case PLUS_EQUAL:
                newValue = currentValue + value;
                break;
            case MINUS_EQUAL:
                newValue = currentValue - value;
                break;
            case STAR_EQUAL:
                newValue = currentValue * value;
                break;
            case SLASH_EQUAL:
                newValue = currentValue / value;
                break;
            case PERCENT_EQUAL:
                newValue = currentValue % value;
                break;
            case BIN_AND_EQUAL:
                newValue = currentValue & value;
                break;
            case BIN_OR_EQUAL:
                newValue = currentValue | value;
                break;
            case BIN_XOR_EQUAL:
                newValue = currentValue ^ value;
                break;
            case BIN_SLEFT_EQUAL:
                newValue = currentValue << value;
                break;
            case BIN_SRIGHT_EQUAL:
                newValue = currentValue >> value;
                break;
            default:
                interpreter->reportError(statement->op.line, statement->op.column, "Runtime Error",
                    "Unknown assignment operator: " + statement->op.lexeme, "");
                throw std::runtime_error("Unknown assignment operator: " + statement->op.lexeme);
        }
        
        interpreter->getEnvironment()->assign(statement->name, newValue);
    }
    } catch (const std::exception& e) {
        std::cerr << "Error in visitAssignStmt: " << e.what() << std::endl;
        throw; // Re-throw to see the full stack trace
    }
}
