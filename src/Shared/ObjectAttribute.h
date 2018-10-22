
#pragma once

#include "Math/Vector.h"
#include <assert.h>
#include <algorithm>
#include <cstring>

constexpr int VariantStringMaxLength = 24;

class Variant
{
public:

    Variant()
    { }

    Variant(int value)
    {
        *this = value;
    }

    Variant(float value)
    {
        *this = value;
    }

    Variant(const char* string)
    {
        *this = string;
    }

    Variant(const math::Vector& point)
    {
        *this = point;
    }

    operator int() const
    {
        assert(type == Type::INT);
        return int_value;
    }

    operator float() const
    {
        assert(type == Type::FLOAT);
        return float_value;
    }

    operator const char*() const
    {
        assert(type == Type::STRING);
        return string_value;
    }

    operator math::Vector() const
    {
        assert(type == Type::POINT);
        return point_value;
    }

    Variant& operator = (const int value)
    {
        type = Type::INT;
        int_value = value;
        return *this;
    }

    Variant& operator = (const float value)
    {
        type = Type::FLOAT;
        float_value = value;
        return *this;
    }

    Variant& operator = (const char* string)
    {
        type = Type::STRING;

        const size_t length = std::strlen(string);
        const size_t data_length = std::min(length, size_t(VariantStringMaxLength - 1));

        std::memcpy(string_value, string, data_length);
        string_value[data_length] = '\0';

        return *this;
    }

    Variant& operator = (const math::Vector& value)
    {
        type = Type::POINT;
        point_value = value;
        return *this;
    }

    enum class Type : short
    {
        NONE,
        INT,
        FLOAT,
        STRING,
        POINT
    };

    Type type = Type::NONE;

    union
    {
        int int_value;
        float float_value;
        char string_value[VariantStringMaxLength];
        math::Vector point_value;
    };
};

struct Attribute
{
    unsigned int id = 0;
    Variant attribute;
};

static_assert(sizeof(Variant) == 28, "Variant size is too big!");
static_assert(sizeof(Attribute) == 32, "Attribute size is too big!");
