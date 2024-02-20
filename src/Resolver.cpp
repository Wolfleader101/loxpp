#include "Resolver.hpp"

#include "Interpreter.hpp"

Resolver::Resolver(ILogger& logger, Interpreter& interpreter) : logger(logger), interpreter(interpreter)
{
}

LoxTypeRef Resolver::visitBlockStmt(const BlockStmt<LoxTypeRef>& stmt)
{
    beginScope();
    resolve(stmt.statements);
    endScope();
    return nullptr;
}

LoxTypeRef Resolver::visitExpressionStmt(const ExpressionStmt<LoxTypeRef>& stmt)
{
    resolve(stmt.expression);
    return nullptr;
}

LoxTypeRef Resolver::visitFunctionStmt(const FunctionStmt<LoxTypeRef>& stmt)
{
    declare(stmt.name);
    define(stmt.name);

    resolveFunction(stmt, FunctionType::FUNCTION);
    return nullptr;
}

LoxTypeRef Resolver::visitIfStmt(const IfStmt<LoxTypeRef>& stmt)
{
    resolve(stmt.condition);
    resolve(stmt.thenBranch);
    if (stmt.elseBranch != nullptr)
    {
        resolve(stmt.elseBranch);
    }
    return nullptr;
}

LoxTypeRef Resolver::visitPrintStmt(const PrintStmt<LoxTypeRef>& stmt)
{
    resolve(stmt.expression);
}

LoxTypeRef Resolver::visitReturnStmt(const ReturnStmt<LoxTypeRef>& stmt)
{
    if (currentFunction == FunctionType::NONE)
    {
        logger.LogError(stmt.keyword, "Cannot return from top-level code.");
    }

    if (stmt.value != nullptr)
    {
        resolve(stmt.value);
    }
    return nullptr;
}

LoxTypeRef Resolver::visitVarStmt(const VarStmt<LoxTypeRef>& stmt)
{
    declare(stmt.name);
    if (stmt.initializer != nullptr)
    {
        resolve(stmt.initializer);
    }
    define(stmt.name);
    return nullptr;
}

LoxTypeRef Resolver::visitWhileStmt(const WhileStmt<LoxTypeRef>& stmt)
{
    resolve(stmt.condition);
    resolve(stmt.body);
    return nullptr;
}

LoxTypeRef Resolver::visitAssignExpr(const AssignExpr<LoxTypeRef>& expr)
{
    resolve(expr.value);
    resolveLocal(expr, expr.name);
    return nullptr;
}

LoxTypeRef Resolver::visitBinaryExpr(const BinaryExpr<LoxTypeRef>& expr)
{
    resolve(expr.left);
    resolve(expr.right);
    return nullptr;
}

LoxTypeRef Resolver::visitCallExpr(const CallExpr<LoxTypeRef>& expr)
{
    resolve(expr.callee);
    for (auto argument : expr.arguments)
    {
        resolve(argument);
    }
    return nullptr;
}

LoxTypeRef Resolver::visitGroupingExpr(const GroupingExpr<LoxTypeRef>& expr)
{
    resolve(expr.expression);
    return nullptr;
}

LoxTypeRef Resolver::visitLiteralExpr(const LiteralExpr<LoxTypeRef>& expr)
{
    // do nothing
    return nullptr;
}

LoxTypeRef Resolver::visitLogicalExpr(const LogicalExpr<LoxTypeRef>& expr)
{
    resolve(expr.left);
    resolve(expr.right);
    return nullptr;
}

LoxTypeRef Resolver::visitUnaryExpr(const UnaryExpr<LoxTypeRef>& expr)
{
    resolve(expr.right);

    return nullptr;
}

LoxTypeRef Resolver::visitVariableExpr(const VariableExpr<LoxTypeRef>& expr)
{
    if (!scopes.empty() && scopes.back().find(expr.name.lexeme) != scopes.back().end() &&
        !scopes.back()[expr.name.lexeme])
    {
        logger.LogError(expr.name, "Cannot read local variable in its own initializer.");
    }
    resolveLocal(expr, expr.name);
}

void Resolver::resolve(const std::vector<std::shared_ptr<Stmt<LoxTypeRef>>>& statements)
{
    for (auto statement : statements)
    {
        resolve(statement);
    }
}

void Resolver::resolve(std::shared_ptr<Stmt<LoxTypeRef>> statement)
{
    statement->accept(*this);
}

void Resolver::resolve(std::shared_ptr<Expr<LoxTypeRef>> expr)
{
    expr->accept(*this);
}

void Resolver::beginScope()
{
    scopes.push_back(std::unordered_map<std::string, bool>());
}

void Resolver::endScope()
{
    scopes.pop_back();
}
void Resolver::declare(const Token& name)
{
    if (scopes.empty())
        return;

    if (scopes.back().find(name.lexeme) != scopes.back().end() && scopes.back()[name.lexeme])
        logger.LogError(name, "Variable with this name already declared in this scope.");

    scopes.back()[name.lexeme] = false;
}

void Resolver::define(const Token& name)
{
    if (scopes.empty())
        return;
    scopes.back()[name.lexeme] = true;
}

void Resolver::resolveLocal(const Expr<LoxTypeRef>& expr, const Token& name)
{
    for (size_t i = scopes.size() - 1; i >= 0; i--)
    {
        if (scopes[i].find(name.lexeme) != scopes[i].end())
        {
            interpreter.resolve(&expr, scopes.size() - 1 - i);
            return;
        }
    }
}

void Resolver::resolveFunction(const FunctionStmt<LoxTypeRef>& stmt, FunctionType type)
{
    FunctionType enclosingFunction = currentFunction;
    currentFunction = type;

    beginScope();
    for (auto param : stmt.params)
    {
        declare(param);
        define(param);
    }

    resolve(stmt.body);
    endScope();

    currentFunction = enclosingFunction;
}