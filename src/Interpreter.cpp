#include "Interpreter.hpp"

#include <iostream>

Interpreter::Interpreter(ILogger& loggerRef) : logger(loggerRef), environment()
{
    environment = std::make_shared<Environment>();
}

void Interpreter::interpret(std::vector<std::shared_ptr<Stmt<LoxTypeRef>>>& statements)
{
    try
    {
        for (std::shared_ptr<Stmt<LoxTypeRef>> statement : statements)
        {
            execute(statement);
        }
    }
    catch (RuntimeError& error)
    {
        logger.LogRuntimeError(error);
    }
}

LoxTypeRef Interpreter::visitLiteralExpr(const LiteralExpr<LoxTypeRef>& expr)
{
    return expr.value;
}

LoxTypeRef Interpreter::visitGroupingExpr(const GroupingExpr<LoxTypeRef>& expr)
{
    return evaluate(expr.expression);
}

LoxTypeRef Interpreter::visitUnaryExpr(const UnaryExpr<LoxTypeRef>& expr)
{
    LoxTypeRef right = evaluate(expr.right);

    switch (expr.op.type)
    {
    case TokenType::BANG:
        return std::make_shared<LoxType>(!isTruthy(right));
    case TokenType::MINUS:
        checkNumberOperands(expr.op, right);
        if (IsDouble(*right))
            return std::make_shared<LoxType>(-std::get<double>(right->value()));
    }

    return nullptr;
}

LoxTypeRef Interpreter::visitCallExpr(const CallExpr<LoxTypeRef>& expr)
{
    LoxTypeRef callee = evaluate(expr.callee);

    std::vector<LoxTypeRef> arguments;

    for (auto argument : expr.arguments)
    {
        arguments.push_back(evaluate(argument));
    }

    std::shared_ptr<LoxCallable> function = std::get<std::shared_ptr<LoxCallable>>(callee->value());

    return function->call(*this, arguments);
}

LoxTypeRef Interpreter::visitBinaryExpr(const BinaryExpr<LoxTypeRef>& expr)
{
    LoxTypeRef left = evaluate(expr.left);
    LoxTypeRef right = evaluate(expr.right);

    switch (expr.op.type)
    {
    case TokenType::GREATER:
        checkNumberOperands(expr.op, left, right);
        return std::make_shared<LoxType>(std::get<double>(left->value()) > std::get<double>(right->value()));
    case TokenType::GREATER_EQUAL:
        checkNumberOperands(expr.op, left, right);
        return std::make_shared<LoxType>(std::get<double>(left->value()) >= std::get<double>(right->value()));
    case TokenType::LESS:
        checkNumberOperands(expr.op, left, right);
        return std::make_shared<LoxType>(std::get<double>(left->value()) < std::get<double>(right->value()));
    case TokenType::LESS_EQUAL:
        checkNumberOperands(expr.op, left, right);
        return std::make_shared<LoxType>(std::get<double>(left->value()) <= std::get<double>(right->value()));
    case TokenType::BANG_EQUAL:
        return std::make_shared<LoxType>(!isEqual(left, right));
    case TokenType::EQUAL_EQUAL:
        return std::make_shared<LoxType>(isEqual(left, right));
    case TokenType::MINUS:
        checkNumberOperands(expr.op, left, right);
        return std::make_shared<LoxType>(std::get<double>(left->value()) - std::get<double>(right->value()));
    case TokenType::PLUS:
        if (IsDouble(*left) && IsDouble(*right))
        {
            return std::make_shared<LoxType>(std::get<double>(left->value()) + std::get<double>(right->value()));
        }

        if (IsString(*left) && IsString(*right))
        {

            return std::make_shared<LoxType>(std::get<std::string>(left->value()) +
                                             std::get<std::string>(right->value()));
        }

        throw RuntimeError(expr.op, "Operands must be two numbers or two strings.");
        break;
    case TokenType::SLASH:
        checkNumberOperands(expr.op, left, right);
        return std::make_shared<LoxType>(std::get<double>(left->value()) / std::get<double>(right->value()));
    case TokenType::STAR:
        checkNumberOperands(expr.op, left, right);
        return std::make_shared<LoxType>(std::get<double>(left->value()) * std::get<double>(right->value()));
    }

    return nullptr;
}

