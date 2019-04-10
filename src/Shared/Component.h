
#pragma once

#include "ObjectAttribute.h"
#include "DefinedAttributes.h"
#include "Hash.h"
#include <string>
#include <vector>
#include <algorithm>

struct Component
{
    uint32_t hash;
    std::string name;
    std::vector<Attribute> properties;
};

inline Component MakeDefaultComponent(const char* name, const std::vector<uint32_t>& properties)
{
    std::vector<Attribute> attributes;
    for(uint32_t property_hash : properties)
        attributes.push_back({ property_hash, world::DefaultAttributeFromHash(property_hash) });

    return { mono::Hash(name), name, std::move(attributes) };
}

static const std::array<Component, 8> default_components = {
    MakeDefaultComponent("transform",           { world::POSITION_ATTRIBUTE, world::ROTATION_ATTRIBUTE } ),
    MakeDefaultComponent("sprite",              { world::SPRITE_ATTRIBUTE, world::ANIMATION_ATTRIBUTE, world::COLOR_ATTRIBUTE, world::FLIP_VERTICAL_ATTRIBUTE, world::FLIP_HORIZONTAL_ATTRIBUTE } ),
    MakeDefaultComponent("physics",             { world::BODY_TYPE_ATTRIBUTE, world::MASS_ATTRIBUTE, world::INERTIA_ATTRIBUTE } ),
    MakeDefaultComponent("circle_shape",        { world::FACTION_ATTRIBUTE, world::RADIUS_ATTRIBUTE, world::POSITION_ATTRIBUTE } ),
    MakeDefaultComponent("box_shape",           { world::FACTION_ATTRIBUTE, world::WIDTH_ATTRIBUTE, world::HEIGHT_ATTRIBUTE, world::POSITION_ATTRIBUTE } ),
    MakeDefaultComponent("segment_shape",       { world::FACTION_ATTRIBUTE, world::START_ATTRIBUTE, world::END_ATTRIBUTE, world::RADIUS_ATTRIBUTE} ),
    MakeDefaultComponent("health",              { world::HEALTH_ATTRIBUTE } ),
    MakeDefaultComponent("entity_behaviour",    { world::ENTITY_BEHAVIOUR_ATTRIBUTE } ),
};

static const uint32_t TRANSFORM_COMPONENT       = default_components[0].hash;
static const uint32_t SPRITE_COMPONENT          = default_components[1].hash;
static const uint32_t PHYSICS_COMPONENT         = default_components[2].hash;
static const uint32_t CIRCLE_SHAPE_COMPONENT    = default_components[3].hash;
static const uint32_t BOX_SHAPE_COMPONENT       = default_components[4].hash;
static const uint32_t SEGMENT_SHAPE_COMPONENT   = default_components[5].hash;
static const uint32_t HEALTH_COMPONENT          = default_components[6].hash;
static const uint32_t BEHAVIOUR_COMPONENT       = default_components[7].hash;

inline Component DefaultComponentFromHash(uint32_t hash)
{
    const auto find_template = [hash](const Component& component_template) {
        return component_template.hash == hash;
    };

    const auto it = std::find_if(default_components.begin(), default_components.end(), find_template);
    if(it != default_components.end())
        return *it;

    return Component();
}

inline void StripUnknownProperties(Component& component)
{
    const Component& template_component = DefaultComponentFromHash(component.hash);
    const std::vector<Attribute>& template_attributes = template_component.properties;

    const auto not_in_template = [&template_attributes](const Attribute& attribute) {
        const uint32_t attribute_hash = attribute.id;
        const auto not_in = [attribute_hash](const Attribute& template_attribute) {
            return template_attribute.id == attribute_hash;
        };

        return std::none_of(template_attributes.begin(), template_attributes.end(), not_in);
    };

    const auto it = std::remove_if(component.properties.begin(), component.properties.end(), not_in_template);
    component.properties.erase(it, component.properties.end());
}
