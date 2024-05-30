#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Expr.hpp"
#include "ILogger.hpp"
#include "Stmt.hpp"

enum class FunctionType
{
    NONE,
    FUNCTION,
    INITIALIZER,
    METHOD
};

class Resolver : public ExprVisitor<LoxTypeRef>, public StmtVisitor<LoxTypeRef>
{
  public:
    Resolver(ILogger& logger, Interpreter& interpreter);

    void resolve(const std::vector<std::shared_ptr<Stmt<LoxTypeRef>>>& statements);

    LoxTypeRef visitBlockStmt(const BlockStmt<LoxTypeRef>& stmt) override;

    LoxTypeRef visitExpressionStmt(const ExpressionStmt<LoxTypeRef>& stmt) override;

    LoxTypeRef visitFunctionStmt(const FunctionStmt<LoxTypeRef>& stmt) override;

    LoxTypeRef visitIfStmt(const IfStmt<LoxTypeRef>& stmt) override;

    LoxTypeRef visitPrintStmt(const PrintStmt<LoxTypeRef>& stmt) override;

    LoxTypeRef visitReturnStmt(const ReturnStmt<LoxTypeRef>& stmt) override;

    LoxTypeRef visitVarStmt(const VarStmt<LoxTypeRef>& stmt) override;

    LoxTypeRef visitWhileStmt(const WhileStmt<LoxTypeRef>& stmt) override;

    LoxTypeRef visitAssignExpr(const AssignExpr<LoxTypeRef>& expr) override;

    LoxTypeRef visitBinaryExpr(const BinaryExpr<LoxTypeRef>& expr) override;

    LoxTypeRef visitCallExpr(const CallExpr<LoxTypeRef>& expr) override;

    LoxTypeRef visitGroupingExpr(const GroupingExpr<LoxTypeRef>& expr) override;

    LoxTypeRef visitLiteralExpr(const LiteralExpr<LoxTypeRef>& expr) override;

    LoxTypeRef visitLogicalExpr(const LogicalExpr<LoxTypeRef>& expr) override;

    LoxTypeRef visitUnaryExpr(const UnaryExpr<LoxTypeRef>& expr) override;

    LoxTypeRef visitVariableExpr(const VariableExpr<LoxTypeRef>& expr) override;

  private:
    ILogger& logger;
    Interpreter& interpreter;
    std::vector<std::unordered_map<std::string, bool>> scopes;
    FunctionType currentFunction = FunctionType::NONE;

    void resolve(std::shared_ptr<Stmt<LoxTypeRef>> statement);
    void resolve(std::shared_ptr<Expr<LoxTypeRef>> expr);

    void beginScope();
    void endScope();

    void declare(const Token& name);
    void define(const Token& name);

    void resolveLocal(const Expr<LoxTypeRef>& expr, const Token& name);
    void resolveFunction(const FunctionStmt<LoxTypeRef>& stmt, FunctionType type);
};