LoxTypeRef Interpreter::visitVariableExpr(const VariableExpr<LoxTypeRef>& expr)
{
    return environment->get(expr.name);
}

LoxTypeRef Interpreter::visitAssignExpr(const AssignExpr<LoxTypeRef>& expr)
{
    LoxTypeRef value = evaluate(expr.value);

    environment->assign(expr.name, value);

    return value;
}

LoxTypeRef Interpreter::visitLogicalExpr(const LogicalExpr<LoxTypeRef>& expr)
{
    LoxTypeRef left = evaluate(expr.left);

    if (expr.op.type == TokenType::OR)
    {
        if (isTruthy(left))
            return left;
    }
    else
    {
        if (!isTruthy(left))
            return left;
    }

    return evaluate(expr.right);
}

LoxTypeRef Interpreter::visitExpressionStmt(const ExpressionStmt<LoxTypeRef>& stmt)
{
    evaluate(stmt.expression);

    return nullptr;
}

LoxTypeRef Interpreter::visitPrintStmt(const PrintStmt<LoxTypeRef>& stmt)
{
    LoxTypeRef value = evaluate(stmt.expression);
    std::cout << LoxTypeToString(*value) << std::endl;

    return nullptr;
}

LoxTypeRef Interpreter::visitBlockStmt(const BlockStmt<LoxTypeRef>& stmt)
{
    executeBlock(stmt.statements, std::make_shared<Environment>(environment));

    return nullptr;
}

LoxTypeRef Interpreter::visitIfStmt(const IfStmt<LoxTypeRef>& stmt)
{
    if (isTruthy(evaluate(stmt.condition)))
    {
        execute(stmt.thenBranch);
    }
    else if (stmt.elseBranch != nullptr)
    {
        execute(stmt.elseBranch);
    }

    return nullptr;
}

LoxTypeRef Interpreter::visitWhileStmt(const WhileStmt<LoxTypeRef>& stmt)
{
    while (isTruthy(evaluate(stmt.condition)))
    {
        execute(stmt.body);
    }

    return nullptr;
}

LoxTypeRef Interpreter::visitVarStmt(const VarStmt<LoxTypeRef>& stmt)
{
    LoxTypeRef value = nullptr;
    if (stmt.initializer != nullptr)
    {
        value = evaluate(stmt.initializer);
    }

    environment->define(stmt.name.lexeme, value);

    return nullptr;
}

void Interpreter::executeBlock(const std::vector<std::shared_ptr<Stmt<LoxTypeRef>>>& statements,
                               std::shared_ptr<Environment> environment)
{
    std::shared_ptr<Environment> previous = this->environment;

    try
    {
        this->environment = environment;

        for (std::shared_ptr<Stmt<LoxTypeRef>> statement : statements)
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

void Interpreter::execute(std::shared_ptr<Stmt<LoxTypeRef>> stmt)
{
    stmt->accept(*this);
}

LoxTypeRef Interpreter::evaluate(std::shared_ptr<Expr<LoxTypeRef>> expr)
{
    return expr->accept(*this);
}

bool Interpreter::isTruthy(LoxTypeRef object)
{
    return isTruthy(*object);
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

bool Interpreter::isEqual(LoxTypeRef a, LoxTypeRef b)
{
    return isEqual(*a, *b);
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

void Interpreter::checkNumberOperands(const Token& op, LoxTypeRef operand)
{
    checkNumberOperands(op, *operand);
}

void Interpreter::checkNumberOperands(const Token& op, LoxTypeRef left, LoxTypeRef right)
{
    checkNumberOperands(op, *left, *right);
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
