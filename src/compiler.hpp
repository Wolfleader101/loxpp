#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

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

    struct Local
    {
        Token name;
        int depth;
    };

    std::vector<Local> m_locals;
    size_t m_localCount;
    int m_scopeDepth;
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

    using ParseFn = std::function<void(Compiler*, bool)>;
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
    void synchronize();

    uint8_t makeConstant(Value value);

    void declaration();
    void varDeclaration();
    void statement();
    void beginScope();
    void block();
    void endScope();
    void ifStatement();
    int emitJump(uint8_t instruction);
    void patchJump(int offset);

    bool match(TokenType type);
    bool check(TokenType type);
    bool identifiersEqual(const Token& a, const Token& b);
    void parsePrecedence(Precedence precedence);
    void namedVariable(const Token& name, bool canAssign);
    int resolveLocal(const Token& name);

    void printStatement();
    void expressionStatement();
    uint8_t parseVariable(const char* errorMessage);
    uint8_t identifierConstant(const Token& name);
    void defineVariable(uint8_t global);
    void declareVariable();
    void addLocal(const Token& name);
    void markInitialized();

    void numberConstant(bool canAssign);
    void stringConstant(bool canAssign);
    void grouping(bool canAssign);
    void unary(bool canAssign);
    void binary(bool canAssign);
    void literal(bool canAssign);
    void variable(bool canAssign);

#define BIND_FN(fn) std::bind(&Compiler::fn, this, std::placeholders::_1)

    const std::map<TokenType, ParseRule> rules = {
        {TOKEN_LEFT_PAREN, {BIND_FN(grouping), nullptr, PREC_NONE}},
        {TOKEN_RIGHT_PAREN, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_LEFT_BRACE, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_RIGHT_BRACE, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_COMMA, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_DOT, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_MINUS, {BIND_FN(unary), BIND_FN(binary), PREC_TERM}},
        {TOKEN_PLUS, {nullptr, BIND_FN(binary), PREC_TERM}},
        {TOKEN_SEMICOLON, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_SLASH, {nullptr, BIND_FN(binary), PREC_FACTOR}},
        {TOKEN_STAR, {nullptr, BIND_FN(binary), PREC_FACTOR}},
        {TOKEN_BANG, {BIND_FN(unary), nullptr, PREC_NONE}},
        {TOKEN_BANG_EQUAL, {nullptr, BIND_FN(binary), PREC_EQUALITY}},
        {TOKEN_EQUAL, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_EQUAL_EQUAL, {nullptr, BIND_FN(binary), PREC_EQUALITY}},
        {TOKEN_GREATER, {nullptr, BIND_FN(binary), PREC_COMPARISON}},
        {TOKEN_GREATER_EQUAL, {nullptr, BIND_FN(binary), PREC_COMPARISON}},
        {TOKEN_LESS, {nullptr, BIND_FN(binary), PREC_COMPARISON}},
        {TOKEN_LESS_EQUAL, {nullptr, BIND_FN(binary), PREC_COMPARISON}},
        {TOKEN_IDENTIFIER, {BIND_FN(variable), nullptr, PREC_NONE}},
        {TOKEN_STRING, {BIND_FN(stringConstant), nullptr, PREC_NONE}},
        {TOKEN_NUMBER, {BIND_FN(numberConstant), nullptr, PREC_NONE}},
        {TOKEN_AND, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_CLASS, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_ELSE, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_FALSE, {BIND_FN(literal), nullptr, PREC_NONE}},
        {TOKEN_FOR, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_FUN, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_IF, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_NIL, {BIND_FN(literal), nullptr, PREC_NONE}},
        {TOKEN_OR, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_PRINT, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_RETURN, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_SUPER, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_THIS, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_TRUE, {BIND_FN(literal), nullptr, PREC_NONE}},
        {TOKEN_VAR, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_WHILE, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_ERROR, {nullptr, nullptr, PREC_NONE}},
        {TOKEN_EOF, {nullptr, nullptr, PREC_NONE}},
    };
};
