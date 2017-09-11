
#pragma once

#include "Hash.h"
#include "ObjectAttribute.h"
#include <vector>
#include <array>

namespace world
{
    struct HashString
    {
        HashString(const char* string, const ObjectAttribute& default_attribute)
            : hash(mono::Hash(string)),
              string(string),
              default_attribute(default_attribute)
        { }

        const unsigned int hash;
        const char* string;
        const ObjectAttribute default_attribute;
    };

    static const std::array<HashString, 5> hash_array = {{
        HashString("position",  ObjectAttribute(math::zeroVec)),
        HashString("rotation",  ObjectAttribute(0.0f)),
        HashString("radius",    ObjectAttribute(1.0f)),
        HashString("interval",  ObjectAttribute(1)),
        HashString("spawn_tag", ObjectAttribute(""))
    }};

    static const unsigned int POSITION_ATTRIBUTE     = mono::Hash("position");
    static const unsigned int ROTATION_ATTRIBUTE     = mono::Hash("rotation");
    static const unsigned int RADIUS_ATTRIBUTE       = mono::Hash("radius");
    static const unsigned int INTERVAL_ATTRIBUTE     = mono::Hash("interval");
    static const unsigned int SPAWN_TAG_ATTRIBUTE    = mono::Hash("spawn_tag");

    inline const char* NameFromHash(unsigned int hash)
    {
        for(const HashString& hash_string : hash_array)
        {
            if(hash_string.hash == hash)
                return hash_string.string;
        }

        return "Unknown hash";
    }

    inline const ObjectAttribute& AttributeFromHash(unsigned int hash)
    {
        for(const HashString& hash_string : hash_array)
        {
            if(hash_string.hash == hash)
                return hash_string.default_attribute;
        }

        static const ObjectAttribute null_attribute;
        return null_attribute;
    }

    template <typename T>
    inline bool FindAttribute(unsigned int id, const std::vector<ID_Attribute>& attributes, T& value)
    {
        const auto find_func = [id](const ID_Attribute& attribute) {
            return id == attribute.id;
        };

        const auto it = std::find_if(attributes.begin(), attributes.end(), find_func);
        const bool found_attribute = (it != attributes.end());
        if(found_attribute)
            value = it->attribute;

        return found_attribute;
    }

    template <typename T>
    inline void SetAttribute(unsigned int id, std::vector<ID_Attribute>& attributes, const T& value)
    {
        const auto find_func = [id](const ID_Attribute& attribute) {
            return id == attribute.id;
        };

        const auto it = std::find_if(attributes.begin(), attributes.end(), find_func);
        const bool found_attribute = (it != attributes.end());
        if(found_attribute)
            it->attribute = value;        
    }

    inline bool FindAttribute(unsigned int id, const std::vector<ID_Attribute>& attributes, ID_Attribute& output)
    {
        const auto find_func = [id](const ID_Attribute& attribute) {
            return id == attribute.id;
        };

        const auto it = std::find_if(attributes.begin(), attributes.end(), find_func);
        const bool found_attribute = (it != attributes.end());
        if(found_attribute)
            output = *it;

        return found_attribute;        
    }
}
