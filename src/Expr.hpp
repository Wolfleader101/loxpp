#pragma once
#include <memory>
#include <string>
#include <vector>

#include "LoxType.hpp"
#include "Token.hpp"

template <typename T>
class ExprVisitor;

template <typename T>
class Expr
{
  public:
    Expr() : id(nextId++)
    {
    }

    virtual ~Expr() = default;
    virtual T accept(ExprVisitor<T>& visitor) const = 0;

    std::size_t getId() const
    {
        return id;
    }

  private:
    static std::size_t nextId;
    std::size_t id;
};

template <typename T>
std::size_t Expr<T>::nextId = 0;

template <typename T>
class AssignExpr;

template <typename T>
class BinaryExpr;

template <typename T>
class CallExpr;

template <typename T>
class GroupingExpr;

template <typename T>
class LiteralExpr;

template <typename T>
class LogicalExpr;

template <typename T>
class UnaryExpr;

template <typename T>
class VariableExpr;

template <typename T>
class ExprVisitor
{
  public:
    virtual ~ExprVisitor() = default;
    virtual T visitAssignExpr(const AssignExpr<T>& expr) = 0;

    virtual T visitBinaryExpr(const BinaryExpr<T>& expr) = 0;

    virtual T visitCallExpr(const CallExpr<T>& expr) = 0;

    virtual T visitGroupingExpr(const GroupingExpr<T>& expr) = 0;

    virtual T visitLiteralExpr(const LiteralExpr<T>& expr) = 0;

    virtual T visitLogicalExpr(const LogicalExpr<T>& expr) = 0;

    virtual T visitUnaryExpr(const UnaryExpr<T>& expr) = 0;

    virtual T visitVariableExpr(const VariableExpr<T>& expr) = 0;
};
template <typename T>
class AssignExpr : public Expr<T>
{
  public:
    AssignExpr(Token name, std::shared_ptr<Expr<T>> value) : name(name), value(value)
    {
    }
    T accept(ExprVisitor<T>& visitor) const override
    {
        return visitor.visitAssignExpr(*this);
    }
    Token name;
    std::shared_ptr<Expr<T>> value;
};

template <typename T>
class BinaryExpr : public Expr<T>
{
  public:
    BinaryExpr(std::shared_ptr<Expr<T>> left, Token op, std::shared_ptr<Expr<T>> right)
        : left(left), op(op), right(right)
    {
    }
    T accept(ExprVisitor<T>& visitor) const override
    {
        return visitor.visitBinaryExpr(*this);
    }
    std::shared_ptr<Expr<T>> left;
    Token op;
    std::shared_ptr<Expr<T>> right;
};

template <typename T>
class CallExpr : public Expr<T>
{
  public:
    CallExpr(std::shared_ptr<Expr<T>> callee, Token paren, std::vector<std::shared_ptr<Expr<T>>> arguments)
        : callee(callee), paren(paren), arguments(arguments)
    {
    }
    T accept(ExprVisitor<T>& visitor) const override
    {
        return visitor.visitCallExpr(*this);
    }
    std::shared_ptr<Expr<T>> callee;
    Token paren;
    std::vector<std::shared_ptr<Expr<T>>> arguments;
};

template <typename T>
class GroupingExpr : public Expr<T>
{
  public:
    GroupingExpr(std::shared_ptr<Expr<T>> expression) : expression(expression)
    {
    }
    T accept(ExprVisitor<T>& visitor) const override
    {
        return visitor.visitGroupingExpr(*this);
    }
    std::shared_ptr<Expr<T>> expression;
};

template <typename T>
class LiteralExpr : public Expr<T>
{
  public:
    LiteralExpr(LoxTypeRef value) : value(value)
    {
    }
    T accept(ExprVisitor<T>& visitor) const override
    {
        return visitor.visitLiteralExpr(*this);
    }
    LoxTypeRef value;
};

template <typename T>
class LogicalExpr : public Expr<T>
{
  public:
    LogicalExpr(std::shared_ptr<Expr<T>> left, Token op, std::shared_ptr<Expr<T>> right)
        : left(left), op(op), right(right)
    {
    }
    T accept(ExprVisitor<T>& visitor) const override
    {
        return visitor.visitLogicalExpr(*this);
    }
    std::shared_ptr<Expr<T>> left;
    Token op;
    std::shared_ptr<Expr<T>> right;
};

template <typename T>
class UnaryExpr : public Expr<T>
{
  public:
    UnaryExpr(Token op, std::shared_ptr<Expr<T>> right) : op(op), right(right)
    {
    }
    T accept(ExprVisitor<T>& visitor) const override
    {
        return visitor.visitUnaryExpr(*this);
    }
    Token op;
    std::shared_ptr<Expr<T>> right;
};

template <typename T>
class VariableExpr : public Expr<T>
{
  public:
    VariableExpr(Token name) : name(name)
    {
    }
    T accept(ExprVisitor<T>& visitor) const override
    {
        return visitor.visitVariableExpr(*this);
    }
    Token name;
};
