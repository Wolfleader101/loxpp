#include "common.hpp"

#include "chunk.hpp"
#include "debug.hpp"
#include "vm.hpp"
#include <fstream>
#include <iostream>

static void repl(VM& vm);
static void runFile(const char* path, VM& vm);

int main(int argc, char* argv[])
{
    VM vm = VM();

    if (argc == 1)
    {
        repl(vm);
    }
    else if (argc == 2)
    {
        runFile(argv[1], vm);
    }
    else
    {
        std::cerr << "Usage: cpplox [path]" << std::endl;
        exit(64);
    }

    Chunk chunk = Chunk();
    auto constant = chunk.addConstant(1.2);
    chunk.writeChunk(OP_CONSTANT, 123);
    chunk.writeChunk(constant, 123);

    constant = chunk.addConstant(3.4);
    chunk.writeChunk(OP_CONSTANT, 123);
    chunk.writeChunk(constant, 123);

    chunk.writeChunk(OP_ADD, 123);

    constant = chunk.addConstant(5.6);
    chunk.writeChunk(OP_CONSTANT, 123);
    chunk.writeChunk(constant, 123);

    chunk.writeChunk(OP_DIVIDE, 123);
    chunk.writeChunk(OP_NEGATE, 123);

    chunk.writeChunk(OP_RETURN, 123);

    disassembleChunk(chunk, "test chunk");

    vm.interpret(&chunk);

    return 0;
}

static void runFile(const char* path, VM& vm)
{
    std::ifstream file_stream(path);

    if (file_stream.is_open())
    {
        std::string file_contents((std::istreambuf_iterator<char>(file_stream)), (std::istreambuf_iterator<char>()));
        file_stream.close();

        InterpretResult result = vm.interpret(file_contents);

        if (result == InterpretResult::INTERPRET_COMPILE_ERROR)
        {
            exit(65);
        }

        if (result == InterpretResult::INTERPRET_RUNTIME_ERROR)
        {
            exit(70);
        }
    }
    else
    {
        std::cerr << "Error: Unable to open the file." << std::endl;
    }
}

static void repl(VM& vm)
{
    for (;;)
    {
        std::cout << "> ";
        std::string line;
        std::getline(std::cin, line);

        if (std::cin.eof())
        {
            std::cout << std::endl;
            break;
        }

        vm.interpret(line);
    }
}