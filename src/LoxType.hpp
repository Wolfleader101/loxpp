#pragma once

#include <optional>
#include <string>
#include <type_traits>
#include <variant>

using LoxType = std::optional<std::variant<bool, double, std::string>>;

constexpr std::size_t BOOL_INDEX =
    std::variant_size_v<std::decay_t<decltype(*std::declval<LoxType>().operator->())>> == 0 ? -1 : 0;
constexpr std::size_t DOUBLE_INDEX =
    std::variant_size_v<std::decay_t<decltype(*std::declval<LoxType>().operator->())>> <= 1 ? -1 : 1;
constexpr std::size_t STRING_INDEX =
    std::variant_size_v<std::decay_t<decltype(*std::declval<LoxType>().operator->())>> <= 2 ? -1 : 2;

inline bool IsBool(const LoxType& loxType)
{
    return loxType.has_value() && loxType.value().index() == BOOL_INDEX;
}

inline bool IsDouble(const LoxType& loxType)
{
    return loxType.has_value() && loxType.value().index() == DOUBLE_INDEX;
}

inline bool IsString(const LoxType& loxType)
{
    return loxType.has_value() && loxType.value().index() == STRING_INDEX;
}

std::string LoxTypeToString(const LoxType& var);