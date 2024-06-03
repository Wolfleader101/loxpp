#include "scanner.hpp"

Token Scanner::scanToken()
{
    skipWhitespace();
    m_start = m_current;

    if (isAtEnd())
        return makeToken(TOKEN_EOF);

    char c = advance();
    if (isAlpha(c))
        return identifierToken();

    if (isDigit(c))
        return numberToken();

    switch (c)
    {
    case '(':
        return makeToken(TOKEN_LEFT_PAREN);
    case ')':
        return makeToken(TOKEN_RIGHT_PAREN);
    case '{':
        return makeToken(TOKEN_LEFT_BRACE);
    case '}':
        return makeToken(TOKEN_RIGHT_BRACE);
    case ';':
        return makeToken(TOKEN_SEMICOLON);
    case ',':
        return makeToken(TOKEN_COMMA);
    case '.':
        return makeToken(TOKEN_DOT);
    case '-':
        return makeToken(TOKEN_MINUS);
    case '+':
        return makeToken(TOKEN_PLUS);
    case '/':
        return makeToken(TOKEN_SLASH);
    case '*':
        return makeToken(TOKEN_STAR);
    case '!':
        return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
        return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
        return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
        return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '"':
        return stringToken();
    }

    return errorToken("Unexpected character.");
}

bool Scanner::isAtEnd()
{
    return m_current >= m_source.size();
}

Token Scanner::makeToken(TokenType type)
{
    return Token(type, &m_source.at(m_start), m_current - m_start, m_line);
}

Token Scanner::errorToken(const std::string& message)
{
    return Token(TOKEN_ERROR, message.c_str(), message.size(), m_line);
}

char Scanner::advance()
{
    return m_source[m_current++]; //! TODO book has m_current - 1??
}

bool Scanner::match(char expected)
{
    if (isAtEnd())
        return false;
    if (m_source[m_current] != expected)
        return false;

    m_current++;
    return true;
}

void Scanner::skipWhitespace()
{
    for (;;)
    {
        char c = peek();
        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            m_line++;
            advance();
            break;
        case '/':
            if (peekNext() == '/')
            {
                while (peek() != '\n' && !isAtEnd())
                    advance();
            }
            else
            {
                return;
            }
        default:
            return;
        }
    }
}

char Scanner::peek()
{
    return isAtEnd() ? '\0' : m_source[m_current];
}

char Scanner::peekNext()
{
    if (isAtEnd())
        return '\0';
    return m_source[m_current + 1];
}
bool Scanner::isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Scanner::isDigit(char c)
{
    return c >= '0' && c <= '9';
}

Token Scanner::stringToken()
{
    while (peek() != '"' && !isAtEnd())
    {
        if (peek() == '\n')
            m_line++;
        advance();
    }

    if (isAtEnd())
        return errorToken("Unterminated string.");

    advance(); // consume the closing "
    return makeToken(TOKEN_STRING);
}

Token Scanner::numberToken()
{
    while (isDigit(peek()))
        advance();

    if (peek() == '.' && isDigit(peekNext()))
    {
        advance(); // consume the .
        while (isDigit(peek()))
            advance();
    }

    return makeToken(TOKEN_NUMBER);
}

Token Scanner::identifierToken()
{
    while (isAlpha(peek()) || isDigit(peek()))
        advance();

    return makeToken(identifierType());
}

TokenType Scanner::identifierType()
{
    switch (m_source[m_start])
    {
    case 'a':
        return checkKeyword(1, 2, "nd", TOKEN_AND);
    case 'c':
        return checkKeyword(1, 4, "lass", TOKEN_CLASS);
    case 'e':
        return checkKeyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
        if (m_current - m_start > 1)
        {
            switch (m_source[m_start + 1])
            {
            case 'a':
                return checkKeyword(2, 3, "lse", TOKEN_FALSE);
            case 'o':
                return checkKeyword(2, 1, "r", TOKEN_FOR);
            case 'u':
                return checkKeyword(2, 1, "n", TOKEN_FUN);
            }
        }
        break;
    case 'i':
        return checkKeyword(1, 1, "f", TOKEN_IF);
    case 'n':
        return checkKeyword(1, 2, "il", TOKEN_NIL);
    case 'o':
        return checkKeyword(1, 1, "r", TOKEN_OR);
    case 'p':
        return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r':
        return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's':
        return checkKeyword(1, 4, "uper", TOKEN_SUPER);
    case 't':
        if (m_current - m_start > 1)
        {
            switch (m_source[m_start + 1])
            {
            case 'h':
                return checkKeyword(2, 2, "is", TOKEN_THIS);
            case 'r':
                return checkKeyword(2, 2, "ue", TOKEN_TRUE);
            }
        }
        break;
    case 'v':
        return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w':
        return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

TokenType Scanner::checkKeyword(int start, int length, const std::string& rest, TokenType type)
{
    if (m_current - m_start == start + length && m_source.compare(m_start + start, length, rest) == 0)
    {
        return type;
    }

    return TOKEN_IDENTIFIER;
}