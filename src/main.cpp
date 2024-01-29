#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "ILogger.hpp"
#include "Token.hpp"

#include "Scanner.hpp"

#include "Expr.hpp"
#include "Stmt.hpp"

#include "Interpreter.hpp"
#include "Parser.hpp"
#include "RuntimeError.hpp"

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
    std::vector<std::shared_ptr<Stmt<LoxType>>> statements = parser.parse();

    if (hadError)
        return;

    interpreter.interpret(statements);

    // AstPrinter printer;
    // std::cout << printer.print(expression) << std::endl;
}

void reportError(int line, const std::string& where, const std::string& message)
{
    hadError = true;
    std::cerr << "[line " << line << "] Error" << where << ": " << message << std::endl;
}