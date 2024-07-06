#pragma once

#include <vector>

#include "value.hpp"

enum OpCode
{
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_RETURN,
};

struct Chunk
{
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<int> lines;

    void writeChunk(uint8_t byte, int line)
    {
        code.push_back(byte);
        lines.push_back(line);
    }

    size_t addConstant(Value value)
    {
        constants.push_back(value);
        return constants.size() - 1;
    }
};
