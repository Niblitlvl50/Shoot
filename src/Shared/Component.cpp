
#include "Component.h"
#include "Util/Hash.h"
#include "System/System.h"

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

static const std::vector<math::Vector> polygon_default = {
    math::Vector(0.0f, 0.0f),
    math::Vector(0.0f, 1.0f),
    math::Vector(1.0f, 1.0f)
};

const std::array<DefaultAttribute, 43> default_attributes = {{
    DefaultAttribute("position",            Variant(math::ZeroVec)),
    DefaultAttribute("rotation",            Variant(0.0f)),
    DefaultAttribute("radius",              Variant(1.0f)),
    DefaultAttribute("size",                Variant(math::Vector(1.0f, 1.0f))),
    DefaultAttribute("time_stamp",          Variant(1000)),
    DefaultAttribute("spawn_tag",           Variant(std::string())),
    DefaultAttribute("path_file",           Variant(std::string())),
    DefaultAttribute("trigger_radius",      Variant(1.0f)),
    DefaultAttribute("color",               Variant(mono::Color::WHITE)),

    DefaultAttribute("pickup_type",         Variant(0)),
    DefaultAttribute("amount",              Variant(10)),

    DefaultAttribute("body_type",           Variant(0)),
    DefaultAttribute("mass",                Variant(10.0f)),
    DefaultAttribute("inertia",             Variant(1.0f)),
    DefaultAttribute("prevent_rotation",    Variant(false)),

    DefaultAttribute("faction",             Variant(0)),
    DefaultAttribute("faction_picker",      Variant(0u)),
    DefaultAttribute("UNUSED",              Variant(1.0f)),
    DefaultAttribute("UNUSED",              Variant(1.0f)),
    DefaultAttribute("start",               Variant(math::ZeroVec)),
    DefaultAttribute("end",                 Variant(math::ZeroVec)),
    DefaultAttribute("sensor",              Variant(false)),

    DefaultAttribute("health",              Variant(100)),
    DefaultAttribute("score",               Variant(90)),
    DefaultAttribute("boss_health",         Variant(false)),

    DefaultAttribute("sprite_file",         Variant(std::string())),
    DefaultAttribute("animation",           Variant(0)),
    DefaultAttribute("flip_vertical",       Variant(false)),
    DefaultAttribute("flip_horizontal",     Variant(false)),
    DefaultAttribute("sprite_layer",        Variant(0)),

    DefaultAttribute("behaviour",           Variant(0)),
    DefaultAttribute("spawn_score",         Variant(10)),
    DefaultAttribute("trigger_name",        Variant(std::string())),
    DefaultAttribute("duration",            Variant(1.0f)),
    DefaultAttribute("easing_func",         Variant(0)),
    DefaultAttribute("logic_op",            Variant(0)),
    DefaultAttribute("num_entities",        Variant(0)),
    DefaultAttribute("font_id",             Variant(0)),
    DefaultAttribute("text",                Variant(std::string())),
    DefaultAttribute("text_shadow",         Variant(false)),
    DefaultAttribute("animation_mode",      Variant(0)),
    DefaultAttribute("repeating",           Variant(false)),
    DefaultAttribute("polygon",             Variant(polygon_default)),
}};

extern const uint32_t POSITION_ATTRIBUTE            = default_attributes[0].hash;
extern const uint32_t ROTATION_ATTRIBUTE            = default_attributes[1].hash;
extern const uint32_t RADIUS_ATTRIBUTE              = default_attributes[2].hash;
extern const uint32_t SIZE_ATTRIBUTE                = default_attributes[3].hash;
extern const uint32_t TIME_STAMP_ATTRIBUTE          = default_attributes[4].hash;
extern const uint32_t SPAWN_TAG_ATTRIBUTE           = default_attributes[5].hash;
extern const uint32_t PATH_FILE_ATTRIBUTE           = default_attributes[6].hash;
extern const uint32_t TRIGGER_RADIUS_ATTRIBUTE      = default_attributes[7].hash;
extern const uint32_t COLOR_ATTRIBUTE               = default_attributes[8].hash;

extern const uint32_t PICKUP_TYPE_ATTRIBUTE         = default_attributes[9].hash;
extern const uint32_t AMOUNT_ATTRIBUTE              = default_attributes[10].hash;

extern const uint32_t BODY_TYPE_ATTRIBUTE           = default_attributes[11].hash;
extern const uint32_t MASS_ATTRIBUTE                = default_attributes[12].hash;
extern const uint32_t INERTIA_ATTRIBUTE             = default_attributes[13].hash;
extern const uint32_t PREVENT_ROTATION_ATTRIBUTE    = default_attributes[14].hash;

