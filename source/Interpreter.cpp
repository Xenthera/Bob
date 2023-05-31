//
// Created by Bobby Lucero on 5/27/23.
//
#include <utility>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <limits>
#include <cmath>
#include "../headers/Interpreter.h"
#include "../headers/helperFunctions/HelperFunctions.h"


sptr(Object) Interpreter::visitLiteralExpr(sptr(LiteralExpr) expr) {
    if(expr->isNull) return msptr(None)();
    if(expr->isNumber){
        double num;
        if(expr->value[1] == 'b')
        {
            num = binaryStringToLong(expr->value);
        }
        else
        {
            num = std::stod(expr->value);
        }
        return msptr(Number)(num);
    }
    if(expr->value == "true") return msptr(Boolean)(true);
    if(expr->value == "false") return msptr(Boolean)(false);
    return msptr(String)(expr->value);
}

sptr(Object) Interpreter::visitGroupingExpr(sptr(GroupingExpr) expression) {

    return evaluate(expression->expression);
}

sptr(Object) Interpreter::visitUnaryExpr(sptr(UnaryExpr) expression)
{
    sptr(Object) right = evaluate(expression->right);

    if(expression->oper.type == MINUS)
    {
        if(std::dynamic_pointer_cast<Number>(right))
        {
            double value = std::dynamic_pointer_cast<Number>(right)->value;
            return msptr(Number)(-value);
        }
        else
        {
            throw std::runtime_error("Operand must be a number when using: " + expression->oper.lexeme);
        }

    }

    if(expression->oper.type == BANG)
    {
        return msptr(Boolean)(!isTruthy(right));
    }

    //unreachable
    throw std::runtime_error("Invalid unary expression");

}

sptr(Object) Interpreter::visitBinaryExpr(sptr(BinaryExpr) expression) 
{
    sptr(Object) left = evaluate(expression->left);
    sptr(Object) right = evaluate(expression->right);

    switch (expression->oper.type) {
        case BANG_EQUAL:
            return msptr(Boolean)(!isEqual(left, right));
        case DOUBLE_EQUAL:
            return msptr(Boolean)(isEqual(left, right));
        default:
            ;
    }

    if(std::dynamic_pointer_cast<Number>(left) && std::dynamic_pointer_cast<Number>(right))
    {
        double left_double = std::dynamic_pointer_cast<Number>(left)->value;
        double right_double = std::dynamic_pointer_cast<Number>(right)->value;
        switch (expression->oper.type) {
            case GREATER:
                return msptr(Boolean)(left_double > right_double);
            case GREATER_EQUAL:
                return msptr(Boolean)(left_double >= right_double);
            case LESS:
                return msptr(Boolean)(left_double < right_double);
            case LESS_EQUAL:
                return msptr(Boolean)(left_double <= right_double);
            case MINUS:
                return msptr(Number)(left_double - right_double);
            case PLUS:
                return msptr(Number)(left_double + right_double);
            case SLASH:
                if(right_double == 0) throw std::runtime_error("DivisionByZeroError: Cannot divide by 0");
                return msptr(Number)(left_double / right_double);
            case STAR:
                return msptr(Number)(left_double * right_double);
            case PERCENT:
                return msptr(Number)(fmod(left_double, right_double));
            default:
                return msptr(None)(); //unreachable
        }
    }
    else if(std::dynamic_pointer_cast<String>(left) && std::dynamic_pointer_cast<String>(right))
    {
        switch (expression->oper.type) {
            case PLUS:
                std::string left_string = std::dynamic_pointer_cast<String>(left)->value;
                std::string right_string = std::dynamic_pointer_cast<String>(right)->value;
                return msptr(String)(left_string + right_string);
        }
        throw std::runtime_error("Cannot use '" + expression->oper.lexeme + "' on two strings");

    }
    else if(std::dynamic_pointer_cast<String>(left) && std::dynamic_pointer_cast<Number>(right))
    {
        switch (expression->oper.type) {
            case STAR:
                std::string left_string = std::dynamic_pointer_cast<String>(left)->value;
                double right_number = std::dynamic_pointer_cast<Number>(right)->value;
                if(isWholeNumer(right_number))
                {
                    std::string s;
                    for (int i = 0; i < (int)right_number; ++i) {
                        s += left_string;
                    }
                    return msptr(String)(s);

                }
                else
                {
                    throw std::runtime_error("String multiplier must be whole number");
                }
        }
        throw std::runtime_error("Cannot use '" + expression->oper.lexeme + "' on a string and a number");

    }
    else
    {
        throw std::runtime_error("Operands must be of same type when using: " + expression->oper.lexeme);
    }

}

