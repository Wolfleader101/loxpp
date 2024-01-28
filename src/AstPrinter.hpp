#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Expr.hpp"

class AstPrinter : public ExprVisitor<std::string>
{
  public:
    std::string print(std::shared_ptr<Expr<std::string>> expr)
    {
        return expr->accept(*this);
    }

    std::string visitBinaryExpr(const BinaryExpr<std::string>& expr) override
    {
        return parenthesize(expr.op.lexeme, {expr.left, expr.right});
    }

    std::string visitGroupingExpr(const GroupingExpr<std::string>& expr) override
    {
        return parenthesize("group", {expr.expression});
    }

    std::string visitLiteralExpr(const LiteralExpr<std::string>& expr) override
    {
        return LoxTypeToString(expr.value);
    }

    std::string visitUnaryExpr(const UnaryExpr<std::string>& expr) override
    {
        return parenthesize(expr.op.lexeme, {expr.right});
    }

  private:
    std::string parenthesize(const std::string& name, std::vector<std::shared_ptr<Expr<std::string>>> exprs)
    {
        std::string builder = "(" + name;

        for (const auto& expr : exprs)
        {
            builder += " " + print(expr);
        }

        builder += ")";

        return builder;
    }
};
