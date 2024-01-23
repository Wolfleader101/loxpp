#include <string>
#include <vector>

#include "Token.hpp"

class Expr
{
};
class Binary : public Expr
{
  public:
    Binary(Expr left, Token op, Expr right) : left(left), op(op), right(right)
    {
    }
    const Expr left;
    const Token op;
    const Expr right;
};

class Grouping : public Expr
{
  public:
    Grouping(Expr expression) : expression(expression)
    {
    }
    const Expr expression;
};

class Literal : public Expr
{
  public:
    Literal(std::string value) : value(value)
    {
    }
    const std::string value;
};

class Unary : public Expr
{
  public:
    Unary(Token op, Expr right) : op(op), right(right)
    {
    }
    const Token op;
    const Expr right;
};
