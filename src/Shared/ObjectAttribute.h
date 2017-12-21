
#pragma once

#include "Math/Vector.h"
#include <assert.h>
#include <algorithm>
#include <cstring>

class ObjectAttribute
{
public:

    enum class Type
    {
        NONE,
        INT,
        FLOAT,
        STRING,
        POINT
    };

    union Data
    {
        Data()
        { }

        int int_value;
        float float_value;
        char string_value[24];
        math::Vector point_value;
    };

    ObjectAttribute()
    { }

    ObjectAttribute(int value)
    {
        *this = value;
    }

    ObjectAttribute(float value)
    {
        *this = value;
    }

    ObjectAttribute(const char* string)
    {
        *this = string;
    }

    ObjectAttribute(const math::Vector& point)
    {
        *this = point;
    }

    operator int() const
    {
        assert(type == Type::INT);
        return data.int_value;
    }

    operator float() const
    {
        assert(type == Type::FLOAT);
        return data.float_value;
    }

    operator const char*() const
    {
        assert(type == Type::STRING);
        return data.string_value;
    }

    operator math::Vector() const
    {
        assert(type == Type::POINT);
        return data.point_value;
    }

    ObjectAttribute& operator = (const int value)
    {
        type = Type::INT;
        data.int_value = value;
        return *this;
    }

    ObjectAttribute& operator = (const float value)
    {
        type = Type::FLOAT;
        data.float_value = value;
        return *this;
    }

    ObjectAttribute& operator = (const char* string)
    {
        type = Type::STRING;

        const size_t length = std::strlen(string);
        const size_t data_length = std::min(length, sizeof(Data) -1);

        std::memcpy(data.string_value, string, data_length);
        data.string_value[data_length] = '\0';

        return *this;
    }

    ObjectAttribute& operator = (const math::Vector value)
    {
        type = Type::POINT;
        data.point_value = value;
        return *this;
    }

    Type type = Type::NONE;
    Data data;
};

struct ID_Attribute
{
    unsigned int id = 0;
    ObjectAttribute attribute;
};
