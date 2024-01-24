#include <string>
#include <vector>

#include "Token.hpp"

template <typename T>
class Visitor;

template <typename T>
class Expr
{
  public:
    virtual ~Expr() = default;

    virtual T accept(Visitor<T>& visitor) const = 0;
};

template <typename T>
class Binary;

template <typename T>
class Grouping;

template <typename T>
class Literal;

template <typename T>
class Unary;

template <typename T>
class Visitor
{
  public:
    virtual ~Visitor() = default;
    virtual T visitBinaryExpr(const Binary<T>& expr) = 0;

    virtual T visitGroupingExpr(const Grouping<T>& expr) = 0;

    virtual T visitLiteralExpr(const Literal<T>& expr) = 0;

    virtual T visitUnaryExpr(const Unary<T>& expr) = 0;
};

template <typename T>
class Binary : public Expr<T>
{
  public:
    Binary(const Expr<T>& left, Token op, const Expr<T>& right) : left(left), op(op), right(right)
    {
    }
    T accept(Visitor<T>& visitor) const override
    {
        return visitor.visitBinaryExpr(*this);
    }
    const Expr<T>& left;
    Token op;
    const Expr<T>& right;
};

template <typename T>
class Grouping : public Expr<T>
{
  public:
    Grouping(const Expr<T>& expression) : expression(expression)
    {
    }
    T accept(Visitor<T>& visitor) const override
    {
        return visitor.visitGroupingExpr(*this);
    }
    const Expr<T>& expression;
};

template <typename T>
class Literal : public Expr<T>
{
  public:
    Literal(std::string value) : value(value)
    {
    }
    T accept(Visitor<T>& visitor) const override
    {
        return visitor.visitLiteralExpr(*this);
    }
    std::string value;
};

template <typename T>
class Unary : public Expr<T>
{
  public:
    Unary(Token op, const Expr<T>& right) : op(op), right(right)
    {
    }
    T accept(Visitor<T>& visitor) const override
    {
        return visitor.visitUnaryExpr(*this);
    }
    Token op;
    const Expr<T>& right;
};