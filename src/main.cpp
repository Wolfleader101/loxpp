#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "ILogger.hpp"
#include "Token.hpp"

#include "Scanner.hpp"

#include "Expr.hpp"

class ParseException : public std::exception
{
  public:
    ParseException(const std::string& message) : message(message)
    {
    }

    const char* what() const noexcept override
    {
        return message.c_str();
    }

  private:
    std::string message;
};

class Parser
{
  public:
    Parser(const std::vector<Token>& tokens, ILogger& logger) : tokens(tokens), logger(logger)
    {
    }

    std::shared_ptr<Expr<std::string>> parse()
    {
        try
        {
            return expression();
        }
        catch (ParseException& exception)
        {
            return nullptr;
        }
    }

  private:
    const std::vector<Token>& tokens;
    ILogger& logger;
    int current = 0;

    bool match(std::vector<TokenType> types)
    {
        for (TokenType type : types)
        {
            if (check(type))
            {
                advance();
                return true;
            }
        }

        return false;
    }

    bool check(TokenType type)
    {
        if (isAtEnd())
        {
            return false;
        }

        return peek().type == type;
    }

    Token advance()
    {
        if (!isAtEnd())
        {
            current++;
        }

        return previous();
    }

    bool isAtEnd()
    {
        return peek().type == TokenType::END_OF_FILE;
    }

    Token peek()
    {
        return tokens[current];
    }

    Token previous()
    {
        return tokens[current - 1];
    }

    std::shared_ptr<Expr<std::string>> expression()
    {
        return equality();
    }

    std::shared_ptr<Expr<std::string>> equality()
    {
        std::shared_ptr<Expr<std::string>> expr = comparison();

        while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL}))
        {
            Token op = previous();
            std::shared_ptr<Expr<std::string>> right = comparison();
            expr = std::make_shared<BinaryExpr<std::string>>(expr, op, right);
        }

        return expr;
    }

    std::shared_ptr<Expr<std::string>> comparison()
    {
        std::shared_ptr<Expr<std::string>> expr = term();

        while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL}))
        {
            Token op = previous();
            std::shared_ptr<Expr<std::string>> right = term();
            expr = std::make_shared<BinaryExpr<std::string>>(expr, op, right);
        }

        return expr;
    }

    std::shared_ptr<Expr<std::string>> term()
    {
        std::shared_ptr<Expr<std::string>> expr = factor();

        while (match({TokenType::MINUS, TokenType::PLUS}))
        {
            Token op = previous();
            std::shared_ptr<Expr<std::string>> right = factor();
            expr = std::make_shared<BinaryExpr<std::string>>(expr, op, right);
        }

        return expr;
    }

    std::shared_ptr<Expr<std::string>> factor()
    {
        std::shared_ptr<Expr<std::string>> expr = unary();

        while (match({TokenType::SLASH, TokenType::STAR}))
        {
            Token op = previous();
            std::shared_ptr<Expr<std::string>> right = unary();
            expr = std::make_shared<BinaryExpr<std::string>>(expr, op, right);
        }

        return expr;
    }

    std::shared_ptr<Expr<std::string>> unary()
    {
        if (match({TokenType::BANG, TokenType::MINUS}))
        {
            Token op = previous();
            std::shared_ptr<Expr<std::string>> right = unary();
            return std::make_shared<UnaryExpr<std::string>>(op, right);
        }

        return primary();
    }

    std::shared_ptr<Expr<std::string>> primary()
    {
        if (match({TokenType::FALSE}))
            return std::make_shared<LiteralExpr<std::string>>("false");

        if (match({TokenType::TRUE}))
            return std::make_shared<LiteralExpr<std::string>>("true");

        if (match({TokenType::NIL}))
            return std::make_shared<LiteralExpr<std::string>>("nil");

        if (match({TokenType::NUMBER, TokenType::STRING}))
            return std::make_shared<LiteralExpr<std::string>>(previous().literal);

        if (match({TokenType::LEFT_PAREN}))
        {
            std::shared_ptr<Expr<std::string>> expr = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
            return std::make_shared<GroupingExpr<std::string>>(expr);
        }

        throw error(peek(), "Expect expression.");
    }

    Token consume(TokenType type, const std::string& message)
    {
        if (check(type))
        {
            return advance();
        }

        throw error(peek(), message);
    }

    ParseException error(Token token, const std::string& message)
    {
        if (token.type == TokenType::END_OF_FILE)
        {
            logger.LogError(token.line, " at end", message);
        }
        else
        {
            logger.LogError(token.line, " at '" + token.lexeme + "'", message);
        }

        return ParseException(message);
    }

    void synchronize()
    {
        advance();

        while (!isAtEnd())
        {
            if (previous().type == TokenType::SEMICOLON)
                return;

            switch (peek().type)
            {
            case TokenType::CLASS:
            case TokenType::FUN:
            case TokenType::VAR:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN:
                return;
            }

            advance();
        }
    }
};

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
        return expr.value;
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

void runCode(std::string& code);

void runFile(const char* path);
void runPrompt();

void reportError(int line, const std::string& where, const std::string& message);

static bool hadError = false;

class LoxppLogger : public ILogger
{
  public:
    void LogError(int line, const std::string& message) override
    {
        reportError(line, "", message);
    }
    void LogError(int line, const std::string& where, const std::string& message) override
    {
        reportError(line, where, message);
    }
};

static LoxppLogger logger;

int main(int argc, char** argv)
{

    if (argc > 2)
    {
        std::cout << "Usage: loxpp [script]" << std::endl;
        exit(64);
    }
    else if (argc == 2)
    {
        runFile(argv[1]);
    }
    else
    {
        runPrompt();
    }

    return 0;
}

void runFile(const char* path)
{
    std::ifstream file_stream(path);

    if (file_stream.is_open())
    {
        std::string file_contents((std::istreambuf_iterator<char>(file_stream)), (std::istreambuf_iterator<char>()));
        file_stream.close();

        runCode(file_contents);

        if (hadError)
        {
            exit(65);
        }
    }
    else
    {
        std::cerr << "Error: Unable to open the file." << std::endl;
    }
}

void runPrompt()
{
    for (;;)
    {
        std::cout << "> ";
        std::string line;
        std::getline(std::cin, line);

        if (std::cin.eof())
        {
            std::cout << std::endl;
            break;
        }

        runCode(line);
        hadError = false;
    }
}

void runCode(std::string& code)
{
    Scanner scanner = Scanner(code, logger);
    std::vector<Token> tokens = scanner.scanTokens();
    Parser parser = Parser(tokens, logger);

    std::shared_ptr<Expr<std::string>> expression = parser.parse();

    if (expression == nullptr)
    {
        return;
    }

    AstPrinter printer;
    std::cout << printer.print(expression) << std::endl;
}

void reportError(int line, const std::string& where, const std::string& message)
{
    hadError = true;
    std::cerr << "[line " << line << "] Error" << where << ": " << message << std::endl;
}