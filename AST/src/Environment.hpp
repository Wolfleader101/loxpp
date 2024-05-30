#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "LoxType.hpp"
#include "Token.hpp"

class Environment
{
  public:
    Environment() = default;

    Environment(std::shared_ptr<Environment> enclosing);

    // TODO should this be a shared ptr or just copy the value?
    void define(const std::string& name, LoxTypeRef value);

    void assign(const Token& name, LoxTypeRef value);

    LoxTypeRef get(const Token& name);

    LoxTypeRef getAt(int distance, const std::string& name);

    void assignAt(int distance, const Token& name, LoxTypeRef value);

    const std::shared_ptr<Environment> enclosing = nullptr;

  private:
    std::unordered_map<std::string, LoxTypeRef> values;

    std::shared_ptr<Environment> ancestor(int distance);
};