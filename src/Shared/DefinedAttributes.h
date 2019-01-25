
#pragma once

#include "Hash.h"
#include "ObjectAttribute.h"
#include <vector>
#include <array>

namespace world
{
    struct DefaultAttribute
    {
        DefaultAttribute(const char* string, const Variant& default_value)
            : hash(mono::Hash(string)),
              string(string),
              default_value(default_value)
        { }

        const uint32_t hash;
        const char* string;
        const Variant default_value;
    };

    static const std::array<DefaultAttribute, 19> default_attributes = {{
        DefaultAttribute("position",        Variant(math::ZeroVec)),
        DefaultAttribute("rotation",        Variant(0.0f)),
        DefaultAttribute("radius",          Variant(1.0f)),
        DefaultAttribute("time_stamp",      Variant(5)),
        DefaultAttribute("spawn_tag",       Variant("")),
        DefaultAttribute("filepath",        Variant("")),
        DefaultAttribute("trigger_radius",  Variant(10.0f)),
        DefaultAttribute("color",           Variant(mono::Color::BLACK)),

        DefaultAttribute("pickup_type",     Variant(0)),
        DefaultAttribute("amount",          Variant(10)),

        DefaultAttribute("body_type",       Variant(0)),
        DefaultAttribute("mass",            Variant(10.0f)),
        DefaultAttribute("inertia",         Variant(1.0f)),

        DefaultAttribute("shape_type",      Variant(0)),
        DefaultAttribute("width",           Variant(1.0f)),
        DefaultAttribute("height",          Variant(1.0f)),
        DefaultAttribute("start",           Variant(math::ZeroVec)),
        DefaultAttribute("end",             Variant(math::ZeroVec)),

        DefaultAttribute("health",          Variant(100))
    }};

    static const uint32_t POSITION_ATTRIBUTE        = default_attributes[0].hash;
    static const uint32_t ROTATION_ATTRIBUTE        = default_attributes[1].hash;
    static const uint32_t RADIUS_ATTRIBUTE          = default_attributes[2].hash;
    static const uint32_t TIME_STAMP_ATTRIBUTE      = default_attributes[3].hash;
    static const uint32_t SPAWN_TAG_ATTRIBUTE       = default_attributes[4].hash;
    static const uint32_t FILEPATH_ATTRIBUTE        = default_attributes[5].hash;
    static const uint32_t TRIGGER_RADIUS_ATTRIBUTE  = default_attributes[6].hash;
    static const uint32_t COLOR_ATTRIBUTE           = default_attributes[7].hash;

    static const uint32_t PICKUP_TYPE_ATTRIBUTE     = default_attributes[8].hash;
    static const uint32_t AMOUNT_ATTRIBUTE          = default_attributes[9].hash;

    static const uint32_t BODY_TYPE_ATTRIBUTE       = default_attributes[10].hash;
    static const uint32_t MASS_ATTRIBUTE            = default_attributes[11].hash;
    static const uint32_t INERTIA_ATTRIBUTE         = default_attributes[12].hash;
    
    static const uint32_t SHAPE_TYPE_ATTRIBUTE      = default_attributes[13].hash;
    static const uint32_t WIDTH_ATTRIBUTE           = default_attributes[14].hash;
    static const uint32_t HEIGHT_ATTRIBUTE          = default_attributes[15].hash;
    static const uint32_t START_ATTRIBUTE           = default_attributes[16].hash;
    static const uint32_t END_ATTRIBUTE             = default_attributes[17].hash;
    
    static const uint32_t HEALTH_ATTRIBUTE          = default_attributes[18].hash;
    
    constexpr const char* PickupItems[] = {
        "ammo",
        "health"
    };

    inline const char* AttributeNameFromHash(uint32_t hash)
    {
        for(const DefaultAttribute& hash_string : default_attributes)
        {
            if(hash_string.hash == hash)
                return hash_string.string;
        }

        return "Unknown hash";
    }

    inline const Variant& DefaultAttributeFromHash(uint32_t hash)
    {
        for(const DefaultAttribute& hash_string : default_attributes)
        {
            if(hash_string.hash == hash)
                return hash_string.default_value;
        }

        static const Variant null_attribute;
        return null_attribute;
    }

    template <typename T>
    inline bool FindAttribute(uint32_t id, const std::vector<Attribute>& attributes, T& value)
    {
        const auto find_func = [id](const Attribute& attribute) {
            return id == attribute.id;
        };

        const auto it = std::find_if(attributes.begin(), attributes.end(), find_func);
        const bool found_attribute = (it != attributes.end());
        if(found_attribute)
            value = it->attribute;

        return found_attribute;
    }

    inline bool FindAttribute(uint32_t id, std::vector<Attribute>& attributes, Attribute*& output)
    {
        const auto find_func = [id](const Attribute& attribute) {
            return id == attribute.id;
        };

        const auto it = std::find_if(attributes.begin(), attributes.end(), find_func);
        const bool found_attribute = (it != attributes.end());
        if(found_attribute)
            output = &*it;

        return found_attribute;
    }

    template <typename T>
    inline void SetAttribute(uint32_t id, std::vector<Attribute>& attributes, const T& value)
    {
        const auto find_func = [id](const Attribute& attribute) {
            return id == attribute.id;
        };

        const auto it = std::find_if(attributes.begin(), attributes.end(), find_func);
        const bool found_attribute = (it != attributes.end());
        if(found_attribute)
            it->attribute = value;
    }

    inline void MergeAttributes(
        std::vector<Attribute>& result_attributes, const std::vector<Attribute>& other_attributes)
    {
        for(const Attribute& input : other_attributes)
        {
            Attribute* attribute = nullptr;
            if(FindAttribute(input.id, result_attributes, attribute))
                attribute->attribute = input.attribute;
        }
    }
}
