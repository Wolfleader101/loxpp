#pragma once

#include <stack>
#include <string>

#include "chunk.hpp"
#include "value.hpp"

enum InterpretResult
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
};

class VM
{
  public:
    VM() = default;

    InterpretResult interpret(const std::string& source);
    InterpretResult interpret(Chunk* chunk);

    void push(Value value)
    {
        m_stack.push(value);
    }
    Value pop()
    {
        Value value = m_stack.top();
        m_stack.pop();
        return value;
    }

  private:
    std::stack<Value> m_stack;
    Chunk* m_currentChunk;
    uint8_t* m_instructionPointer;

    InterpretResult run();

    void printStack();
};