extern const uint32_t FACTION_ATTRIBUTE             = default_attributes[15].hash;
extern const uint32_t FACTION_PICKER_ATTRIBUTE      = default_attributes[16].hash;

extern const uint32_t UNUSED_1                      = default_attributes[17].hash;
extern const uint32_t UNUSED_2                      = default_attributes[18].hash;
extern const uint32_t START_ATTRIBUTE               = default_attributes[19].hash;
extern const uint32_t END_ATTRIBUTE                 = default_attributes[20].hash;
extern const uint32_t SENSOR_ATTRIBUTE              = default_attributes[21].hash;

extern const uint32_t HEALTH_ATTRIBUTE              = default_attributes[22].hash;
extern const uint32_t SCORE_ATTRIBUTE               = default_attributes[23].hash;
extern const uint32_t BOSS_HEALTH_ATTRIBUTE         = default_attributes[24].hash;

extern const uint32_t SPRITE_ATTRIBUTE              = default_attributes[25].hash;
extern const uint32_t ANIMATION_ATTRIBUTE           = default_attributes[26].hash;
extern const uint32_t FLIP_VERTICAL_ATTRIBUTE       = default_attributes[27].hash;
extern const uint32_t FLIP_HORIZONTAL_ATTRIBUTE     = default_attributes[28].hash;
extern const uint32_t SPRITE_LAYER_ATTRIBUTE        = default_attributes[29].hash;

extern const uint32_t ENTITY_BEHAVIOUR_ATTRIBUTE    = default_attributes[30].hash;
extern const uint32_t SPAWN_SCORE_ATTRIBUTE         = default_attributes[31].hash;
extern const uint32_t TRIGGER_NAME_ATTRIBUTE        = default_attributes[32].hash;
extern const uint32_t DURATION_ATTRIBUTE            = default_attributes[33].hash;
extern const uint32_t EASING_FUNC_ATTRIBUTE         = default_attributes[34].hash;
extern const uint32_t LOGIC_OP_ATTRIBUTE            = default_attributes[35].hash;
extern const uint32_t N_ENTITIES_ATTRIBUTE          = default_attributes[36].hash;

extern const uint32_t FONT_ID_ATTRIBUTE             = default_attributes[37].hash;
extern const uint32_t TEXT_ATTRIBUTE                = default_attributes[38].hash;
extern const uint32_t TEXT_SHADOW_ATTRIBUTE         = default_attributes[39].hash;
extern const uint32_t ANIMATION_MODE_ATTRIBUTE      = default_attributes[40].hash;
extern const uint32_t REPEATING_ATTRIBUTE           = default_attributes[41].hash;
extern const uint32_t POLYGON_ATTRIBUTE             = default_attributes[42].hash;


extern const uint32_t NULL_COMPONENT            = mono::Hash("null");
extern const uint32_t TRANSFORM_COMPONENT       = mono::Hash("transform");
extern const uint32_t SPRITE_COMPONENT          = mono::Hash("sprite");
extern const uint32_t TEXT_COMPONENT            = mono::Hash("text");
extern const uint32_t PHYSICS_COMPONENT         = mono::Hash("physics");
extern const uint32_t CIRCLE_SHAPE_COMPONENT    = mono::Hash("circle_shape");
extern const uint32_t BOX_SHAPE_COMPONENT       = mono::Hash("box_shape");
extern const uint32_t SEGMENT_SHAPE_COMPONENT   = mono::Hash("segment_shape");
extern const uint32_t POLYGON_SHAPE_COMPONENT   = mono::Hash("polygon_shape");
extern const uint32_t HEALTH_COMPONENT          = mono::Hash("health");
extern const uint32_t BEHAVIOUR_COMPONENT       = mono::Hash("entity_behaviour");
extern const uint32_t SPAWN_POINT_COMPONENT     = mono::Hash("spawn_point");
extern const uint32_t SHAPE_TRIGGER_COMPONENT   = mono::Hash("shape_trigger");
extern const uint32_t DEATH_TRIGGER_COMPONENT   = mono::Hash("death_trigger");
extern const uint32_t AREA_TRIGGER_COMPONENT    = mono::Hash("area_entity_trigger");
extern const uint32_t TIME_TRIGGER_COMPONENT    = mono::Hash("time_trigger");
extern const uint32_t PICKUP_COMPONENT          = mono::Hash("pickup");
extern const uint32_t ANIMATION_COMPONENT       = mono::Hash("set_animation");
extern const uint32_t TRANSLATION_COMPONENT     = mono::Hash("set_translation");
extern const uint32_t ROTATION_COMPONENT        = mono::Hash("set_rotation");


