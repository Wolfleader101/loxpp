#pragma once
#include <string>
#include <vector>
#include <memory>

#include "LoxType.hpp"
#include "Token.hpp"

template <typename T>
class StmtVisitor;

template <typename T>
class Stmt {
public:
   virtual ~Stmt() = default;
    virtual T accept(StmtVisitor<T>& visitor) const = 0;
};

template<typename T>
    class ExpressionStmt;

template<typename T>
    class PrintStmt;

template <typename T>
class StmtVisitor
{
public:
    virtual ~StmtVisitor() = default;
    virtual T visitExpressionStmt(const ExpressionStmt<T>& stmt) = 0;

    virtual T visitPrintStmt(const PrintStmt<T>& stmt) = 0;

};
template<typename T>
class ExpressionStmt : public Stmt<T>
{
public:
    ExpressionStmt(std::shared_ptr<Expr<T>> expression)
        : expression(expression)
    {
    }
    T accept(StmtVisitor<T>& visitor) const override
    {
        return visitor.visitExpressionStmt(*this);
    }
    std::shared_ptr<Expr<T>> expression;
};

template<typename T>
class PrintStmt : public Stmt<T>
{
public:
    PrintStmt(std::shared_ptr<Expr<T>> expressionVar        : Token name, std::shared_ptr<Expr<T>> initializer)
        :        : Token name(       : Token name), initializer(initializer)
    {
    }
    T accept(StmtVisitor<T>& visitor) const override
    {
        return visitor.visitPrintStmt(*this);
    }
    std::shared_ptr<Expr<T>> expressionVar        : Token name;
    std::shared_ptr<Expr<T>> initializer;
};

