#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "ILogger.hpp"
#include "Token.hpp"

#include "Scanner.hpp"

#include "Expr.hpp"

#include "RuntimeError.hpp"

class Interpreter : public ExprVisitor<LoxType>
{
  public:
    Interpreter(ILogger& logger) : logger(logger)
    {
    }
    void interpret(std::shared_ptr<Expr<LoxType>> expr)
    {
        try
        {
            LoxType value = evaluate(expr);
            std::cout << LoxTypeToString(value) << std::endl;
        }
        catch (RuntimeError& error)
        {
            logger.LogRuntimeError(error);
        }
    }

    LoxType visitLiteralExpr(const LiteralExpr<LoxType>& expr) override
    {
        return expr.value;
    }

    LoxType visitGroupingExpr(const GroupingExpr<LoxType>& expr) override
    {
        return evaluate(expr.expression);
    }

    LoxType visitUnaryExpr(const UnaryExpr<LoxType>& expr) override
    {
        LoxType right = evaluate(expr.right);

        switch (expr.op.type)
        {
        case TokenType::BANG:
            return !isTruthy(right);
        case TokenType::MINUS:
            checkNumberOperands(expr.op, right);
            return -std::get<double>(right.value());
        }

        return LoxType();
    }

    LoxType visitBinaryExpr(const BinaryExpr<LoxType>& expr) override
    {
        LoxType left = evaluate(expr.left);
        LoxType right = evaluate(expr.right);

        switch (expr.op.type)
        {
        case TokenType::GREATER:
            checkNumberOperands(expr.op, left, right);
            return std::get<double>(left.value()) > std::get<double>(right.value());
        case TokenType::GREATER_EQUAL:
            checkNumberOperands(expr.op, left, right);
            return std::get<double>(left.value()) >= std::get<double>(right.value());
        case TokenType::LESS:
            checkNumberOperands(expr.op, left, right);
            return std::get<double>(left.value()) < std::get<double>(right.value());
        case TokenType::LESS_EQUAL:
            checkNumberOperands(expr.op, left, right);
            return std::get<double>(left.value()) <= std::get<double>(right.value());
        case TokenType::BANG_EQUAL:
            return !isEqual(left, right);
        case TokenType::EQUAL_EQUAL:
            return isEqual(left, right);
        case TokenType::MINUS:
            checkNumberOperands(expr.op, left, right);
            return std::get<double>(left.value()) - std::get<double>(right.value());
        case TokenType::PLUS:
            if (IsDouble(left) && IsDouble(right))
            {
                return std::get<double>(left.value()) + std::get<double>(right.value());
            }

            if (IsString(left) && IsString(right))
            {
                return std::get<std::string>(left.value()) + std::get<std::string>(right.value());
            }

            throw RuntimeError(expr.op, "Operands must be two numbers or two strings.");
            break;
        case TokenType::SLASH:
            checkNumberOperands(expr.op, left, right);
            return std::get<double>(left.value()) / std::get<double>(right.value());
        case TokenType::STAR:
            checkNumberOperands(expr.op, left, right);
            return std::get<double>(left.value()) * std::get<double>(right.value());
        }

        return LoxType();
    }

  private:
    ILogger& logger;

    LoxType evaluate(std::shared_ptr<Expr<LoxType>> expr)
    {
        return expr->accept(*this);
    }

    bool isTruthy(const LoxType& object)
    {
        if (!object.has_value())
        {
            return false;
        }

        return std::visit(
            [](const auto& value) -> bool {
                using T = std::decay_t<decltype(value)>;

                if constexpr (std::is_same_v<T, bool>)
                {
                    return value;
                }
                else
                {
                    return true;
                }
            },
            object.value());
    }

    bool isEqual(const LoxType& a, const LoxType& b)
    {
        if (!a.has_value() && !b.has_value())
        {
            return true;
        }

        if (!a.has_value() || !b.has_value())
        {
            return false;
        }

        return std::visit(
            [&b](const auto& aValue) -> bool {
                return std::visit(
                    [&aValue](const auto& bValue) -> bool {
                        using Ta = std::decay_t<decltype(aValue)>;
                        using Tb = std::decay_t<decltype(bValue)>;

                        // if the types are different, they are not equal
                        if constexpr (!std::is_same_v<Ta, Tb>)
                        {
                            return false;
                        }
                        else
                        {
                            return aValue == bValue;
                        }
                    },
                    b.value());
            },
            a.value());
    }

