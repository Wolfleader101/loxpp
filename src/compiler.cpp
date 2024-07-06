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
    // expression();
    // consume(TOKEN_EOF, "Expect end of expression.");
    while (!match(TOKEN_EOF))
    {
        declaration();
    }

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

void Compiler::numberConstant(bool)
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
void Compiler::grouping(bool)
{
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::unary(bool)
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

    bool canAssign = precedence <= Precedence::PREC_ASSIGNMENT;
    prefixRule(this, canAssign); // Call the prefix parse function

    while (precedence <= rules.at(m_parser.current.type).precedence)
    {
        advance();
        ParseFn infixRule = rules.at(m_parser.previous.type).infix;
        infixRule(this, canAssign); // Call the infix parse function
    }

    if (canAssign && match(TOKEN_EQUAL))
    {
        error("Invalid assignment target.");
    }
}

void Compiler::binary(bool)
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

void Compiler::literal(bool)
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

void Compiler::stringConstant(bool)
{
    auto obj = std::make_shared<ObjString>(m_parser.previous.start + 1, m_parser.previous.length - 2);
    m_vm.insertObject(obj);
    emitConstant(Value(obj));
}

bool Compiler::match(TokenType type)
{
    if (!check(type))
        return false;
    advance();
    return true;
}

bool Compiler::check(TokenType type)
{
    return m_parser.current.type == type;
}

void Compiler::declaration()
{
    if (match(TOKEN_VAR))
    {
        varDeclaration();
    }
    else
    {
        statement();
    }

    if (m_parser.panicMode)
    {
        synchronize();
    }
}

void Compiler::statement()
{
    if (match(TOKEN_PRINT))
    {
        printStatement();
    }
    else if (match(TOKEN_IF))
    {
        ifStatement();
    }
    else if (match(TOKEN_LEFT_BRACE))
    {
        beginScope();
        block();
        endScope();
    }
    else
    {
        expressionStatement();
    }
}

void Compiler::printStatement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

void Compiler::expressionStatement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

void Compiler::synchronize()
{
    m_parser.panicMode = false;

    while (m_parser.current.type != TOKEN_EOF)
    {
        if (m_parser.previous.type == TOKEN_SEMICOLON)
            return;

        switch (m_parser.current.type)
        {
        case TOKEN_CLASS:
        case TOKEN_FUN:
        case TOKEN_VAR:
        case TOKEN_FOR:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_PRINT:
        case TOKEN_RETURN:
            return;
        default:
            // do nothing
            ;
        }

        advance();
    }
}

void Compiler::varDeclaration()
{
    uint8_t global = parseVariable("Expect variable name.");

    if (match(TOKEN_EQUAL))
    {
        expression();
    }
    else
    {
        emitByte(OP_NIL);
    }

    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    defineVariable(global);
}

uint8_t Compiler::parseVariable(const char* errorMessage)
{
    consume(TOKEN_IDENTIFIER, errorMessage);

    declareVariable();
    if (m_scopeDepth > 0)
        return 0;

    return identifierConstant(m_parser.previous);
}

uint8_t Compiler::identifierConstant(const Token& name)
{
    return makeConstant(Value(std::make_shared<ObjString>(name.start, name.length)));
}

void Compiler::defineVariable(uint8_t global)
{
    if (m_scopeDepth > 0)
    {
        markInitialized();
        return;
    }

    emitBytes(OP_DEFINE_GLOBAL, global);
}

void Compiler::variable(bool canAssign)
{
    namedVariable(m_parser.previous, canAssign);
}

void Compiler::namedVariable(const Token& name, bool canAssign)
{
    uint8_t getOp, setOp;
    int arg = resolveLocal(name);
    if (arg != -1)
    {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    }
    else
    {
        arg = identifierConstant(name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && match(TOKEN_EQUAL))
    {
        expression();
        emitBytes(setOp, static_cast<uint8_t>(arg));
    }
    else
    {
        emitBytes(getOp, static_cast<uint8_t>(arg));
    }
}

void Compiler::beginScope()
{
    m_scopeDepth++;
}

void Compiler::block()
{
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
    {
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

void Compiler::endScope()
{
    m_scopeDepth--;

    while (m_localCount > 0 && m_locals[m_localCount - 1].depth > m_scopeDepth)
    {
        emitByte(OP_POP);
        m_localCount--;
    }
}

void Compiler::declareVariable()
{
    if (m_scopeDepth == 0)
        return;

    Token& name = m_parser.previous;

    for (size_t i = m_localCount - 1; i >= 0; i--)
    {
        Local& local = m_locals[i];
        if (local.depth != -1 && local.depth < m_scopeDepth)
            break;

        if (identifiersEqual(name, local.name))
        {
            error("Variable with this name already declared in this scope.");
        }
    }

    addLocal(name);
}

void Compiler::addLocal(const Token& name)
{
#define UINT8_COUNT (UINT8_MAX + 1)
    if (m_localCount == UINT8_COUNT) // TODO i dont think this is needed because of vector?
    {
        error("Too many local variables in function.");
        return;
    }

    Local& local = m_locals[m_localCount++];
    local.name = name;
    local.depth = -1;
#undef UINT8_COUNT
}

bool Compiler::identifiersEqual(const Token& a, const Token& b)
{
    if (a.length != b.length)
        return false;

    return std::strncmp(a.start, b.start, a.length) == 0;
}

int Compiler::resolveLocal(const Token& name)
{
    for (size_t i = m_localCount - 1; i >= 0; i--)
    {
        Local& local = m_locals[i];
        if (identifiersEqual(name, local.name))
        {
            if (local.depth == -1)
            {
                error("Cannot read local variable in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

void Compiler::markInitialized()
{
    if (m_scopeDepth == 0)
        return;

    m_locals[m_localCount - 1].depth = m_scopeDepth;
}

void Compiler::ifStatement()
{
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();

    int elseJump = emitJump(OP_JUMP);

    patchJump(thenJump);
    emitByte(OP_POP);

    if (match(TOKEN_ELSE))
        statement();

    patchJump(elseJump);
}

int Compiler::emitJump(uint8_t instruction)
{
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return m_currentChunk->code.size() - 2;
}

void Compiler::patchJump(int offset)
{
    // -2 to adjust for the bytecode for the jump offset itself.
    int jump = m_currentChunk->code.size() - offset - 2;

    if (jump > UINT16_MAX)
    {
        error("Too much code to jump over.");
    }

    m_currentChunk->code[offset] = (jump >> 8) & 0xff;
    m_currentChunk->code[offset + 1] = jump & 0xff;
}