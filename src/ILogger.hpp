#pragma once

#include <string>

#include "RuntimeError.hpp"

class ILogger
{
  public:
    virtual ~ILogger() = default;

    virtual void LogError(int line, const std::string& where, const std::string& message) = 0;
    virtual void LogError(int line, const std::string& message) = 0;

    virtual void LogRuntimeError(const RuntimeError& error) = 0;
};