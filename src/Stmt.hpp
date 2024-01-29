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
class PrintStmt;

template <typename T>
class VarStmt;

template <typename T>
class StmtVisitor
{
  public:
    virtual ~StmtVisitor() = default;
    virtual T visitBlockStmt(const BlockStmt<T>& stmt) = 0;

    virtual T visitExpressionStmt(const ExpressionStmt<T>& stmt) = 0;

    virtual T visitPrintStmt(const PrintStmt<T>& stmt) = 0;

    virtual T visitVarStmt(const VarStmt<T>& stmt) = 0;
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
