#include "Environment.hpp"

#include "RuntimeError.hpp"

Environment::Environment(std::shared_ptr<Environment> enclosing) : enclosing(enclosing)
{
}

void Environment::define(const std::string& name, LoxTypeRef value)
{
    values[name] = value;
}

void Environment::assign(const Token& name, LoxTypeRef value)
{
    if (values.find(name.lexeme) != values.end())
    {
        values[name.lexeme] = value;
        return;
    }

    if (enclosing != nullptr)
    {
        enclosing->assign(name, value);
        return;
    }

    throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}

LoxTypeRef Environment::get(const Token& name)
{
    if (values.find(name.lexeme) != values.end())
    {
        return values[name.lexeme];
    }

    if (enclosing != nullptr)
    {
        return enclosing->get(name);
    }

    throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}
