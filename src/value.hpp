#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

enum ValueType
{
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ
};

enum ObjType
{
    OBJ_STRING,
};

struct Obj
{
    ObjType type;

    virtual ~Obj() = default;

    bool isString() const
    {
        return type == OBJ_STRING;
    }

  protected:
    Obj(ObjType type) : type(type)
    {
    }
};

struct ObjString : Obj
{
    std::string str;

    ObjString() : str(), Obj(OBJ_STRING)
    {
    }

    ObjString(const char* str, int length) : Obj(OBJ_STRING), str(str, length)
    {
    }

    ObjString(const std::string& str) : Obj(OBJ_STRING), str(str)
    {
    }
};
struct Value
{
    ValueType type;
    std::variant<std::monostate, bool, double, std::shared_ptr<Obj>> as;

    Value() : type(VAL_NIL), as(std::monostate{})
    {
    }

    Value(bool value) : type(VAL_BOOL), as(value)
    {
    }
    Value(double value) : type(VAL_NUMBER), as(value)
    {
    }

    Value(std::nullptr_t) : type(VAL_NIL), as(std::monostate{})
    {
    }

    Value(std::shared_ptr<Obj> value) : type(VAL_OBJ), as(value)
    {
    }

    bool isNil() const
    {
        return std::holds_alternative<std::monostate>(as);
    }
    bool isBool() const
    {
        return std::holds_alternative<bool>(as);
    }
    bool isNumber() const
    {
        return std::holds_alternative<double>(as);
    }
    bool isObj() const
    {
        return std::holds_alternative<std::shared_ptr<Obj>>(as);
    }
    bool isString() const
    {
        return isObj() && asObj()->isString();
    }

    bool asBool() const
    {
        return std::get<bool>(as);
    }
    double asNumber() const
    {
        return std::get<double>(as);
    }
    std::shared_ptr<Obj> asObj() const
    {
        return std::get<std::shared_ptr<Obj>>(as);
    }

    std::shared_ptr<ObjString> asString() const
    {
        return std::dynamic_pointer_cast<ObjString>(asObj());
    }

    bool operator==(const Value& other) const
    {
        if (type != other.type)
        {
            return false;
        }

        if (isString() && other.isString())
        {
            return asString()->str == other.asString()->str;
        }

        return as == other.as;
    }
};

void printValue(Value value);
void printObject(Value value);