#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Expr.hpp"

class AstPrinter : public ExprVisitor<std::string>
{
  public:
    std::string print(std::shared_ptr<Expr<std::string>> expr);

    std::string visitBinaryExpr(const BinaryExpr<std::string>& expr) override;
    std::string visitGroupingExpr(const GroupingExpr<std::string>& expr) override;
    std::string visitLiteralExpr(const LiteralExpr<std::string>& expr) override;
    std::string visitUnaryExpr(const UnaryExpr<std::string>& expr) override;

  private:
    std::string parenthesize(const std::string& name, std::vector<std::shared_ptr<Expr<std::string>>> exprs);
};
