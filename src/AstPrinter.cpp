#include "AstPrinter.hpp"

std::string AstPrinter::print(std::shared_ptr<Expr<std::string>> expr)
{
    return expr->accept(*this);
}

std::string AstPrinter::visitBinaryExpr(const BinaryExpr<std::string>& expr)
{
    return parenthesize(expr.op.lexeme, {expr.left, expr.right});
}

std::string AstPrinter::visitGroupingExpr(const GroupingExpr<std::string>& expr)
{
    return parenthesize("group", {expr.expression});
}

std::string AstPrinter::visitLiteralExpr(const LiteralExpr<std::string>& expr)
{
    return LoxTypeToString(*expr.value);
}

std::string AstPrinter::visitUnaryExpr(const UnaryExpr<std::string>& expr)
{
    return parenthesize(expr.op.lexeme, {expr.right});
}

std::string AstPrinter::parenthesize(const std::string& name, std::vector<std::shared_ptr<Expr<std::string>>> exprs)
{
    std::string builder = "(" + name;

    for (const auto& expr : exprs)
    {
        builder += " " + print(expr);
    }

    builder += ")";

    return builder;
}