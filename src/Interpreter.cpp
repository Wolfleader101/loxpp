#include "Interpreter.hpp"

#include <iostream>

Interpreter::Interpreter(ILogger& loggerRef) : logger(loggerRef), environment()
{
    environment = std::make_shared<Environment>();
}

void Interpreter::interpret(std::vector<std::shared_ptr<Stmt<LoxType>>> statements)
{
    try
    {
        for (std::shared_ptr<Stmt<LoxType>> statement : statements)
        {
            execute(statement);
        }
    }
    catch (RuntimeError& error)
    {
        logger.LogRuntimeError(error);
    }
}

LoxType Interpreter::visitLiteralExpr(const LiteralExpr<LoxType>& expr)
{
    return expr.value;
}

LoxType Interpreter::visitGroupingExpr(const GroupingExpr<LoxType>& expr)
{
    return evaluate(expr.expression);
}

LoxType Interpreter::visitUnaryExpr(const UnaryExpr<LoxType>& expr)
{
    LoxType right = evaluate(expr.right);

    switch (expr.op.type)
    {
    case TokenType::BANG:
        return !isTruthy(right);
    case TokenType::MINUS:
        checkNumberOperands(expr.op, right);
        return -std::get<double>(right.value());
    }

    return LoxType();
}

LoxType Interpreter::visitBinaryExpr(const BinaryExpr<LoxType>& expr)
{
    LoxType left = evaluate(expr.left);
    LoxType right = evaluate(expr.right);

    switch (expr.op.type)
    {
    case TokenType::GREATER:
        checkNumberOperands(expr.op, left, right);
        return std::get<double>(left.value()) > std::get<double>(right.value());
    case TokenType::GREATER_EQUAL:
        checkNumberOperands(expr.op, left, right);
        return std::get<double>(left.value()) >= std::get<double>(right.value());
    case TokenType::LESS:
        checkNumberOperands(expr.op, left, right);
        return std::get<double>(left.value()) < std::get<double>(right.value());
    case TokenType::LESS_EQUAL:
        checkNumberOperands(expr.op, left, right);
        return std::get<double>(left.value()) <= std::get<double>(right.value());
    case TokenType::BANG_EQUAL:
        return !isEqual(left, right);
    case TokenType::EQUAL_EQUAL:
        return isEqual(left, right);
    case TokenType::MINUS:
        checkNumberOperands(expr.op, left, right);
        return std::get<double>(left.value()) - std::get<double>(right.value());
    case TokenType::PLUS:
        if (IsDouble(left) && IsDouble(right))
        {
            return std::get<double>(left.value()) + std::get<double>(right.value());
        }

        if (IsString(left) && IsString(right))
        {
            return std::get<std::string>(left.value()) + std::get<std::string>(right.value());
        }

        throw RuntimeError(expr.op, "Operands must be two numbers or two strings.");
        break;
    case TokenType::SLASH:
        checkNumberOperands(expr.op, left, right);
        return std::get<double>(left.value()) / std::get<double>(right.value());
    case TokenType::STAR:
        checkNumberOperands(expr.op, left, right);
        return std::get<double>(left.value()) * std::get<double>(right.value());
    }

    return LoxType();
}

LoxType Interpreter::visitVariableExpr(const VariableExpr<LoxType>& expr)
{
    return *environment->get(expr.name);
}

LoxType Interpreter::visitAssignExpr(const AssignExpr<LoxType>& expr)
{
    LoxType value = evaluate(expr.value);

    environment->assign(expr.name, std::make_shared<LoxType>(value));

    return value;
}

LoxType Interpreter::visitExpressionStmt(const ExpressionStmt<LoxType>& stmt)
{
    evaluate(stmt.expression);

    return std::nullopt;
}

LoxType Interpreter::visitPrintStmt(const PrintStmt<LoxType>& stmt)
{
    LoxType value = evaluate(stmt.expression);
    std::cout << LoxTypeToString(value) << std::endl;

    return std::nullopt;
}

LoxType Interpreter::visitBlockStmt(const BlockStmt<LoxType>& stmt)
{
    executeBlock(stmt.statements, std::make_shared<Environment>(environment));

    return std::nullopt;
}

void Interpreter::executeBlock(const std::vector<std::shared_ptr<Stmt<LoxType>>>& statements,
                               std::shared_ptr<Environment> environment)
{
    std::shared_ptr<Environment> previous = this->environment;

    try
    {
        this->environment = environment;

        for (std::shared_ptr<Stmt<LoxType>> statement : statements)
        {
            execute(statement);
        }
    }
    catch (...)
    {
        this->environment = previous;
        throw;
    }

    this->environment = previous;
}

void Interpreter::execute(std::shared_ptr<Stmt<LoxType>> stmt)
{
    stmt->accept(*this);
}

LoxType Interpreter::evaluate(std::shared_ptr<Expr<LoxType>> expr)
{
    return expr->accept(*this);
}

bool Interpreter::isTruthy(const LoxType& object)
{
    if (!object.has_value())
    {
        return false;
    }

    return std::visit(
        [](const auto& value) -> bool {
            using T = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<T, bool>)
            {
                return value;
            }
            else
            {
                return true;
            }
        },
        object.value());
}

bool Interpreter::isEqual(const LoxType& a, const LoxType& b)
{
    if (!a.has_value() && !b.has_value())
    {
        return true;
    }

    if (!a.has_value() || !b.has_value())
    {
        return false;
    }

    return std::visit(
        [&b](const auto& aValue) -> bool {
            return std::visit(
                [&aValue](const auto& bValue) -> bool {
                    using Ta = std::decay_t<decltype(aValue)>;
                    using Tb = std::decay_t<decltype(bValue)>;

                    // if the types are different, they are not equal
                    if constexpr (!std::is_same_v<Ta, Tb>)
                    {
                        return false;
                    }
                    else
                    {
                        return aValue == bValue;
                    }
                },
                b.value());
        },
        a.value());
}

void Interpreter::checkNumberOperands(const Token& op, const LoxType& operand)
{
    if (IsDouble(operand))
        return;

    throw RuntimeError(op, "Operand must be a number.");
}

void Interpreter::checkNumberOperands(const Token& op, const LoxType& left, const LoxType& right)
{
    if (IsDouble(left) && IsDouble(right))
        return;

    throw RuntimeError(op, "Operands must be numbers.");
}

LoxType Interpreter::visitVarStmt(const VarStmt<LoxType>& stmt)
{
    LoxType value = std::nullopt;
    if (stmt.initializer != nullptr)
    {
        value = evaluate(stmt.initializer);
    }

    environment->define(stmt.name.lexeme, std::make_shared<LoxType>(value));

    return std::nullopt;
}