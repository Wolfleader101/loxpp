#include <fstream>
#include <iostream>
#include <string>

#include "ILogger.hpp"
#include "Token.hpp"

#include "Scanner.hpp"

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