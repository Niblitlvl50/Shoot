
#include "Component.h"
#include "System/System.h"

extern const std::array<DefaultAttribute, 29> default_attributes = {{
    DefaultAttribute("position",            Variant(math::ZeroVec)),
    DefaultAttribute("rotation",            Variant(0.0f)),
    DefaultAttribute("radius",              Variant(1.0f)),
    DefaultAttribute("time_stamp",          Variant(5)),
    DefaultAttribute("spawn_tag",           Variant("")),
    DefaultAttribute("path_file",           Variant("")),
    DefaultAttribute("trigger_radius",      Variant(10.0f)),
    DefaultAttribute("color",               Variant(mono::Color::WHITE)),

    DefaultAttribute("pickup_type",         Variant(0)),
    DefaultAttribute("amount",              Variant(10)),

    DefaultAttribute("body_type",           Variant(0)),
    DefaultAttribute("mass",                Variant(10.0f)),
    DefaultAttribute("inertia",             Variant(1.0f)),
    DefaultAttribute("prevent_rotation",    Variant(false)),

    DefaultAttribute("faction",             Variant(0)),
    DefaultAttribute("width",               Variant(1.0f)),
    DefaultAttribute("height",              Variant(1.0f)),
    DefaultAttribute("start",               Variant(math::ZeroVec)),
    DefaultAttribute("end",                 Variant(math::ZeroVec)),
    DefaultAttribute("sensor",              Variant(false)),

    DefaultAttribute("health",              Variant(100)),
    DefaultAttribute("score",               Variant(90)),
    DefaultAttribute("boss_health",         Variant(false)),

    DefaultAttribute("sprite_file",         Variant("")),
    DefaultAttribute("animation",           Variant(0)),
    DefaultAttribute("flip_vertical",       Variant(false)),
    DefaultAttribute("flip_horizontal",     Variant(false)),

    DefaultAttribute("behaviour",           Variant(0)),
    DefaultAttribute("spawn_score",         Variant(10)),
}};

extern const uint32_t POSITION_ATTRIBUTE            = default_attributes[0].hash;
extern const uint32_t ROTATION_ATTRIBUTE            = default_attributes[1].hash;
extern const uint32_t RADIUS_ATTRIBUTE              = default_attributes[2].hash;
extern const uint32_t TIME_STAMP_ATTRIBUTE          = default_attributes[3].hash;
extern const uint32_t SPAWN_TAG_ATTRIBUTE           = default_attributes[4].hash;
extern const uint32_t PATH_FILE_ATTRIBUTE           = default_attributes[5].hash;
extern const uint32_t TRIGGER_RADIUS_ATTRIBUTE      = default_attributes[6].hash;
extern const uint32_t COLOR_ATTRIBUTE               = default_attributes[7].hash;

extern const uint32_t PICKUP_TYPE_ATTRIBUTE         = default_attributes[8].hash;
extern const uint32_t AMOUNT_ATTRIBUTE              = default_attributes[9].hash;

extern const uint32_t BODY_TYPE_ATTRIBUTE           = default_attributes[10].hash;
extern const uint32_t MASS_ATTRIBUTE                = default_attributes[11].hash;
extern const uint32_t INERTIA_ATTRIBUTE             = default_attributes[12].hash;
extern const uint32_t PREVENT_ROTATION_ATTRIBUTE    = default_attributes[13].hash;

extern const uint32_t FACTION_ATTRIBUTE             = default_attributes[14].hash;
extern const uint32_t WIDTH_ATTRIBUTE               = default_attributes[15].hash;
extern const uint32_t HEIGHT_ATTRIBUTE              = default_attributes[16].hash;
extern const uint32_t START_ATTRIBUTE               = default_attributes[17].hash;
extern const uint32_t END_ATTRIBUTE                 = default_attributes[18].hash;
extern const uint32_t SENSOR_ATTRIBUTE              = default_attributes[19].hash;

extern const uint32_t HEALTH_ATTRIBUTE              = default_attributes[20].hash;
extern const uint32_t SCORE_ATTRIBUTE               = default_attributes[21].hash;
extern const uint32_t BOSS_HEALTH_ATTRIBUTE         = default_attributes[22].hash;

extern const uint32_t SPRITE_ATTRIBUTE              = default_attributes[23].hash;
extern const uint32_t ANIMATION_ATTRIBUTE           = default_attributes[24].hash;
extern const uint32_t FLIP_VERTICAL_ATTRIBUTE       = default_attributes[25].hash;
extern const uint32_t FLIP_HORIZONTAL_ATTRIBUTE     = default_attributes[26].hash;
extern const uint32_t ENTITY_BEHAVIOUR_ATTRIBUTE    = default_attributes[27].hash;

extern const uint32_t SPAWN_SCORE_ATTRIBUTE         = default_attributes[28].hash;


