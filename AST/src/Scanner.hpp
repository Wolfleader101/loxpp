#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "ILogger.hpp"
#include "Token.hpp"

#include "LoxType.hpp"

class Scanner
{
  public:
    Scanner(const std::string& source, ILogger& logger);

    std::vector<Token> scanTokens();

  private:
    std::string source;
    ILogger& logger;
    std::vector<Token> tokens;
    int start = 0;
    int current = 0;
    int line = 1;

    const std::unordered_map<std::string, TokenType> keywords = {
        {"and", TokenType::AND},     {"class", TokenType::CLASS},   {"else", TokenType::ELSE},
        {"false", TokenType::FALSE}, {"for", TokenType::FOR},       {"fun", TokenType::FUN},
        {"if", TokenType::IF},       {"nil", TokenType::NIL},       {"or", TokenType::OR},
        {"print", TokenType::PRINT}, {"return", TokenType::RETURN}, {"super", TokenType::SUPER},
        {"this", TokenType::THIS},   {"true", TokenType::TRUE},     {"var", TokenType::VAR},
        {"while", TokenType::WHILE},
    };

    void scanToken();

    bool isAtEnd();

    char advance();

    void addToken(TokenType type);

    void addToken(TokenType type, const LoxType& literal);

    bool match(char expected);

    char peek();

    void readString();

    bool isDigit(char c);

    void readNumber();

    char peekNext();

    bool isAlpha(char c);

    bool isAlphaNumeric(char c);

    void readIdentifier();
};