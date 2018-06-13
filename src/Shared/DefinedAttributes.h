
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

        const unsigned int hash;
        const char* string;
        const Variant default_value;
    };

    static const std::array<DefaultAttribute, 9> default_attributes = {{
        DefaultAttribute("position",          Variant(math::ZeroVec)),
        DefaultAttribute("rotation",          Variant(0.0f)),
        DefaultAttribute("radius",            Variant(1.0f)),
        DefaultAttribute("time_stamp",        Variant(5)),
        DefaultAttribute("spawn_tag",         Variant("")),
        DefaultAttribute("filepath",          Variant("")),
        DefaultAttribute("trigger_radius",    Variant(10.0f)),
        DefaultAttribute("amount",            Variant(10)),
        DefaultAttribute("pickup_type",       Variant(0))
    }};

    static const unsigned int POSITION_ATTRIBUTE        = default_attributes[0].hash;
    static const unsigned int ROTATION_ATTRIBUTE        = default_attributes[1].hash;
    static const unsigned int RADIUS_ATTRIBUTE          = default_attributes[2].hash;
    static const unsigned int TIME_STAMP_ATTRIBUTE      = default_attributes[3].hash;
    static const unsigned int SPAWN_TAG_ATTRIBUTE       = default_attributes[4].hash;
    static const unsigned int FILEPATH_ATTRIBUTE        = default_attributes[5].hash;
    static const unsigned int TRIGGER_RADIUS_ATTRIBUTE  = default_attributes[6].hash;
    static const unsigned int AMOUNT_ATTRIBUTE          = default_attributes[7].hash;
    static const unsigned int PICKUP_TYPE_ATTRIBUTE     = default_attributes[8].hash;
    
    constexpr const char* PickupItems[] = {
        "ammo",
        "health"
    };

    inline const char* AttributeNameFromHash(unsigned int hash)
    {
        for(const DefaultAttribute& hash_string : default_attributes)
        {
            if(hash_string.hash == hash)
                return hash_string.string;
        }

        return "Unknown hash";
    }

    inline const Variant& DefaultAttributeFromHash(unsigned int hash)
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
    inline bool FindAttribute(unsigned int id, const std::vector<Attribute>& attributes, T& value)
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

    inline bool FindAttribute(unsigned int id, std::vector<Attribute>& attributes, Attribute*& output)
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
    inline void SetAttribute(unsigned int id, std::vector<Attribute>& attributes, const T& value)
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
