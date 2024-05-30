#include "chunk.hpp"
#include "debug.hpp"

int main(int argc, char* argv[])
{
    Chunk chunk = Chunk();
    auto constant = chunk.addConstant(1.2);
    chunk.writeChunk(OP_CONSTANT, 123);
    chunk.writeChunk(constant, 123);

    chunk.writeChunk(OP_RETURN, 123);

    disassembleChunk(chunk, "test chunk");

    return 0;
}