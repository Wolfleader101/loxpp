#pragma once

#include <vector>

#include "value.hpp"

enum OpCode
{
    OP_CONSTANT,
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
