#include <fstream>
#include <iostream>
#include <string>

#include "ILogger.hpp"
#include "Token.hpp"

#include "Scanner.hpp"

#include "Expr.hpp"

class AstPrinter : public Visitor<std::string>
{
  public:
    std::string print(const Expr<std::string>& expr)
    {
        return expr.accept(*this);
    }

    std::string visitBinaryExpr(const Binary<std::string>& expr) override
    {
        return parenthesize(expr.op.lexeme, {&expr.left, &expr.right});
    }

    std::string visitGroupingExpr(const Grouping<std::string>& expr) override
    {
        return parenthesize("group", {&expr.expression});
    }

    std::string visitLiteralExpr(const Literal<std::string>& expr) override
    {
        return expr.value;
    }

    std::string visitUnaryExpr(const Unary<std::string>& expr) override
    {
        return parenthesize(expr.op.lexeme, {&expr.right});
    }

  private:
    std::string parenthesize(const std::string& name, std::vector<const Expr<std::string>*> exprs)
    {
        std::string builder = "(" + name;

        for (const auto& expr : exprs)
        {
            builder += " " + print(*expr);
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
};

static LoxppLogger logger;

int main(int argc, char** argv)
{

    Literal<std::string> literal = Literal<std::string>("123");
    Unary<std::string> unary = Unary<std::string>(Token(TokenType::MINUS, "-", "", 1), literal);

    Literal<std::string> literal2 = Literal<std::string>("45.67");
    Grouping<std::string> grouping = Grouping<std::string>(literal2);

    Binary<std::string> expr = Binary<std::string>(unary, Token(TokenType::STAR, "*", "", 1), grouping);

    std::cout << AstPrinter().print(expr) << std::endl;

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

    for (Token token : tokens)
    {
        std::cout << token.toString() << std::endl;
    }
}

void reportError(int line, const std::string& where, const std::string& message)
{
    hadError = true;
    std::cerr << "[line " << line << "] Error" << where << ": " << message << std::endl;
}