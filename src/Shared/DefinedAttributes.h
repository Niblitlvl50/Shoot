
#pragma once

#include "Hash.h"
#include "ObjectAttribute.h"
#include <vector>
#include <array>

namespace world
{
    struct HashString
    {
        HashString(const char* string, const Variant& default_attribute)
            : hash(mono::Hash(string)),
              string(string),
              default_attribute(default_attribute)
        { }

        const unsigned int hash;
        const char* string;
        const Variant default_attribute;
    };

    static const std::array<HashString, 7> hash_array = {{
        HashString("position",          Variant(math::zeroVec)),
        HashString("rotation",          Variant(0.0f)),
        HashString("radius",            Variant(1.0f)),
        HashString("time_stamp",        Variant(5)),
        HashString("spawn_tag",         Variant("")),
        HashString("filepath",          Variant("")),
        HashString("trigger_radius",    Variant(10.0f))
    }};

    static const unsigned int POSITION_ATTRIBUTE        = hash_array[0].hash;
    static const unsigned int ROTATION_ATTRIBUTE        = hash_array[1].hash;
    static const unsigned int RADIUS_ATTRIBUTE          = hash_array[2].hash;
    static const unsigned int TIME_STAMP_ATTRIBUTE      = hash_array[3].hash;
    static const unsigned int SPAWN_TAG_ATTRIBUTE       = hash_array[4].hash;
    static const unsigned int FILEPATH_ATTRIBUTE        = hash_array[5].hash;
    static const unsigned int TRIGGER_RADIUS_ATTRIBUTE  = hash_array[6].hash;
    
    inline const char* NameFromHash(unsigned int hash)
    {
        for(const HashString& hash_string : hash_array)
        {
            if(hash_string.hash == hash)
                return hash_string.string;
        }

        return "Unknown hash";
    }

    inline const Variant& DefaultAttributeFromHash(unsigned int hash)
    {
        for(const HashString& hash_string : hash_array)
        {
            if(hash_string.hash == hash)
                return hash_string.default_attribute;
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
