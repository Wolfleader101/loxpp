#include "debug.hpp"

#include <iomanip>
#include <iostream>

#include "value.hpp"

static size_t simpleInstruction(const std::string& name, size_t offset);
static size_t constantInstruction(const std::string& name, const Chunk& chunk, size_t offset);
static size_t byteInstruction(const std::string& name, const Chunk& chunk, size_t offset);
static size_t jumpInstruction(const std::string& name, int sign, const Chunk& chunk, size_t offset);

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
    case OP_NIL:
        return simpleInstruction("OP_NIL", offset);
    case OP_TRUE:
        return simpleInstruction("OP_TRUE", offset);
    case OP_FALSE:
        return simpleInstruction("OP_FALSE", offset);
    case OP_POP:
        return simpleInstruction("OP_POP", offset);
    case OP_GET_LOCAL:
        return byteInstruction("OP_GET_LOCAL", chunk, offset);
    case OP_SET_LOCAL:
        return byteInstruction("OP_SET_LOCAL", chunk, offset);
    case OP_GET_GLOBAL:
        return constantInstruction("OP_GET_GLOBAL", chunk, offset);
    case OP_DEFINE_GLOBAL:
        return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
    case OP_SET_GLOBAL:
        return constantInstruction("OP_SET_GLOBAL", chunk, offset);
    case OP_EQUAL:
        return simpleInstruction("OP_EQUAL", offset);
    case OP_GREATER:
        return simpleInstruction("OP_GREATER", offset);
    case OP_LESS:
        return simpleInstruction("OP_LESS", offset);
    case OP_ADD:
        return simpleInstruction("OP_ADD", offset);
    case OP_SUBTRACT:
        return simpleInstruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
        return simpleInstruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
        return simpleInstruction("OP_DIVIDE", offset);
    case OP_NOT:
        return simpleInstruction("OP_NOT", offset);
    case OP_NEGATE:
        return simpleInstruction("OP_NEGATE", offset);
    case OP_PRINT:
        return simpleInstruction("OP_PRINT", offset);
    case OP_JUMP:
        return jumpInstruction("OP_JUMP", 1, chunk, offset);
    case OP_JUMP_IF_FALSE:
        return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
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

static size_t byteInstruction(const std::string& name, const Chunk& chunk, size_t offset)
{
    uint8_t slot = chunk.code[offset + 1];

    std::cout << std::left << std::setw(16) << std::setfill(' ') << name << " " << std::right << std::setw(4)
              << std::setfill(' ') << static_cast<int>(slot) << std::endl;

    return offset + 2;
}

static size_t jumpInstruction(const std::string& name, int sign, const Chunk& chunk, size_t offset)
{
    uint16_t jump = static_cast<uint16_t>(chunk.code[offset + 1] << 8);
    jump |= chunk.code[offset + 2];

    std::cout << std::left << std::setw(16) << std::setfill(' ') << name << " " << std::right << std::setw(4)
              << std::setfill(' ') << static_cast<int>(offset) << " -> " << (offset + 3 + sign * jump) << std::endl;

    return offset + 3;
}