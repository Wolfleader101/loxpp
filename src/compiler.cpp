#include "compiler.hpp"

#include <iomanip>
#include <iostream>

#include "common.hpp"
#include "scanner.hpp"
#include "vm.hpp"

#ifdef DEBUG_PRINT_CODE
#include "debug.hpp"
#endif

bool Compiler::compile(const std::string& source, Chunk* chunk)
{
    m_scanner.setSource(source);
    m_currentChunk = chunk;

    m_parser.hadError = false;
    m_parser.panicMode = false;

    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");
    endCompiler();

    return !m_parser.hadError;
}

void Compiler::advance()
{
    m_parser.previous = m_parser.current;

    for (;;)
    {
        m_parser.current = m_scanner.scanToken();
        if (m_parser.current.type != TOKEN_ERROR)
            break;

        errorAtCurrent(m_parser.current.start);
    }
}

void Compiler::errorAtCurrent(const char* message)
{
    errorAt(m_parser.current, message);
}

void Compiler::error(const char* message)
{
    errorAt(m_parser.previous, message);
}

void Compiler::errorAt(const Token& token, const char* message)
{
    if (m_parser.panicMode)
        return;
    m_parser.panicMode = true;
    std::cout << "[line " << token.line << "] Error";

    if (token.type == TOKEN_EOF)
    {
        std::cout << " at end";
    }
    else if (token.type == TOKEN_ERROR)
    {
    }
    else
    {
        std::cout << " at '" << std::string(token.start, token.length) << "'";
    }

    std::cout << ": " << message << std::endl;
    m_parser.hadError = true;
}

void Compiler::consume(TokenType type, const char* message)
{
    if (m_parser.current.type == type)
    {
        advance();
        return;
    }

    errorAtCurrent(message);
}

void Compiler::emitByte(uint8_t byte)
{
    m_currentChunk->writeChunk(byte, m_parser.previous.line);
}

void Compiler::endCompiler()
{
    emitReturn();

#ifdef DEBUG_PRINT_CODE
    if (!m_parser.hadError)
        disassembleChunk(*m_currentChunk, "code");
#endif
}

void Compiler::emitReturn()
{
    emitByte(OP_RETURN);
}

void Compiler::emitBytes(uint8_t b1, uint8_t b2)
{
    emitByte(b1);
    emitByte(b2);
}

void Compiler::expression()
{
    parsePrecedence(Precedence::PREC_ASSIGNMENT);
}

void Compiler::numberConstant()
{
    double value = std::stod(std::string(m_parser.previous.start, m_parser.previous.length));
    emitConstant(Value(value));
}

void Compiler::emitConstant(Value value)
{
    emitBytes(OP_CONSTANT, makeConstant(value));
}

uint8_t Compiler::makeConstant(Value value)
{
    size_t constant = m_currentChunk->addConstant(value);
    if (constant > UINT8_MAX)
    {
        error("Too many constants in one chunk.");
        return 0;
    }

    return static_cast<uint8_t>(constant);
}
void Compiler::grouping()
{
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::unary()
{
    TokenType operatorType = m_parser.previous.type;

    parsePrecedence(Precedence::PREC_UNARY);

    // emite operator instruction
    switch (operatorType)
    {
    case TOKEN_BANG:
        emitByte(OP_NOT);
        break;
    case TOKEN_MINUS:
        emitByte(OP_NEGATE);
        break;
    default:
        return;
    }
}

void Compiler::parsePrecedence(Precedence precedence)
{
    advance();
    ParseFn prefixRule = rules.at(m_parser.previous.type).prefix;
    if (prefixRule == nullptr)
    {
        error("Expect expression.");
        return;
    }

    prefixRule(this); // Call the prefix parse function

    while (precedence <= rules.at(m_parser.current.type).precedence)
    {
        advance();
        ParseFn infixRule = rules.at(m_parser.previous.type).infix;
        infixRule(this); // Call the infix parse function
    }
}

void Compiler::binary()
{
    TokenType operatorType = m_parser.previous.type;
    const ParseRule& rule = rules.at(operatorType);
    parsePrecedence(static_cast<Precedence>(rule.precedence + 1));

    switch (operatorType)
    {
    case TOKEN_BANG_EQUAL:
        emitBytes(OP_EQUAL, OP_NOT);
        break;
    case TOKEN_EQUAL_EQUAL:
        emitByte(OP_EQUAL);
        break;
    case TOKEN_GREATER:
        emitByte(OP_GREATER);
        break;
    case TOKEN_GREATER_EQUAL:
        emitBytes(OP_LESS, OP_NOT);
        break;
    case TOKEN_LESS:
        emitByte(OP_LESS);
        break;
    case TOKEN_LESS_EQUAL:
        emitBytes(OP_GREATER, OP_NOT);
        break;
    case TOKEN_PLUS:
        emitByte(OP_ADD);
        break;
    case TOKEN_MINUS:
        emitByte(OP_SUBTRACT);
        break;
    case TOKEN_STAR:
        emitByte(OP_MULTIPLY);
        break;
    case TOKEN_SLASH:
        emitByte(OP_DIVIDE);
        break;
    default:
        return; // Unreachable.
    }
}

void Compiler::literal()
{
    switch (m_parser.previous.type)
    {
    case TOKEN_FALSE:
        emitByte(OP_FALSE);
        break;
    case TOKEN_TRUE:
        emitByte(OP_TRUE);
        break;
    case TOKEN_NIL:
        emitByte(OP_NIL);
        break;
    default:
        return; // Unreachable.
    }
}

void Compiler::stringConstant()
{
    auto obj = std::make_shared<ObjString>(m_parser.previous.start + 1, m_parser.previous.length - 2);
    m_vm.insertObject(obj);
    emitConstant(Value(obj));
}