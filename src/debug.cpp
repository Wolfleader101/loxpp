#include "debug.hpp"

#include <iomanip>
#include <iostream>

#include "value.hpp"

static size_t simpleInstruction(const std::string& name, size_t offset);
static size_t constantInstruction(const std::string& name, const Chunk& chunk, size_t offset);

void disassembleChunk(const Chunk& chunk, const std::string& name)
{
    std::cout << "== " << name << " ==\n";

    for (size_t offset = 0; offset < chunk.code.size();)
    {
        offset = disassembleInstruction(chunk, offset);
    }
}

size_t disassembleInstruction(const Chunk& chunk, size_t offset)
{
    std::cout << std::setfill('0') << std::setw(4) << offset << " ";

    if (offset > 0 && chunk.lines[offset] == chunk.lines[offset - 1])
    {
        std::cout << "   | ";
    }
    else
    {
        std::cout << std::setfill(' ') << std::setw(4) << chunk.lines[offset] << " ";
    }

    uint8_t instruction = chunk.code[offset];
    switch (instruction)
    {
    case OP_CONSTANT:
        return constantInstruction("OP_CONSTANT", chunk, offset);
    case OP_RETURN:
        return simpleInstruction("OP_RETURN", offset);
    default:
        std::cout << "Unknown opcode " << instruction << std::endl;
        return offset + 1;
    }
}

static size_t simpleInstruction(const std::string& name, size_t offset)
{
    std::cout << name << std::endl;
    return offset + 1;
}

static size_t constantInstruction(const std::string& name, const Chunk& chunk, size_t offset)
{
    uint8_t constant = chunk.code[offset + 1];

    std::cout << std::left << std::setw(16) << std::setfill(' ') << name << " " << std::right << std::setw(4)
              << std::setfill(' ') << static_cast<int>(constant) << " '";
    Value val = chunk.constants[constant];
    printValue(val);
    std::cout << "'" << std::endl;

    return offset + 2;
}