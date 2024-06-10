#pragma once

#include <functional>
#include <map>
#include <string>

#include "chunk.hpp"
#include "scanner.hpp"

class VM;

class Compiler
{
  public:
    Compiler(VM& vm) : m_vm(vm), m_scanner(), m_currentChunk(nullptr)
    {
    }

    bool compile(const std::string& source, Chunk* chunk);

  private:
    VM& m_vm;
    struct Parser
    {
        Token current;
        Token previous;
        bool hadError;
        bool panicMode;
    };

    enum Precedence
    {
        PREC_NONE,
        PREC_ASSIGNMENT, // =
        PREC_OR,         // or
        PREC_AND,        // and
        PREC_EQUALITY,   // == !=
        PREC_COMPARISON, // < > <= >=
        PREC_TERM,       // + -
        PREC_FACTOR,     // * /
        PREC_UNARY,      // ! -
        PREC_CALL,       // . ()
        PREC_PRIMARY
    };

    using ParseFn = std::function<void(Compiler*)>;
    struct ParseRule
    {
        ParseFn prefix;
        ParseFn infix;
        Precedence precedence;
    };

    Scanner m_scanner;
    Parser m_parser;
    Chunk* m_currentChunk;

    void advance();
    void consume(TokenType type, const char* message);
    void expression();
    void emitByte(uint8_t byte);
    void emitBytes(uint8_t b1, uint8_t b2);
    void endCompiler();
    void emitReturn();
    void emitConstant(Value value);

    void errorAtCurrent(const char* message);
    void errorAt(const Token& token, const char* message);
    void error(const char* message);

    uint8_t makeConstant(Value value);
    void numberConstant();
    void stringConstant();
    void grouping();
    void unary();
    void binary();
    void literal();

    void parsePrecedence(Precedence precedence);

    const std::map<TokenType, ParseRule> rules = {
        {TOKEN_LEFT_PAREN, {std::bind(&Compiler::grouping, this), nullptr, PREC_NONE}},
        {TOKEN_RIGHT_PAREN, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_LEFT_BRACE, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_RIGHT_BRACE, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_COMMA, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_DOT, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_MINUS, {std::bind(&Compiler::unary, this), std::bind(&Compiler::binary, this), PREC_TERM}},
        {TOKEN_PLUS, {nullptr, std::bind(&Compiler::binary, this), PREC_TERM}},
        {TOKEN_SEMICOLON, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_SLASH, {nullptr, std::bind(&Compiler::binary, this), PREC_FACTOR}},
        {TOKEN_STAR, {nullptr, std::bind(&Compiler::binary, this), PREC_FACTOR}},
        {TOKEN_BANG, {std::bind(&Compiler::unary, this), nullptr, PREC_NONE}},
        {TOKEN_BANG_EQUAL, {nullptr, std::bind(&Compiler::binary, this), PREC_EQUALITY}},
        {TOKEN_EQUAL, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_EQUAL_EQUAL, {nullptr, std::bind(&Compiler::binary, this), PREC_EQUALITY}},
        {TOKEN_GREATER, {nullptr, std::bind(&Compiler::binary, this), PREC_COMPARISON}},
        {TOKEN_GREATER_EQUAL, {nullptr, std::bind(&Compiler::binary, this), PREC_COMPARISON}},
        {TOKEN_LESS, {nullptr, std::bind(&Compiler::binary, this), PREC_COMPARISON}},
        {TOKEN_LESS_EQUAL, {nullptr, std::bind(&Compiler::binary, this), PREC_COMPARISON}},
        {TOKEN_IDENTIFIER, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_STRING, {std::bind(&Compiler::stringConstant, this), nullptr, PREC_NONE}},
        {TOKEN_NUMBER, {std::bind(&Compiler::numberConstant, this), nullptr, PREC_NONE}},
        {TOKEN_AND, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_CLASS, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_ELSE, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_FALSE, {std::bind(&Compiler::literal, this), nullptr, PREC_NONE}},
        {TOKEN_FOR, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_FUN, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_IF, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_NIL, {std::bind(&Compiler::literal, this), nullptr, PREC_NONE}},
        {TOKEN_OR, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_PRINT, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_RETURN, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_SUPER, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_THIS, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_TRUE, {std::bind(&Compiler::literal, this), nullptr, PREC_NONE}},
        {TOKEN_VAR, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_WHILE, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_ERROR, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_EOF, {nullptr, nullptr, PREC_NONE}},
    };
};