extern const std::array<Component, 9> default_components = {
    MakeDefaultComponent("transform",           { POSITION_ATTRIBUTE, ROTATION_ATTRIBUTE } ),
    MakeDefaultComponent("sprite",              { SPRITE_ATTRIBUTE, ANIMATION_ATTRIBUTE, COLOR_ATTRIBUTE, FLIP_VERTICAL_ATTRIBUTE, FLIP_HORIZONTAL_ATTRIBUTE } ),
    MakeDefaultComponent("physics",             { BODY_TYPE_ATTRIBUTE, MASS_ATTRIBUTE, INERTIA_ATTRIBUTE, PREVENT_ROTATION_ATTRIBUTE } ),
    MakeDefaultComponent("circle_shape",        { FACTION_ATTRIBUTE, SENSOR_ATTRIBUTE, RADIUS_ATTRIBUTE, POSITION_ATTRIBUTE } ),
    MakeDefaultComponent("box_shape",           { FACTION_ATTRIBUTE, SENSOR_ATTRIBUTE, WIDTH_ATTRIBUTE, HEIGHT_ATTRIBUTE, POSITION_ATTRIBUTE } ),
    MakeDefaultComponent("segment_shape",       { FACTION_ATTRIBUTE, SENSOR_ATTRIBUTE, START_ATTRIBUTE, END_ATTRIBUTE, RADIUS_ATTRIBUTE} ),
    MakeDefaultComponent("health",              { HEALTH_ATTRIBUTE, SCORE_ATTRIBUTE, BOSS_HEALTH_ATTRIBUTE } ),
    MakeDefaultComponent("entity_behaviour",    { ENTITY_BEHAVIOUR_ATTRIBUTE } ),
    MakeDefaultComponent("spawn_point",         { SPAWN_SCORE_ATTRIBUTE } ),
};

extern const uint32_t TRANSFORM_COMPONENT       = default_components[0].hash;
extern const uint32_t SPRITE_COMPONENT          = default_components[1].hash;
extern const uint32_t PHYSICS_COMPONENT         = default_components[2].hash;
extern const uint32_t CIRCLE_SHAPE_COMPONENT    = default_components[3].hash;
extern const uint32_t BOX_SHAPE_COMPONENT       = default_components[4].hash;
extern const uint32_t SEGMENT_SHAPE_COMPONENT   = default_components[5].hash;
extern const uint32_t HEALTH_COMPONENT          = default_components[6].hash;
extern const uint32_t BEHAVIOUR_COMPONENT       = default_components[7].hash;
extern const uint32_t SPAWN_POINT_COMPONENT     = default_components[8].hash;


const char* AttributeNameFromHash(uint32_t hash)
{
    for(const DefaultAttribute& hash_string : default_attributes)
    {
        if(hash_string.hash == hash)
            return hash_string.string;
    }

    return nullptr;
}

const Variant& DefaultAttributeFromHash(uint32_t hash)
{
    for(const DefaultAttribute& hash_string : default_attributes)
    {
        if(hash_string.hash == hash)
            return hash_string.default_value;
    }

    const char* attribute_name = AttributeNameFromHash(hash);
    System::Log("Component|Unable to find default attribute for hash: %u (%s)\n", hash, attribute_name);
    static const Variant null_attribute;
    return null_attribute;
}

bool FindAttribute(uint32_t id, std::vector<Attribute>& attributes, Attribute*& output)
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


void MergeAttributes(std::vector<Attribute>& result_attributes, const std::vector<Attribute>& other_attributes)
{
    for(const Attribute& input : other_attributes)
    {
        Attribute* attribute = nullptr;
        if(FindAttribute(input.id, result_attributes, attribute))
            attribute->attribute = input.attribute;
    }
}

void UnionAttributes(std::vector<Attribute>& result_attributes, const std::vector<Attribute>& other_attributes)
{
    for(const Attribute& input : other_attributes)
    {
        Attribute* attribute = nullptr;
        const bool found = FindAttribute(input.id, result_attributes, attribute);
        if(!found)
            result_attributes.push_back(input);
    }
}

Component MakeDefaultComponent(const char* name, const std::vector<uint32_t>& properties)
{
    std::vector<Attribute> attributes;
    for(uint32_t property_hash : properties)
        attributes.push_back({ property_hash, DefaultAttributeFromHash(property_hash) });

    return { mono::Hash(name), name, std::move(attributes) };
}

Component DefaultComponentFromHash(uint32_t hash)
{
    const auto find_template = [hash](const Component& component_template) {
        return component_template.hash == hash;
    };

    const auto it = std::find_if(default_components.begin(), default_components.end(), find_template);
    if(it != default_components.end())
        return *it;

    return Component();
}

void StripUnknownProperties(Component& component)
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
