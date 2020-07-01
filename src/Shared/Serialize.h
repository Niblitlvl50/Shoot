
#pragma once

#include "ObjectAttribute.h"
#include "Math/Serialize.h"
#include "Rendering/Serialize.h"
#include "nlohmann/json.hpp"

inline void to_json(nlohmann::json& json, const Attribute& attribute)
{
    json["id"] = attribute.id;
    json["variant_type"] = attribute.attribute.type;

    switch(attribute.attribute.type)
    {
    case Variant::Type::NONE:
        json["variant_value"] = nullptr;
        break;
    case Variant::Type::BOOL:
        json["variant_value"] = (bool)attribute.attribute.int_value;
        break;
    case Variant::Type::INT:
        json["variant_value"] = attribute.attribute.int_value;
        break;
    case Variant::Type::UINT:
        json["variant_value"] = attribute.attribute.uint_value;
        break;
    case Variant::Type::FLOAT:
        json["variant_value"] = attribute.attribute.float_value;
        break;
    case Variant::Type::STRING:
        json["variant_value"] = (const char*)attribute.attribute;
        break;
    case Variant::Type::POINT:
        json["variant_value"] = attribute.attribute.point_value;
        break;
    case Variant::Type::COLOR:
        json["variant_value"] = attribute.attribute.color_value;
        break;
    }
}

inline void from_json(const nlohmann::json& json, Attribute& attribute)
{
    attribute.id = json["id"].get<uint32_t>();
    attribute.attribute.type = Variant::Type(json["variant_type"].get<short>());

    switch(attribute.attribute.type)
    {
    case Variant::Type::NONE:
        break;
    case Variant::Type::BOOL:
        attribute.attribute = json["variant_value"].get<bool>();
        break;
    case Variant::Type::INT:
        attribute.attribute = json["variant_value"].get<int>();
        break;
    case Variant::Type::UINT:
        attribute.attribute = json["variant_value"].get<uint32_t>();
        break;
    case Variant::Type::FLOAT:
        attribute.attribute = json["variant_value"].get<float>();
        break;
    case Variant::Type::STRING:
    {
        const std::string& string_value = json["variant_value"].get<std::string>();
        attribute.attribute = string_value.c_str();
        break;
    }
    case Variant::Type::POINT:
        attribute.attribute = json["variant_value"].get<math::Vector>();
        break;
    case Variant::Type::COLOR:
        attribute.attribute = json["variant_value"].get<mono::Color::RGBA>();
        break;
    }
}
