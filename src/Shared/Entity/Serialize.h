
#pragma once

#include "EntitySystem/ObjectAttribute.h"
#include "Component.h"
#include "Math/Serialize.h"
#include "Rendering/Serialize.h"
#include "nlohmann/json.hpp"

inline void to_json(nlohmann::json& json, const Attribute& attribute)
{
    json["id"] = attribute.id;
    json["name"] = AttributeNameFromHash(attribute.id);
    json["variant_type"] = attribute.value.index();

    const auto visitor = [&json](const auto& value)
    {
        json["variant_value"] = value;
    };
    std::visit(visitor, attribute.value);
}

inline void from_json(const nlohmann::json& json, Attribute& attribute)
{
    attribute.id = json["id"].get<uint32_t>();

    const uint32_t value_index = json["variant_type"].get<int>();
    switch(value_index)
    {
    case VariantTypeIndex::BOOL:
        attribute.value = json["variant_value"].get<bool>();
        break;
    case VariantTypeIndex::INT:
        attribute.value = json["variant_value"].get<int>();
        break;
    case VariantTypeIndex::UINT:
        attribute.value = json["variant_value"].get<uint32_t>();
        break;
    case VariantTypeIndex::FLOAT:
        attribute.value = json["variant_value"].get<float>();
        break;
    case VariantTypeIndex::POINT:
        attribute.value = json["variant_value"].get<math::Vector>();
        break;
    case VariantTypeIndex::COLOR:
        attribute.value = json["variant_value"].get<mono::Color::RGBA>();
        break;
    case VariantTypeIndex::STRING:
        attribute.value = json["variant_value"].get<std::string>();
        break;
    case VariantTypeIndex::POLYGON:
        attribute.value = json["variant_value"].get<std::vector<math::Vector>>();
        break;
    }
}
