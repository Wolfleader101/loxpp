#include "vm.hpp"

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
    Compiler compiler(*this);

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
#define READ_SHORT() (m_instructionPointer += 2, (uint16_t)((m_instructionPointer[-2] << 8) | m_instructionPointer[-1]))
#define READ_CONSTANT() (m_currentChunk->constants[READ_BYTE()])
#define BINARY_OP(op)                                   \
    do                                                  \
    {                                                   \
        if (!peek(0).isNumber() || !peek(1).isNumber()) \
        {                                               \
            runtimeError("Operands must be numbers.");  \
            return INTERPRET_RUNTIME_ERROR;             \
        }                                               \
        double b = pop().asNumber();                    \
        double a = pop().asNumber();                    \
        push(Value(a op b));                            \
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
        case OP_NIL:
            push(Value(nullptr));
            break;
        case OP_TRUE:
            push(Value(true));
            break;
        case OP_FALSE:
            push(Value(false));
            break;
        case OP_POP:
            pop();
            break;
        case OP_GET_LOCAL: {
            uint8_t slot = READ_BYTE();
            push(m_stack[slot]);
            break;
        }
        case OP_SET_LOCAL: {
            uint8_t slot = READ_BYTE();
            m_stack[slot] = peek(0);
            break;
        }
        case OP_GET_GLOBAL: {
            std::shared_ptr<ObjString> name = READ_CONSTANT().asString();
            Value value;
            try
            {
                value = m_globals[name->str];
            }
            catch (std::out_of_range)
            {
                runtimeError("Undefined variable '%s'.", name->str.c_str());
                return INTERPRET_RUNTIME_ERROR;
            }

            push(value);
            break;
        }
        case OP_DEFINE_GLOBAL: {
            std::shared_ptr<ObjString> name = READ_CONSTANT().asString();
            m_globals[name->str] = peek(0);
            pop();
            break;
        }
        case OP_SET_GLOBAL: {
            std::shared_ptr<ObjString> name = READ_CONSTANT().asString();
            if (m_globals.find(name->str) == m_globals.end())
            {
                runtimeError("Undefined variable '%s'.", name->str.c_str());
                return INTERPRET_RUNTIME_ERROR;
            }

            m_globals[name->str] = peek(0);
            break;
        }
        case OP_EQUAL: {
            Value b = pop();
            Value a = pop();
            push(Value(a == b));
            break;
        }
        case OP_ADD:
            if (peek(0).isString() && peek(1).isString())
            {
                concactenate();
            }
            else if (peek(0).isNumber() && peek(1).isNumber())
            {
                BINARY_OP(+);
            }
            else
            {
                runtimeError("Operands must be two numbers or two strings.");
                return INTERPRET_RUNTIME_ERROR;
            }
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
        case OP_NOT:
            push(Value(isFalsey(pop())));
            break;
        case OP_NEGATE:
            if (!peek(0).isNumber())
            {
                runtimeError("Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }
            push(-pop().asNumber());
            break;
        case OP_PRINT: {
            printValue(pop());
            std::cout << std::endl;
        }
        case OP_JUMP: {
            uint16_t offset = READ_SHORT();
            m_instructionPointer += offset;
            break;
        }
        case OP_JUMP_IF_FALSE: {
            uint16_t offset = READ_SHORT();
            if (isFalsey(peek(0)))
            {
                m_instructionPointer += offset;
            }
            break;
        }
        case OP_RETURN:
            return INTERPRET_OK;
        }
    }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef BINARY_OP
}

void VM::printStack()
{
    std::cout << "          ";

    for (const Value& value : m_stack)
    {
        std::cout << "[ ";
        printValue(value);
        std::cout << " ]";
    }

    std::cout << std::endl;
}

Value VM::peek(int distance)
{
    return m_stack[m_stack.size() - 1 - distance];
}

bool VM::isFalsey(const Value& value)
{
    return value.isNil() || (value.isBool() && !value.asBool());
}

void VM::concactenate()
{
    std::shared_ptr<ObjString> b = pop().asString();
    std::shared_ptr<ObjString> a = pop().asString();

    std::string result = a->str + b->str;
    m_objects.push_back(std::make_shared<ObjString>(result));

    push(Value(m_objects.back()));
}