sptr(Object) Interpreter::visitVariableExpr(sptr(VarExpr) expression)
{
    return environment.get(expression->name);
}

sptr(Object) Interpreter::visitAssignExpr(sptr(AssignExpr) expression) {
    sptr(Object) value = evaluate(expression->value);
    environment.assign(expression->name, value);
    return value;
}

void Interpreter::visitExpressionStmt(sptr(ExpressionStmt) statement) {
    evaluate(statement->expression);
}

void Interpreter::visitPrintStmt(sptr(PrintStmt) statement) {
    sptr(Object) value = evaluate(statement->expression);
    std::cout << stringify(value) << std::endl;
}

void Interpreter::visitVarStmt(sptr(VarStmt) statement)
{
    sptr(Object) value = msptr(None)();
    if(!std::dynamic_pointer_cast<None>(statement->initializer))
    {
        value = evaluate(statement->initializer);
    }

    //std::cout << "Visit var stmt: " << statement->name.lexeme << " set to: " << stringify(value) << std::endl;

    environment.define(statement->name.lexeme, value);
}

void Interpreter::interpret(std::vector<sptr(Stmt)> statements) {


    for(sptr(Stmt) s : statements)
    {
        execute(s);
    }

    //std::cout << "\033[0;32m" << stringify(value) << std::endl;

}

void Interpreter::execute(sptr(Stmt) statement)
{
    statement->accept(this);
}

sptr(Object) Interpreter::evaluate(sptr(Expr) expr) {
    return expr->accept(this);
}

bool Interpreter::isTruthy(sptr(Object) object) {

    if(auto boolean = std::dynamic_pointer_cast<Boolean>(object))
    {
        return boolean->value;
    }

    if(auto obj = std::dynamic_pointer_cast<None>(object))
    {
        return false;
    }

    return true;
}

bool Interpreter::isEqual(sptr(Object) a, sptr(Object) b) {
    if(auto left = std::dynamic_pointer_cast<Number>(a))
    {
        if(auto right = std::dynamic_pointer_cast<Number>(b))
        {
            return left->value == right->value;
        }

        return false;
    }
    else if(auto left = std::dynamic_pointer_cast<Boolean>(a))
    {
        if(auto right = std::dynamic_pointer_cast<Boolean>(b))
        {
            return left->value == right->value;
        }

        return false;
    }
    else if(auto left = std::dynamic_pointer_cast<String>(a))
    {
        if(auto right = std::dynamic_pointer_cast<String>(b))
        {
            return left->value == right->value;
        }

        return false;
    }
    else if(auto left = std::dynamic_pointer_cast<None>(a))
    {
        if(auto right = std::dynamic_pointer_cast<None>(b))
        {
            return true;
        }

        return false;
    }

    throw std::runtime_error("Invalid isEqual compariosn");
}

std::string Interpreter::stringify(std::shared_ptr<Object> object) {
    if(std::dynamic_pointer_cast<None>(object))
    {
        return "none";
    }
    else if(auto num = std::dynamic_pointer_cast<Number>(object))
    {
        double integral = num->value;
        double fractional = std::modf(num->value, &integral);

        std::stringstream ss;
        if(std::abs(fractional) < std::numeric_limits<double>::epsilon())
        {
            ss << std::fixed << std::setprecision(0) << integral;

            return ss.str();
        }
        else
        {
            ss << std::fixed << std::setprecision(std::numeric_limits<double>::digits10 - 1) << num->value;
            std::string str = ss.str();
            str.erase(str.find_last_not_of('0') + 1, std::string::npos);
            if (str.back() == '.') {
                str.pop_back();
            }

            return str;
        }
    }
    else if(auto string = std::dynamic_pointer_cast<String>(object))
    {
        return string->value;
    }
    else if(auto Bool = std::dynamic_pointer_cast<Boolean>(object))
    {
        return Bool->value == 1 ? "true" : "false";
    }

    throw std::runtime_error("Could not convert object to string");
}

bool Interpreter::isWholeNumer(double num) {
    double integral = num;
    double fractional = std::modf(num, &integral);

    if(std::abs(fractional) < std::numeric_limits<double>::epsilon())
    {
        return true;
    }
    else
    {
        return false;
    }
}









