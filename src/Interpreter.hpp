#pragma once

#include <memory>
#include <vector>

#include "Environment.hpp"
#include "Expr.hpp"
#include "ILogger.hpp"
#include "LoxType.hpp"
#include "Stmt.hpp"

class Interpreter : public ExprVisitor<LoxType>, public StmtVisitor<LoxType>
{
  public:
    Interpreter(ILogger& logger);

    void interpret(std::vector<std::shared_ptr<Stmt<LoxType>>> statements);

    LoxType visitLiteralExpr(const LiteralExpr<LoxType>& expr) override;

    LoxType visitGroupingExpr(const GroupingExpr<LoxType>& expr) override;

    LoxType visitUnaryExpr(const UnaryExpr<LoxType>& expr) override;

    LoxType visitBinaryExpr(const BinaryExpr<LoxType>& expr) override;

    LoxType visitVariableExpr(const VariableExpr<LoxType>& expr) override;

    LoxType visitAssignExpr(const AssignExpr<LoxType>& expr) override;

    LoxType visitExpressionStmt(const ExpressionStmt<LoxType>& stmt) override;

    LoxType visitPrintStmt(const PrintStmt<LoxType>& stmt) override;

    LoxType visitVarStmt(const VarStmt<LoxType>& stmt) override;

    LoxType visitBlockStmt(const BlockStmt<LoxType>& stmt) override;

  private:
    ILogger& logger;
    std::shared_ptr<Environment> environment;

    void execute(std::shared_ptr<Stmt<LoxType>> stmt);
    LoxType evaluate(std::shared_ptr<Expr<LoxType>> expr);

    bool isTruthy(const LoxType& object);

    bool isEqual(const LoxType& a, const LoxType& b);

    void checkNumberOperands(const Token& op, const LoxType& operand);

    void checkNumberOperands(const Token& op, const LoxType& left, const LoxType& right);

    void executeBlock(const std::vector<std::shared_ptr<Stmt<LoxType>>>& statements,
                      std::shared_ptr<Environment> environment);
};