    void checkNumberOperands(const Token& op, const LoxType& operand)
    {
        if (IsDouble(operand))
            return;

        throw RuntimeError(op, "Operand must be a number.");
    }

    void checkNumberOperands(const Token& op, const LoxType& left, const LoxType& right)
    {
        if (IsDouble(left) && IsDouble(right))
            return;

        throw RuntimeError(op, "Operands must be numbers.");
    }
};

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

    std::shared_ptr<Expr<LoxType>> parse()
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

    std::shared_ptr<Expr<LoxType>> expression()
    {
        return equality();
    }

    std::shared_ptr<Expr<LoxType>> equality()
    {
        std::shared_ptr<Expr<LoxType>> expr = comparison();

        while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL}))
        {
            Token op = previous();
            std::shared_ptr<Expr<LoxType>> right = comparison();
            expr = std::make_shared<BinaryExpr<LoxType>>(expr, op, right);
        }

        return expr;
    }

    std::shared_ptr<Expr<LoxType>> comparison()
    {
        std::shared_ptr<Expr<LoxType>> expr = term();

        while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL}))
        {
            Token op = previous();
            std::shared_ptr<Expr<LoxType>> right = term();
            expr = std::make_shared<BinaryExpr<LoxType>>(expr, op, right);
        }

        return expr;
    }

    std::shared_ptr<Expr<LoxType>> term()
    {
        std::shared_ptr<Expr<LoxType>> expr = factor();

        while (match({TokenType::MINUS, TokenType::PLUS}))
        {
            Token op = previous();
            std::shared_ptr<Expr<LoxType>> right = factor();
            expr = std::make_shared<BinaryExpr<LoxType>>(expr, op, right);
        }

        return expr;
    }

    std::shared_ptr<Expr<LoxType>> factor()
    {
        std::shared_ptr<Expr<LoxType>> expr = unary();

        while (match({TokenType::SLASH, TokenType::STAR}))
        {
            Token op = previous();
            std::shared_ptr<Expr<LoxType>> right = unary();
            expr = std::make_shared<BinaryExpr<LoxType>>(expr, op, right);
        }

        return expr;
    }

    std::shared_ptr<Expr<LoxType>> unary()
    {
        if (match({TokenType::BANG, TokenType::MINUS}))
        {
            Token op = previous();
            std::shared_ptr<Expr<LoxType>> right = unary();
            return std::make_shared<UnaryExpr<LoxType>>(op, right);
        }

        return primary();
    }

    std::shared_ptr<Expr<LoxType>> primary()
    {
        if (match({TokenType::FALSE}))
            return std::make_shared<LiteralExpr<LoxType>>("false");

        if (match({TokenType::TRUE}))
            return std::make_shared<LiteralExpr<LoxType>>("true");

        if (match({TokenType::NIL}))
            return std::make_shared<LiteralExpr<LoxType>>("nil");

        if (match({TokenType::NUMBER, TokenType::STRING}))
            return std::make_shared<LiteralExpr<LoxType>>(previous().literal);

        if (match({TokenType::LEFT_PAREN}))
        {
            std::shared_ptr<Expr<LoxType>> expr = expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
            return std::make_shared<GroupingExpr<LoxType>>(expr);
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

void runCode(std::string& code);

void runFile(const char* path);
void runPrompt();

void reportError(int line, const std::string& where, const std::string& message);

static bool hadError = false;
static bool hadRuntimeError = false;

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

    void LogRuntimeError(const RuntimeError& error) override
    {
        std::cerr << error.what() << std::endl;
        std::cerr << "[line " << error.token.line << "] " << error.message << std::endl;
        hadRuntimeError = true;
    }
};

static LoxppLogger logger;
static Interpreter interpreter(logger);

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
            exit(65);

        if (hadRuntimeError)
            exit(70);
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

    std::shared_ptr<Expr<LoxType>> expression = parser.parse();

    if (expression == nullptr)
    {
        return;
    }

    if (hadError)
        return;

    interpreter.interpret(expression);

    // AstPrinter printer;
    // std::cout << printer.print(expression) << std::endl;
}

void reportError(int line, const std::string& where, const std::string& message)
{
    hadError = true;
    std::cerr << "[line " << line << "] Error" << where << ": " << message << std::endl;
}