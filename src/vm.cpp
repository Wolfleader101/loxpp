#include "vm.hpp"

#include <iostream>

#include "common.hpp"
#include "compiler.hpp"
#include "debug.hpp"

InterpretResult VM::interpret(Chunk* chunk)
{
    m_currentChunk = chunk;
    m_instructionPointer = m_currentChunk->code.data();

    return run();
}

InterpretResult VM::interpret(const std::string& source)
{
    Chunk chunk;
    Compiler compiler;

    if (!compiler.compile(source, &chunk))
    {
        return INTERPRET_COMPILE_ERROR;
    }

    m_currentChunk = &chunk;
    m_instructionPointer = m_currentChunk->code.data();

    InterpretResult result = run();

    return result;
}

InterpretResult VM::run()
{
#define READ_BYTE() (*m_instructionPointer++)
#define READ_CONSTANT() (m_currentChunk->constants[READ_BYTE()])
#define BINARY_OP(op)     \
    do                    \
    {                     \
        double b = pop(); \
        double a = pop(); \
        push(a op b);     \
    } while (false)

    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        printStack();
        disassembleInstruction(*m_currentChunk, (int)(m_instructionPointer - m_currentChunk->code.data()));
#endif
        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
        case OP_CONSTANT: {
            Value constant = READ_CONSTANT();
            push(constant);
            std::cout << std::endl;
            break;
        }
        case OP_ADD:
            BINARY_OP(+);
            break;
        case OP_SUBTRACT:
            BINARY_OP(-);
            break;
        case OP_MULTIPLY:
            BINARY_OP(*);
            break;
        case OP_DIVIDE:
            BINARY_OP(/);
            break;
        case OP_NEGATE:
            push(-pop());
            break;

        case OP_RETURN: {
            printValue(pop());
            std::cout << std::endl;
            return INTERPRET_OK;
        }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

void VM::printStack()
{
    std::cout << "          ";

    std::stack<Value> tempStack;

    // transfer the stack to a temporary stack and print the values
    while (!m_stack.empty())
    {
        Value value = m_stack.top();
        m_stack.pop();
        tempStack.push(value);

        std::cout << "[ ";
        printValue(value);
        std::cout << " ]";
    }

    while (!tempStack.empty())
    {
        m_stack.push(tempStack.top());
        tempStack.pop();
    }

    std::cout << std::endl;
}