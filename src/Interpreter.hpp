#pragma once

#include <memory>
#include <vector>

#include "Environment.hpp"
#include "Expr.hpp"
#include "ILogger.hpp"
#include "LoxType.hpp"
#include "Stmt.hpp"

class Interpreter : public ExprVisitor<LoxTypeRef>, public StmtVisitor<LoxTypeRef>
{
  public:
    Interpreter(ILogger& logger);

    void interpret(std::vector<std::shared_ptr<Stmt<LoxTypeRef>>>& statements);

    LoxTypeRef visitLiteralExpr(const LiteralExpr<LoxTypeRef>& expr) override;

    LoxTypeRef visitGroupingExpr(const GroupingExpr<LoxTypeRef>& expr) override;

    LoxTypeRef visitUnaryExpr(const UnaryExpr<LoxTypeRef>& expr) override;

    LoxTypeRef visitCallExpr(const CallExpr<LoxTypeRef>& expr) override;

    LoxTypeRef visitBinaryExpr(const BinaryExpr<LoxTypeRef>& expr) override;

    LoxTypeRef visitVariableExpr(const VariableExpr<LoxTypeRef>& expr) override;

    LoxTypeRef visitAssignExpr(const AssignExpr<LoxTypeRef>& expr) override;

    LoxTypeRef visitLogicalExpr(const LogicalExpr<LoxTypeRef>& expr) override;

    LoxTypeRef visitExpressionStmt(const ExpressionStmt<LoxTypeRef>& stmt) override;

    LoxTypeRef visitFunctionStmt(const FunctionStmt<LoxTypeRef>& stmt) override;

    LoxTypeRef visitPrintStmt(const PrintStmt<LoxTypeRef>& stmt) override;

    LoxTypeRef visitReturnStmt(const ReturnStmt<LoxTypeRef>& stmt) override;

    LoxTypeRef visitVarStmt(const VarStmt<LoxTypeRef>& stmt) override;

    LoxTypeRef visitBlockStmt(const BlockStmt<LoxTypeRef>& stmt) override;

    LoxTypeRef visitIfStmt(const IfStmt<LoxTypeRef>& stmt) override;

    LoxTypeRef visitWhileStmt(const WhileStmt<LoxTypeRef>& stmt) override;

    const std::shared_ptr<Environment> globals;

    void executeBlock(const std::vector<std::shared_ptr<Stmt<LoxTypeRef>>>& statements,
                      std::shared_ptr<Environment> environment);

  private:
    ILogger& logger;
    std::shared_ptr<Environment> environment;

    void execute(std::shared_ptr<Stmt<LoxTypeRef>> stmt);
    LoxTypeRef evaluate(std::shared_ptr<Expr<LoxTypeRef>> expr);

    bool isTruthy(const LoxType& object);

    bool isTruthy(LoxTypeRef object);

    bool isEqual(const LoxType& a, const LoxType& b);

    bool isEqual(LoxTypeRef a, LoxTypeRef b);

    void checkNumberOperands(const Token& op, const LoxType& operand);

    void checkNumberOperands(const Token& op, LoxTypeRef operand);

    void checkNumberOperands(const Token& op, const LoxType& left, const LoxType& right);

    void checkNumberOperands(const Token& op, LoxTypeRef left, LoxTypeRef right);
};