const char* ComponentNameFromHash(uint32_t hash)
{
    if(hash == NULL_COMPONENT)
        return "null";
    else if(hash == TRANSFORM_COMPONENT)
        return "transform";
    else if(hash == SPRITE_COMPONENT)
        return "sprite";
    else if(hash == TEXT_COMPONENT)
        return "text";
    else if(hash == PHYSICS_COMPONENT)
        return "physics";
    else if(hash == CIRCLE_SHAPE_COMPONENT)
        return "circle_shape";
    else if(hash == BOX_SHAPE_COMPONENT)
        return "box_shape";
    else if(hash == SEGMENT_SHAPE_COMPONENT)
        return "segment_shape";
    else if(hash == POLYGON_SHAPE_COMPONENT)
        return "polygon_shape";
    else if(hash == HEALTH_COMPONENT)
        return "health";
    else if(hash == BEHAVIOUR_COMPONENT)
        return "entity_behaviour";
    else if(hash == SPAWN_POINT_COMPONENT)
        return "spawn_point";
    else if(hash == SHAPE_TRIGGER_COMPONENT)
        return "shape_trigger";
    else if(hash == DEATH_TRIGGER_COMPONENT)
        return "death_trigger";
    else if(hash == AREA_TRIGGER_COMPONENT)
        return "area_entity_trigger";
    else if(hash == TIME_TRIGGER_COMPONENT)
        return "time_trigger";
    else if(hash == PICKUP_COMPONENT)
        return "pickup";
    else if(hash == ANIMATION_COMPONENT)
        return "set_animation";
    else if(hash == TRANSLATION_COMPONENT)
        return "set_translation";
    else if(hash == ROTATION_COMPONENT)
        return "set_rotation";

    return "Unknown";
}

Component MakeComponent(uint32_t hash, uint32_t depends_on, bool allow_multiple, const std::vector<uint32_t>& properties)
{
    std::vector<Attribute> attributes;
    for(uint32_t property_hash : properties)
        attributes.push_back({ property_hash, DefaultAttributeFromHash(property_hash) });

    return { hash, depends_on, allow_multiple, std::move(attributes) };
}

