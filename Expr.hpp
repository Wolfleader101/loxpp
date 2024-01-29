#pragma once
#include <string>
#include <vector>
#include <memory>

#include "LoxType.hpp"
#include "Token.hpp"

template <typename T>
class ExprVisitor;

template <typename T>
class Expr {
public:
   virtual ~Expr() = default;
    virtual T accept(ExprVisitor<T>& visitor) const = 0;
};

template<typename T>
    class BinaryExpr;

template<typename T>
    class GroupingExpr;

template<typename T>
    class LiteralExpr;

template<typename T>
    class UnaryExpr;

template <typename T>
class ExprVisitor
{
public:
    virtual ~ExprVisitor() = default;
    virtual T visitBinaryExpr(const BinaryExpr<T>& expr) = 0;

    virtual T visitGroupingExpr(const GroupingExpr<T>& expr) = 0;

    virtual T visitLiteralExpr(const LiteralExpr<T>& expr) = 0;

    virtual T visitUnaryExpr(const UnaryExpr<T>& expr) = 0;

};
template<typename T>
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

template<typename T>
class GroupingExpr : public Expr<T>
{
public:
    GroupingExpr(std::shared_ptr<Expr<T>> expression)
        : expression(expression)
    {
    }
    T accept(ExprVisitor<T>& visitor) const override
    {
        return visitor.visitGroupingExpr(*this);
    }
    std::shared_ptr<Expr<T>> expression;
};

template<typename T>
class LiteralExpr : public Expr<T>
{
public:
    LiteralExpr(LoxType value)
        : value(value)
    {
    }
    T accept(ExprVisitor<T>& visitor) const override
    {
        return visitor.visitLiteralExpr(*this);
    }
    LoxType value;
};

template<typename T>
class UnaryExpr : public Expr<T>
{
public:
    UnaryExpr(Token op, std::shared_ptr<Expr<T>> right)
        : op(op), right(right)
    {
    }
    T accept(ExprVisitor<T>& visitor) const override
    {
        return visitor.visitUnaryExpr(*this);
    }
    Token op;
    std::shared_ptr<Expr<T>> right;
};

