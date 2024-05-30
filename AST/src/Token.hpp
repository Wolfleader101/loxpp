#pragma once

#include <string>

#include "LoxType.hpp"

enum class TokenType
{
    // single char tokens
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    SLASH,
    STAR,

    // single or two char tokens
    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,

    // literals
    IDENTIFIER,
    STRING,
    NUMBER,

    // keywords
    AND,
    CLASS,
    ELSE,
    FALSE,
    FUN,
    FOR,
    IF,
    NIL,
    OR,
    PRINT,
    RETURN,
    SUPER,
    THIS,
    TRUE,
    VAR,
    WHILE,

    END_OF_FILE
};

constexpr const char* TokenTypeToString(TokenType type);

class Token
{
  public:
    Token(TokenType type, const std::string& lexeme, const LoxType& literal, int line);

    std::string toString();

    TokenType type;
    std::string lexeme;
    LoxType literal;
    int line;
};