const ComponentArray default_components = {
    MakeComponent(TRANSFORM_COMPONENT,      NULL_COMPONENT,     false,  { POSITION_ATTRIBUTE, ROTATION_ATTRIBUTE } ),
    MakeComponent(SPRITE_COMPONENT,         NULL_COMPONENT,     false,  { SPRITE_ATTRIBUTE, ANIMATION_ATTRIBUTE, SPRITE_LAYER_ATTRIBUTE, COLOR_ATTRIBUTE, FLIP_VERTICAL_ATTRIBUTE, FLIP_HORIZONTAL_ATTRIBUTE } ),
    MakeComponent(TEXT_COMPONENT,           NULL_COMPONENT,     false,  { TEXT_ATTRIBUTE, FONT_ID_ATTRIBUTE, COLOR_ATTRIBUTE, TEXT_SHADOW_ATTRIBUTE } ),
    MakeComponent(PHYSICS_COMPONENT,        NULL_COMPONENT,     false,  { BODY_TYPE_ATTRIBUTE, MASS_ATTRIBUTE, INERTIA_ATTRIBUTE, PREVENT_ROTATION_ATTRIBUTE } ),
    MakeComponent(CIRCLE_SHAPE_COMPONENT,   PHYSICS_COMPONENT,  true,   { FACTION_ATTRIBUTE, RADIUS_ATTRIBUTE, POSITION_ATTRIBUTE, SENSOR_ATTRIBUTE } ),
    MakeComponent(BOX_SHAPE_COMPONENT,      PHYSICS_COMPONENT,  true,   { FACTION_ATTRIBUTE, SIZE_ATTRIBUTE, POSITION_ATTRIBUTE, SENSOR_ATTRIBUTE } ),
    MakeComponent(SEGMENT_SHAPE_COMPONENT,  PHYSICS_COMPONENT,  true,   { FACTION_ATTRIBUTE, START_ATTRIBUTE, END_ATTRIBUTE, RADIUS_ATTRIBUTE, SENSOR_ATTRIBUTE } ),
    MakeComponent(POLYGON_SHAPE_COMPONENT,  PHYSICS_COMPONENT,  true,   { FACTION_ATTRIBUTE, POLYGON_ATTRIBUTE, SENSOR_ATTRIBUTE } ),
    MakeComponent(HEALTH_COMPONENT,         NULL_COMPONENT,     false,  { HEALTH_ATTRIBUTE, SCORE_ATTRIBUTE, BOSS_HEALTH_ATTRIBUTE } ),
    MakeComponent(BEHAVIOUR_COMPONENT,      NULL_COMPONENT,     false,  { ENTITY_BEHAVIOUR_ATTRIBUTE } ),
    MakeComponent(SPAWN_POINT_COMPONENT,    NULL_COMPONENT,     false,  { SPAWN_SCORE_ATTRIBUTE } ),
    MakeComponent(SHAPE_TRIGGER_COMPONENT,  PHYSICS_COMPONENT,  false,  { TRIGGER_NAME_ATTRIBUTE, FACTION_PICKER_ATTRIBUTE } ),
    MakeComponent(DEATH_TRIGGER_COMPONENT,  HEALTH_COMPONENT,   false,  { TRIGGER_NAME_ATTRIBUTE } ),
    MakeComponent(AREA_TRIGGER_COMPONENT,   NULL_COMPONENT,     false,  { TRIGGER_NAME_ATTRIBUTE, SIZE_ATTRIBUTE, FACTION_PICKER_ATTRIBUTE, LOGIC_OP_ATTRIBUTE, N_ENTITIES_ATTRIBUTE } ),
    MakeComponent(TIME_TRIGGER_COMPONENT,   NULL_COMPONENT,     false,  { TRIGGER_NAME_ATTRIBUTE, TIME_STAMP_ATTRIBUTE, REPEATING_ATTRIBUTE } ),
    MakeComponent(PICKUP_COMPONENT,         PHYSICS_COMPONENT,  false,  { PICKUP_TYPE_ATTRIBUTE, AMOUNT_ATTRIBUTE } ),
    MakeComponent(ANIMATION_COMPONENT,      SPRITE_COMPONENT,   true,   { TRIGGER_NAME_ATTRIBUTE, ANIMATION_ATTRIBUTE } ),
    MakeComponent(TRANSLATION_COMPONENT,    NULL_COMPONENT,     true,   { ANIMATION_MODE_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE, DURATION_ATTRIBUTE, POSITION_ATTRIBUTE, EASING_FUNC_ATTRIBUTE } ),
    MakeComponent(ROTATION_COMPONENT,       NULL_COMPONENT,     true,   { ANIMATION_MODE_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE, DURATION_ATTRIBUTE, ROTATION_ATTRIBUTE, EASING_FUNC_ATTRIBUTE } ),
};

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

bool FindAttribute(uint32_t id, const std::vector<Attribute>& attributes, const Attribute*& output)
{
    return FindAttribute(id, const_cast<std::vector<Attribute>&>(attributes), const_cast<Attribute*&>(output));
}

void MergeAttributes(std::vector<Attribute>& result_attributes, const std::vector<Attribute>& other_attributes)
{
    for(const Attribute& input : other_attributes)
    {
        Attribute* attribute = nullptr;
        if(FindAttribute(input.id, result_attributes, attribute))
            attribute->value = input.value;
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

Component DefaultComponentFromHash(uint32_t hash)
{
    const auto find_template = [hash](const Component& component_template) {
        return component_template.hash == hash;
    };

    const auto it = std::find_if(default_components.begin(), default_components.end(), find_template);
    if(it != default_components.end())
        return *it;

    const char* component_name = ComponentNameFromHash(hash);
    System::Log("Component|Unable to find default component for hash: %u (%s)\n", hash, component_name);

    return Component();
}

Component* FindComponentFromHash(uint32_t hash, std::vector<Component>& components)
{
    const auto find_by_hash = [hash](const Component& component)
    {
        return component.hash == hash;
    };

    auto it = std::find_if(components.begin(), components.end(), find_by_hash);
    if(it != components.end())
        return &*it;

    return nullptr;
}

const Component* FindComponentFromHash(uint32_t hash, const std::vector<Component>& components)
{
    return FindComponentFromHash(hash, (std::vector<Component>&)components);
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

namespace
{
    void add_component_recursivly(uint32_t component_hash, std::vector<Component>& components)
    {
        Component new_component = DefaultComponentFromHash(component_hash);

        if(new_component.depends_on != NULL_COMPONENT)
        {
            const Component* found_dependency = FindComponentFromHash(new_component.depends_on, components);
            if(!found_dependency)
                add_component_recursivly(new_component.depends_on, components);
        }

        components.push_back(std::move(new_component));
    };
}

uint32_t shared::AddComponent(uint32_t hash, std::vector<Component>& components)
{
    const uint32_t num_components = components.size();
    add_component_recursivly(hash, components);
    return components.size() - num_components;
}
