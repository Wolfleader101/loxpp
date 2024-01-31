#pragma once

#include <exception>
#include <memory>
#include <string>
#include <vector>

#include "Expr.hpp"
#include "ILogger.hpp"
#include "LoxType.hpp"
#include "Stmt.hpp"
#include "Token.hpp"

class ParseException : public std::exception
{
  public:
    ParseException(const std::string& message);

    const char* what() const noexcept override;

  private:
    std::string message;
};

class Parser
{
  public:
    Parser(const std::vector<Token>& tokens, ILogger& logger);

    std::vector<std::shared_ptr<Stmt<LoxTypeRef>>> parse();

  private:
    const std::vector<Token>& tokens;
    ILogger& logger;
    int current = 0;

    bool match(std::vector<TokenType> types);

    bool check(TokenType type);

    Token advance();

    bool isAtEnd();

    Token peek();

    Token previous();

    std::shared_ptr<Stmt<LoxTypeRef>> declaration();

    std::shared_ptr<Stmt<LoxTypeRef>> varDeclaration();

    std::shared_ptr<Stmt<LoxTypeRef>> statement();

    std::shared_ptr<Stmt<LoxTypeRef>> block();

    std::shared_ptr<Stmt<LoxTypeRef>> ifStatement();

    std::shared_ptr<Stmt<LoxTypeRef>> printStatement();

    std::shared_ptr<Stmt<LoxTypeRef>> expressionStatement();

    std::shared_ptr<Stmt<LoxTypeRef>> whileStatement();

    std::shared_ptr<Stmt<LoxTypeRef>> forStatement();

    std::shared_ptr<Expr<LoxTypeRef>> assignment();

    std::shared_ptr<Expr<LoxTypeRef>> expression();

    std::shared_ptr<Expr<LoxTypeRef>> orExpr();

    std::shared_ptr<Expr<LoxTypeRef>> andExpr();

    std::shared_ptr<Expr<LoxTypeRef>> equality();

    std::shared_ptr<Expr<LoxTypeRef>> comparison();

    std::shared_ptr<Expr<LoxTypeRef>> term();

    std::shared_ptr<Expr<LoxTypeRef>> factor();

    std::shared_ptr<Expr<LoxTypeRef>> unary();

    std::shared_ptr<Expr<LoxTypeRef>> call();

    std::shared_ptr<Expr<LoxTypeRef>> finishCall(std::shared_ptr<Expr<LoxTypeRef>> callee);

    std::shared_ptr<Expr<LoxTypeRef>> primary();

    Token consume(TokenType type, const std::string& message);

    ParseException error(Token token, const std::string& message);

    void synchronize();
};
