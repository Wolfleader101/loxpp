#include "value.hpp"

#include <iostream>

void printValue(Value value)
{
    switch (value.type)
    {
    case VAL_BOOL:
        std::cout << (value.asBool() ? "true" : "false");
        break;
    case VAL_NIL:
        std::cout << "nil";
        break;
    case VAL_OBJ:
        printObject(value);
        break;
    case VAL_NUMBER:
        std::cout << value.asNumber();
        break;
    }
}

void printObject(Value value)
{
    switch (value.asObj()->type)
    {
    case OBJ_STRING:
        std::cout << value.asString()->str;
        break;
    }
}