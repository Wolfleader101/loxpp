#include "Scanner.hpp"

Scanner::Scanner(const std::string& source, ILogger& logger) : source(source), logger(logger), tokens()
{
}

std::vector<Token> Scanner::scanTokens()
{
    while (!isAtEnd())
    {
        // We are at the beginning of the next lexeme.
        start = current;
        scanToken();
    }

    tokens.push_back(Token(TokenType::END_OF_FILE, "", "", line));
    return tokens;
}

void Scanner::scanToken()
{
    char c = advance();
    switch (c)
    {
    case '(':
        addToken(TokenType::LEFT_PAREN);
        break;
    case ')':
        addToken(TokenType::RIGHT_PAREN);
        break;
    case '{':
        addToken(TokenType::LEFT_BRACE);
        break;
    case '}':
        addToken(TokenType::RIGHT_BRACE);
        break;
    case ',':
        addToken(TokenType::COMMA);
        break;
    case '.':
        addToken(TokenType::DOT);
        break;
    case '-':
        addToken(TokenType::MINUS);
        break;
    case '+':
        addToken(TokenType::PLUS);
        break;
    case ';':
        addToken(TokenType::SEMICOLON);
        break;
    case '*':
        addToken(TokenType::STAR);
        break;
    case '!':
        addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
        break;
    case '=':
        addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
        break;
    case '<':
        addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
        break;
    case '>':
        addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
        break;
    case '/':
        if (match('/'))
        {
            // A comment goes until the end of the line.
            while (peek() != '\n' && !isAtEnd())
            {
                advance();
            }
        }
        else
        {
            addToken(TokenType::SLASH);
        }
        break;
    case ' ':
    case '\r':
    case '\t':
        // Ignore whitespace.
        break;
    case '\n':
        line++;
        break;
    case '"':
        readString();
        break;
    default:
        if (isDigit(c))
        {
            readNumber();
        }
        else if (isAlpha(c))
        {
            readIdentifier();
        }
        else
        {
            logger.LogError(line, "Unexpected character.");
        }
        break;
    }
}

bool Scanner::isAtEnd()
{
    return current >= source.length();
}

char Scanner::advance()
{
    current++;
    return source[current - 1];
}

void Scanner::addToken(TokenType type)
{
    addToken(type, "");
}

void Scanner::addToken(TokenType type, const std::string& literal)
{
    std::string text = source.substr(start, current - start);
    tokens.push_back(Token(type, text, literal, line));
}

bool Scanner::match(char expected)
{
    if (isAtEnd())
    {
        return false;
    }

    if (source[current] != expected)
    {
        return false;
    }

    current++;
    return true;
}

char Scanner::peek()
{
    if (isAtEnd())
    {
        return '\0';
    }

    return source[current];
}

void Scanner::readString()
{
    while (peek() != '"' && !isAtEnd())
    {
        if (peek() == '\n')
            line++;
        advance();
    }

    if (isAtEnd())
    {
        logger.LogError(line, "Unterminated string.");
        return;
    }

    // The closing ".
    advance();

    // trim the srounding quotes
    std::string value = source.substr(start + 1, current - start - 2);
    addToken(TokenType::STRING, value);
}

bool Scanner::isDigit(char c)
{
    return c >= '0' && c <= '9';
}

void Scanner::readNumber()
{
    while (isDigit(peek()))
    {
        advance();
    }

    // Look for a fractional part.
    if (peek() == '.' && isDigit(peekNext()))
    {
        // Consume the "."
        advance();

        while (isDigit(peek()))
        {
            advance();
        }
    }

    addToken(TokenType::NUMBER, source.substr(start, current - start));
}

char Scanner::peekNext()
{
    if (current + 1 >= source.length())
    {
        return '\0';
    }

    return source[current + 1];
}

bool Scanner::isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Scanner::isAlphaNumeric(char c)
{
    return isAlpha(c) || isDigit(c);
}

void Scanner::readIdentifier()
{
    while (isAlphaNumeric(peek()))
    {
        advance();
    }

    std::string text = source.substr(start, current - start);
    TokenType type = keywords.contains(text) ? keywords.at(text) : TokenType::IDENTIFIER;

    addToken(type);
}