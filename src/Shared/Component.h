
#pragma once

#include "ObjectAttribute.h"
#include "Util/Hash.h"
#include <string>
#include <vector>
#include <algorithm>
#include <array>


struct DefaultAttribute
{
    DefaultAttribute(const char* string, const Variant& default_value)
        : hash(mono::Hash(string))
        , string(string)
        , default_value(default_value)
    { }

    const uint32_t hash;
    const char* string;
    const Variant default_value;
};

extern const std::array<DefaultAttribute, 25> default_attributes;

extern const uint32_t POSITION_ATTRIBUTE;
extern const uint32_t ROTATION_ATTRIBUTE;
extern const uint32_t RADIUS_ATTRIBUTE;
extern const uint32_t TIME_STAMP_ATTRIBUTE;
extern const uint32_t SPAWN_TAG_ATTRIBUTE;
extern const uint32_t PATH_FILE_ATTRIBUTE;
extern const uint32_t TRIGGER_RADIUS_ATTRIBUTE;
extern const uint32_t COLOR_ATTRIBUTE;

extern const uint32_t PICKUP_TYPE_ATTRIBUTE;
extern const uint32_t AMOUNT_ATTRIBUTE;

extern const uint32_t BODY_TYPE_ATTRIBUTE;
extern const uint32_t MASS_ATTRIBUTE;
extern const uint32_t INERTIA_ATTRIBUTE;
extern const uint32_t PREVENT_ROTATION_ATTRIBUTE;

extern const uint32_t FACTION_ATTRIBUTE;
extern const uint32_t WIDTH_ATTRIBUTE;
extern const uint32_t HEIGHT_ATTRIBUTE;
extern const uint32_t START_ATTRIBUTE;
extern const uint32_t END_ATTRIBUTE;

extern const uint32_t HEALTH_ATTRIBUTE;
extern const uint32_t SPRITE_ATTRIBUTE;
extern const uint32_t ANIMATION_ATTRIBUTE;
extern const uint32_t FLIP_VERTICAL_ATTRIBUTE;
extern const uint32_t FLIP_HORIZONTAL_ATTRIBUTE;
extern const uint32_t ENTITY_BEHAVIOUR_ATTRIBUTE;

const char* AttributeNameFromHash(uint32_t hash);
const Variant& DefaultAttributeFromHash(uint32_t hash);
bool FindAttribute(uint32_t id, std::vector<Attribute>& attributes, Attribute*& output);
void MergeAttributes(std::vector<Attribute>& result_attributes, const std::vector<Attribute>& other_attributes);
void UnionAttributes(std::vector<Attribute>& result_attributes, const std::vector<Attribute>& other_attributes);

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


struct Component
{
    uint32_t hash;
    std::string name;
    std::vector<Attribute> properties;
};

extern const std::array<Component, 8> default_components;

extern const uint32_t TRANSFORM_COMPONENT;
extern const uint32_t SPRITE_COMPONENT;
extern const uint32_t PHYSICS_COMPONENT;
extern const uint32_t CIRCLE_SHAPE_COMPONENT;
extern const uint32_t BOX_SHAPE_COMPONENT;
extern const uint32_t SEGMENT_SHAPE_COMPONENT;
extern const uint32_t HEALTH_COMPONENT;
extern const uint32_t BEHAVIOUR_COMPONENT;

Component MakeDefaultComponent(const char* name, const std::vector<uint32_t>& properties);
Component DefaultComponentFromHash(uint32_t hash);
void StripUnknownProperties(Component& component);
