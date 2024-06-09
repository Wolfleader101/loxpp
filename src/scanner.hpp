#pragma once

#include <string>

enum TokenType
{
    // Single-character tokens.
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_SEMICOLON,
    TOKEN_SLASH,
    TOKEN_STAR,
    // One or two character tokens.
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    // Literals.
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,
    // Keywords.
    TOKEN_AND,
    TOKEN_CLASS,
    TOKEN_ELSE,
    TOKEN_FALSE,
    TOKEN_FOR,
    TOKEN_FUN,
    TOKEN_IF,
    TOKEN_NIL,
    TOKEN_OR,
    TOKEN_PRINT,
    TOKEN_RETURN,
    TOKEN_SUPER,
    TOKEN_THIS,
    TOKEN_TRUE,
    TOKEN_VAR,
    TOKEN_WHILE,

    TOKEN_ERROR,
    TOKEN_EOF
};

struct Token
{
    Token() = default;

    Token(TokenType type, const char* start, int length, int line)
        : type(type), start(start), length(length), line(line)
    {
    }
    TokenType type;
    const char* start;
    int length;
    int line;
};

class Scanner
{
  public:
    Scanner() = default;

    Scanner(const std::string& source) : m_source(source)
    {
        m_start = 0;
        m_current = 0;
        m_line = 1;
    }

    Token scanToken();

    void setSource(const std::string& source)
    {
        m_source = source;
        m_start = 0;
        m_current = 0;
        m_line = 1;
    }

  private:
    std::string m_source;

    size_t m_start;
    size_t m_current;
    int m_line;

    bool isAtEnd();
    Token makeToken(TokenType type);
    Token errorToken(const std::string& message);
    char advance();
    bool match(char expected);
    void skipWhitespace();
    char peek();
    char peekNext();

    bool isAlpha(char c);
    bool isDigit(char c);

    Token stringToken();
    Token numberToken();
    Token identifierToken();

    TokenType identifierType();
    TokenType checkKeyword(int start, int length, const std::string& rest, TokenType type);
};