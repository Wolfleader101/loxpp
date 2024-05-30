#include "debug.hpp"

#include <iomanip>
#include <iostream>

static size_t simpleInstruction(const std::string& name, size_t offset);

void disassembleChunk(const Chunk& chunk, const std::string& name)
{
    std::cout << "== " << name << " ==\n";

    for (size_t offset = 0; offset < chunk.size(); offset++)
    {
        offset = disassembleInstruction(chunk, offset);
    }
}

size_t disassembleInstruction(const Chunk& chunk, size_t offset)
{
    std::cout << std::setfill('0') << std::setw(4) << offset << " ";

    // if (offset > 0 && chunk[offset] == chunk[offset - 1]) {
    //     std::cout << "|\n";
    // }

    uint8_t instruction = chunk[offset];
    switch (instruction)
    {
    case OP_RETURN:
        return simpleInstruction("OP_RETURN", offset);
    default:
        std::cout << "Unknown opcode " << instruction << "\n";
        return offset + 1;
    }
}

static size_t simpleInstruction(const std::string& name, size_t offset)
{
    std::cout << name << "\n";
    return offset + 1;
}