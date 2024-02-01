#include "LoxType.hpp"

#include <sstream>

std::string LoxTypeToString(const LoxType& var)
{
    if (!var.has_value())
    {
        return "nil";
    }

    return std::visit(
        [](const auto& value) -> std::string {
            using T = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<T, bool>)
            {
                return value ? "true" : "false";
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                std::ostringstream oss;
                oss << value;
                return oss.str();
            }
            else if constexpr (std::is_same_v<T, std::shared_ptr<LoxCallable>>)
            {
                return value.get()->toString();
            }
            else
            {
                // if its a string
                return value;
            }
        },
        var.value());
}