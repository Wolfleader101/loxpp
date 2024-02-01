#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Expr.hpp"
#include "LoxType.hpp"
#include "Token.hpp"

template <typename T>
class StmtVisitor;

template <typename T>
class Stmt
{
  public:
    virtual ~Stmt() = default;
    virtual T accept(StmtVisitor<T>& visitor) const = 0;
};

template <typename T>
class BlockStmt;

template <typename T>
class ExpressionStmt;

template <typename T>
class FunctionStmt;

template <typename T>
class IfStmt;

template <typename T>
class PrintStmt;

template <typename T>
class ReturnStmt;

template <typename T>
class VarStmt;

template <typename T>
class WhileStmt;

template <typename T>
class StmtVisitor
{
  public:
    virtual ~StmtVisitor() = default;
    virtual T visitBlockStmt(const BlockStmt<T>& stmt) = 0;

    virtual T visitExpressionStmt(const ExpressionStmt<T>& stmt) = 0;

    virtual T visitFunctionStmt(const FunctionStmt<T>& stmt) = 0;

    virtual T visitIfStmt(const IfStmt<T>& stmt) = 0;

    virtual T visitPrintStmt(const PrintStmt<T>& stmt) = 0;

    virtual T visitReturnStmt(const ReturnStmt<T>& stmt) = 0;

    virtual T visitVarStmt(const VarStmt<T>& stmt) = 0;

    virtual T visitWhileStmt(const WhileStmt<T>& stmt) = 0;
};
template <typename T>
class BlockStmt : public Stmt<T>
{
  public:
    BlockStmt(std::vector<std::shared_ptr<Stmt<T>>> statements) : statements(statements)
    {
    }
    T accept(StmtVisitor<T>& visitor) const override
    {
        return visitor.visitBlockStmt(*this);
    }
    std::vector<std::shared_ptr<Stmt<T>>> statements;
};

template <typename T>
class ExpressionStmt : public Stmt<T>
{
  public:
    ExpressionStmt(std::shared_ptr<Expr<T>> expression) : expression(expression)
    {
    }
    T accept(StmtVisitor<T>& visitor) const override
    {
        return visitor.visitExpressionStmt(*this);
    }
    std::shared_ptr<Expr<T>> expression;
};

template <typename T>
class FunctionStmt : public Stmt<T>
{
  public:
    FunctionStmt(Token name, std::vector<Token> params, std::vector<std::shared_ptr<Stmt<T>>> body)
        : name(name), params(params), body(body)
    {
    }
    T accept(StmtVisitor<T>& visitor) const override
    {
        return visitor.visitFunctionStmt(*this);
    }
    Token name;
    std::vector<Token> params;
    std::vector<std::shared_ptr<Stmt<T>>> body;
};

template <typename T>
class IfStmt : public Stmt<T>
{
  public:
    IfStmt(std::shared_ptr<Expr<T>> condition, std::shared_ptr<Stmt<T>> thenBranch, std::shared_ptr<Stmt<T>> elseBranch)
        : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch)
    {
    }
    T accept(StmtVisitor<T>& visitor) const override
    {
        return visitor.visitIfStmt(*this);
    }
    std::shared_ptr<Expr<T>> condition;
    std::shared_ptr<Stmt<T>> thenBranch;
    std::shared_ptr<Stmt<T>> elseBranch;
};

template <typename T>
class PrintStmt : public Stmt<T>
{
  public:
    PrintStmt(std::shared_ptr<Expr<T>> expression) : expression(expression)
    {
    }
    T accept(StmtVisitor<T>& visitor) const override
    {
        return visitor.visitPrintStmt(*this);
    }
    std::shared_ptr<Expr<T>> expression;
};

template <typename T>
class ReturnStmt : public Stmt<T>
{
  public:
    ReturnStmt(Token keyword, std::shared_ptr<Expr<T>> value) : keyword(keyword), value(value)
    {
    }
    T accept(StmtVisitor<T>& visitor) const override
    {
        return visitor.visitReturnStmt(*this);
    }
    Token keyword;
    std::shared_ptr<Expr<T>> value;
};

template <typename T>
class VarStmt : public Stmt<T>
{
  public:
    VarStmt(Token name, std::shared_ptr<Expr<T>> initializer) : name(name), initializer(initializer)
    {
    }
    T accept(StmtVisitor<T>& visitor) const override
    {
        return visitor.visitVarStmt(*this);
    }
    Token name;
    std::shared_ptr<Expr<T>> initializer;
};

template <typename T>
class WhileStmt : public Stmt<T>
{
  public:
    WhileStmt(std::shared_ptr<Expr<T>> condition, std::shared_ptr<Stmt<T>> body) : condition(condition), body(body)
    {
    }
    T accept(StmtVisitor<T>& visitor) const override
    {
        return visitor.visitWhileStmt(*this);
    }
    std::shared_ptr<Expr<T>> condition;
    std::shared_ptr<Stmt<T>> body;
};
