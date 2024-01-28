#pragma once

#include "Token.hpp"
#include <exception>
#include <string>

class RuntimeError : public std::exception
{
  public:
    RuntimeError(const Token& token, const std::string& message) : token(token), message(message)
    {
    }

    const char* what() const noexcept override
    {
        return message.c_str();
    }

    const Token token;
    const std::string message;
};