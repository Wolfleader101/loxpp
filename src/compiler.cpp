#include "compiler.hpp"

#include "scanner.hpp"

#include <iomanip>
#include <iostream>

void compile(const std::string& source)
{
    Scanner scanner(source);

    int line = -1;

    for (;;)
    {
        Token token = scanner.scanToken();

        if (token.line != line)
        {
            std::cout << std::setfill('0') << std::setw(4) << token.line << " ";
            line = token.line;
        }
        else
        {
            std::cout << "   | ";
        }

        printf("%2d '%.*s'\n", token.type, token.length, token.start);

        if (token.type == TOKEN_EOF)
            break;
    }
}