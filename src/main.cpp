#include "common.hpp"

#include "chunk.hpp"
#include "debug.hpp"

int main(int argc, char* argv[])
{
    Chunk chunk;
    chunk.push_back(OP_RETURN);
    disassembleChunk(chunk, "test chunk");

    return 0;
}