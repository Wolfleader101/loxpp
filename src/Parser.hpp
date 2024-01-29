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

    std::vector<std::shared_ptr<Stmt<LoxType>>> parse();

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

    std::shared_ptr<Stmt<LoxType>> declaration();

    std::shared_ptr<Stmt<LoxType>> varDeclaration();

    std::shared_ptr<Stmt<LoxType>> statement();

    std::shared_ptr<Stmt<LoxType>> block();

    std::shared_ptr<Stmt<LoxType>> printStatement();

    std::shared_ptr<Stmt<LoxType>> expressionStatement();

    std::shared_ptr<Expr<LoxType>> assignment();

    std::shared_ptr<Expr<LoxType>> expression();

    std::shared_ptr<Expr<LoxType>> equality();

    std::shared_ptr<Expr<LoxType>> comparison();

    std::shared_ptr<Expr<LoxType>> term();

    std::shared_ptr<Expr<LoxType>> factor();

    std::shared_ptr<Expr<LoxType>> unary();

    std::shared_ptr<Expr<LoxType>> primary();

    Token consume(TokenType type, const std::string& message);

    ParseException error(Token token, const std::string& message);

    void synchronize();
};
