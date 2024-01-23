#pragma once

#include <string>

class ILogger
{
  public:
    virtual ~ILogger() = default;

    virtual void LogError(int line, const std::string& message) = 0;
};