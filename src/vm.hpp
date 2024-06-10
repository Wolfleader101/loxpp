#pragma once

#include <format>
#include <iostream>
#include <string>
#include <vector>

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
        m_stack.push_back(value);
    }

    Value pop()
    {
        Value value = m_stack.back();
        m_stack.pop_back();
        return value;
    }

    void insertObject(std::shared_ptr<Obj> object)
    {
        m_objects.push_back(object);
    }

    std::shared_ptr<Obj> getObject(size_t index)
    {
        return m_objects[index];
    }

    std::shared_ptr<Obj> getLastObject()
    {
        return m_objects.back();
    }

  private:
    std::vector<Value> m_stack;
    std::vector<std::shared_ptr<Obj>> m_objects;
    Chunk* m_currentChunk;
    uint8_t* m_instructionPointer;

    InterpretResult run();

    Value peek(int distance);

    template <typename... Args>
    void runtimeError(const std::string& format, Args&&... args)
    {
        std::string message = std::vformat(format, std::make_format_args(std::forward<Args>(args)...));

        std::cout << message << std::endl;

        size_t instruction = m_instructionPointer - m_currentChunk->code.data() - 1;
        int line = m_currentChunk->lines[instruction];
        std::cout << "[line " << line << "] in script" << std::endl;
        resetStack();
    }

    void resetStack()
    {
        m_stack.clear();
    }

    void printStack();
    bool isFalsey(const Value& value);

    void concactenate();
    void freeVM()
    {
        m_objects.clear();
    }
};