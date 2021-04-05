
#include "Component.h"
#include "Util/Hash.h"
#include "System/System.h"

#include <array>

struct DefaultAttribute
{
    DefaultAttribute(const char* string, const Variant& default_value, const char* tooltip = "")
        : hash(mono::Hash(string))
        , string(string)
        , default_value(default_value)
        , tooltip(tooltip)
    { }

    const uint32_t hash;
    const char* string;
    const Variant default_value;
    const char* tooltip;
};

static const std::vector<math::Vector> polygon_default = {
    { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }
};

const std::array<DefaultAttribute, 57> default_attributes = {{
    { "position",           Variant(math::ZeroVec) },
    { "rotation",           Variant(0.0f) },
    { "radius",             Variant(1.0f) },
    { "size",               Variant(math::Vector(1.0f, 1.0f)) },
    { "time_stamp",         Variant(1000) },
    { "spawn_tag",          Variant(std::string()) },
    { "path_file",          Variant(std::string()) },
    { "trigger_radius",     Variant(1.0f) },
    { "color",              Variant(mono::Color::WHITE) },
    { "shadow_offset",      Variant(math::ZeroVec) },
    { "shadow_size",        Variant(1.0f) },
    { "pickup_type",        Variant(0) },
    { "amount",             Variant(10) },
    { "body_type",          Variant(0) },
    { "mass",               Variant(10.0f) },
    { "inertia",            Variant(1.0f) },
    { "prevent_rotation",   Variant(false) },
    { "faction",            Variant(0) },
    { "faction_picker",     Variant(0u) },
    { "zoom_level",         Variant(1.0f), "multiples, 0.5 means half zoom, 2.0f means double zoom." },
    { "width",              Variant(1.0f) },
    { "start",              Variant(math::ZeroVec) },
    { "end",                Variant(math::ZeroVec) },
    { "sensor",             Variant(false) },
    { "health",             Variant(100) },
    { "score",              Variant(90) },
    { "boss_health",        Variant(false) },
    { "sprite_file",        Variant(std::string()) },
    { "animation",          Variant(0) },
    { "unused",             Variant(false) },
    { "unused",             Variant(false) },
    { "sprite_layer",       Variant(0) },
    { "sprite_properties",  Variant(0u) },
    { "behaviour",          Variant(0) },
    { "spawn_score",        Variant(10) },
    { "trigger_name",       Variant(std::string()) },
    { "trigger_name_exit",  Variant(std::string()) },
    { "duration",           Variant(1.0f) },
    { "easing_func",        Variant(0) },
    { "logic_op",           Variant(0) },
    { "num_entities",       Variant(0) },
    { "font_id",            Variant(0) },
    { "text",               Variant(std::string()) },
    { "text_shadow",        Variant(false) },
    { "animation_mode",     Variant(0) },
    { "repeating",          Variant(false) },
    { "polygon",            Variant(polygon_default) },
    { "random_start_frame", Variant(false) },
    { "trigger_name_completed", Variant(std::string()) },
    { "count",                  Variant(2) },
    { "reset_on_compeleted",    Variant(false) },
    { "center_flags",           Variant(0u) },
    { "interaction_type",       Variant(0) },
    { "path_type",              Variant(0) },
    { "path_points",            Variant(polygon_default) },
    { "path_closed",            Variant(false) },
    { "entity_reference",       Variant(0u) },
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
extern const uint32_t SHADOW_OFFSET_ATTRIBUTE       = default_attributes[9].hash;
extern const uint32_t SHADOW_SIZE_ATTRIBUTE         = default_attributes[10].hash;

extern const uint32_t PICKUP_TYPE_ATTRIBUTE         = default_attributes[11].hash;
extern const uint32_t AMOUNT_ATTRIBUTE              = default_attributes[12].hash;

extern const uint32_t BODY_TYPE_ATTRIBUTE           = default_attributes[13].hash;
extern const uint32_t MASS_ATTRIBUTE                = default_attributes[14].hash;
//extern const uint32_t INERTIA_ATTRIBUTE             = default_attributes[15].hash;
extern const uint32_t PREVENT_ROTATION_ATTRIBUTE    = default_attributes[16].hash;

extern const uint32_t FACTION_ATTRIBUTE             = default_attributes[17].hash;
extern const uint32_t FACTION_PICKER_ATTRIBUTE      = default_attributes[18].hash;

extern const uint32_t ZOOM_LEVEL_ATTRIBUTE          = default_attributes[19].hash;
extern const uint32_t WIDTH_ATTRIBUTE               = default_attributes[20].hash;
extern const uint32_t START_ATTRIBUTE               = default_attributes[21].hash;
extern const uint32_t END_ATTRIBUTE                 = default_attributes[22].hash;
extern const uint32_t SENSOR_ATTRIBUTE              = default_attributes[23].hash;

extern const uint32_t HEALTH_ATTRIBUTE              = default_attributes[24].hash;
extern const uint32_t SCORE_ATTRIBUTE               = default_attributes[25].hash;
extern const uint32_t BOSS_HEALTH_ATTRIBUTE         = default_attributes[26].hash;

extern const uint32_t SPRITE_ATTRIBUTE              = default_attributes[27].hash;
extern const uint32_t ANIMATION_ATTRIBUTE           = default_attributes[28].hash;
extern const uint32_t UNUSED_3                      = default_attributes[29].hash;
extern const uint32_t UNUSED_4                      = default_attributes[30].hash;
extern const uint32_t SPRITE_LAYER_ATTRIBUTE        = default_attributes[31].hash;
extern const uint32_t SPRITE_PROPERTIES_ATTRIBUTE   = default_attributes[32].hash;

extern const uint32_t ENTITY_BEHAVIOUR_ATTRIBUTE    = default_attributes[33].hash;
extern const uint32_t SPAWN_SCORE_ATTRIBUTE         = default_attributes[34].hash;
extern const uint32_t TRIGGER_NAME_ATTRIBUTE        = default_attributes[35].hash;
extern const uint32_t TRIGGER_NAME_EXIT_ATTRIBUTE   = default_attributes[36].hash;
extern const uint32_t DURATION_ATTRIBUTE            = default_attributes[37].hash;
extern const uint32_t EASING_FUNC_ATTRIBUTE         = default_attributes[38].hash;
extern const uint32_t LOGIC_OP_ATTRIBUTE            = default_attributes[39].hash;
extern const uint32_t N_ENTITIES_ATTRIBUTE          = default_attributes[40].hash;

extern const uint32_t FONT_ID_ATTRIBUTE             = default_attributes[41].hash;
extern const uint32_t TEXT_ATTRIBUTE                = default_attributes[42].hash;
extern const uint32_t TEXT_SHADOW_ATTRIBUTE         = default_attributes[43].hash;
extern const uint32_t ANIMATION_MODE_ATTRIBUTE      = default_attributes[44].hash;
extern const uint32_t REPEATING_ATTRIBUTE           = default_attributes[45].hash;
extern const uint32_t POLYGON_ATTRIBUTE             = default_attributes[46].hash;
extern const uint32_t RANDOM_START_FRAME_ATTRIBUTE  = default_attributes[47].hash;

extern const uint32_t TRIGGER_NAME_COMPLETED_ATTRIBUTE  = default_attributes[48].hash;
extern const uint32_t COUNT_ATTRIBUTE                   = default_attributes[49].hash;
extern const uint32_t RESET_ON_COMPLETED_ATTRIBUTE      = default_attributes[50].hash;
extern const uint32_t CENTER_FLAGS_ATTRIBUTE            = default_attributes[51].hash;
extern const uint32_t INTERACTION_TYPE_ATTRIBUTE        = default_attributes[52].hash;

extern const uint32_t PATH_TYPE_ATTRIBUTE       = default_attributes[53].hash;
extern const uint32_t PATH_POINTS_ATTRIBUTE     = default_attributes[54].hash;
extern const uint32_t PATH_CLOSED_ATTRIBUTE     = default_attributes[55].hash;

extern const uint32_t ENTITY_REFERENCE_ATTRIBUTE    = default_attributes[56].hash;


extern const uint32_t NULL_COMPONENT                = mono::Hash("null");
extern const uint32_t TRANSFORM_COMPONENT           = mono::Hash("transform");
extern const uint32_t SPRITE_COMPONENT              = mono::Hash("sprite");
extern const uint32_t TEXT_COMPONENT                = mono::Hash("text");
extern const uint32_t PHYSICS_COMPONENT             = mono::Hash("physics");
extern const uint32_t CIRCLE_SHAPE_COMPONENT        = mono::Hash("circle_shape");
extern const uint32_t BOX_SHAPE_COMPONENT           = mono::Hash("box_shape");
extern const uint32_t SEGMENT_SHAPE_COMPONENT       = mono::Hash("segment_shape");
extern const uint32_t POLYGON_SHAPE_COMPONENT       = mono::Hash("polygon_shape");
extern const uint32_t HEALTH_COMPONENT              = mono::Hash("health");
extern const uint32_t BEHAVIOUR_COMPONENT           = mono::Hash("entity_behaviour");
extern const uint32_t SPAWN_POINT_COMPONENT         = mono::Hash("spawn_point");
extern const uint32_t SHAPE_TRIGGER_COMPONENT       = mono::Hash("shape_trigger");
extern const uint32_t DEATH_TRIGGER_COMPONENT       = mono::Hash("death_trigger");
extern const uint32_t AREA_TRIGGER_COMPONENT        = mono::Hash("area_entity_trigger");
extern const uint32_t TIME_TRIGGER_COMPONENT        = mono::Hash("time_trigger");
extern const uint32_t COUNTER_TRIGGER_COMPONENT     = mono::Hash("counter_trigger");
extern const uint32_t PICKUP_COMPONENT              = mono::Hash("pickup");
extern const uint32_t ANIMATION_COMPONENT           = mono::Hash("set_animation");
extern const uint32_t TRANSLATION_COMPONENT         = mono::Hash("set_translation");
extern const uint32_t ROTATION_COMPONENT            = mono::Hash("set_rotation");
extern const uint32_t CAMERA_ZOOM_COMPONENT         = mono::Hash("camera_zoom");
extern const uint32_t CAMERA_POINT_COMPONENT        = mono::Hash("camera_point");
extern const uint32_t INTERACTION_COMPONENT         = mono::Hash("interaction");
extern const uint32_t INTERACTION_SWITCH_COMPONENT  = mono::Hash("interaction_switch");

extern const uint32_t PATH_COMPONENT                = mono::Hash("path");

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
    else if(hash == COUNTER_TRIGGER_COMPONENT)
        return "counter_trigger";
    else if(hash == PICKUP_COMPONENT)
        return "pickup";
    else if(hash == ANIMATION_COMPONENT)
        return "set_animation";
    else if(hash == TRANSLATION_COMPONENT)
        return "set_translation";
    else if(hash == ROTATION_COMPONENT)
        return "set_rotation";
    else if(hash == CAMERA_ZOOM_COMPONENT)
        return "camera_zoom";
    else if(hash == CAMERA_POINT_COMPONENT)
        return "camera_point";
    else if(hash == INTERACTION_COMPONENT)
        return "interaction";
    else if(hash == INTERACTION_SWITCH_COMPONENT)
        return "interaction_switch";
    else if(hash == PATH_COMPONENT)
        return "path";

    return "Unknown";
}

Component MakeComponent(
    uint32_t hash, uint32_t depends_on, bool allow_multiple, const char* category, const std::vector<uint32_t>& properties)
{
    std::vector<Attribute> attributes;
    for(uint32_t property_hash : properties)
        attributes.push_back({ property_hash, DefaultAttributeFromHash(property_hash) });

    return { hash, depends_on, allow_multiple, category, std::move(attributes) };
}

const ComponentArray default_components = {
    MakeComponent(TRANSFORM_COMPONENT,          NULL_COMPONENT,     false,  "general",      { POSITION_ATTRIBUTE, ROTATION_ATTRIBUTE }),
    MakeComponent(HEALTH_COMPONENT,             NULL_COMPONENT,     false,  "general",      { HEALTH_ATTRIBUTE, SCORE_ATTRIBUTE, BOSS_HEALTH_ATTRIBUTE }),
    MakeComponent(PICKUP_COMPONENT,             PHYSICS_COMPONENT,  false,  "general",      { PICKUP_TYPE_ATTRIBUTE, AMOUNT_ATTRIBUTE }),
    MakeComponent(SPAWN_POINT_COMPONENT,        NULL_COMPONENT,     false,  "general",      { SPAWN_SCORE_ATTRIBUTE }),
    MakeComponent(INTERACTION_COMPONENT,        NULL_COMPONENT,     false,  "general",      { TRIGGER_NAME_ATTRIBUTE, INTERACTION_TYPE_ATTRIBUTE }),
    MakeComponent(INTERACTION_SWITCH_COMPONENT, NULL_COMPONENT,     false,  "general",      { TRIGGER_NAME_ATTRIBUTE, TRIGGER_NAME_EXIT_ATTRIBUTE, INTERACTION_TYPE_ATTRIBUTE }),
    MakeComponent(PATH_COMPONENT,               NULL_COMPONENT,     false,  "general",      { PATH_TYPE_ATTRIBUTE, PATH_POINTS_ATTRIBUTE, WIDTH_ATTRIBUTE, PATH_CLOSED_ATTRIBUTE }),
    MakeComponent(SPRITE_COMPONENT,             NULL_COMPONENT,     false,  "rendering",    { SPRITE_ATTRIBUTE, ANIMATION_ATTRIBUTE, SPRITE_LAYER_ATTRIBUTE, COLOR_ATTRIBUTE, SPRITE_PROPERTIES_ATTRIBUTE, SHADOW_OFFSET_ATTRIBUTE, SHADOW_SIZE_ATTRIBUTE, RANDOM_START_FRAME_ATTRIBUTE }),
    MakeComponent(TEXT_COMPONENT,               NULL_COMPONENT,     false,  "rendering",    { TEXT_ATTRIBUTE, FONT_ID_ATTRIBUTE, COLOR_ATTRIBUTE, CENTER_FLAGS_ATTRIBUTE, TEXT_SHADOW_ATTRIBUTE }),
    MakeComponent(PHYSICS_COMPONENT,            NULL_COMPONENT,     false,  "physics",      { BODY_TYPE_ATTRIBUTE, MASS_ATTRIBUTE, PREVENT_ROTATION_ATTRIBUTE }),
    MakeComponent(BOX_SHAPE_COMPONENT,          PHYSICS_COMPONENT,  true,   "physics",      { FACTION_ATTRIBUTE, SIZE_ATTRIBUTE, POSITION_ATTRIBUTE, SENSOR_ATTRIBUTE }),
    MakeComponent(CIRCLE_SHAPE_COMPONENT,       PHYSICS_COMPONENT,  true,   "physics",      { FACTION_ATTRIBUTE, RADIUS_ATTRIBUTE, POSITION_ATTRIBUTE, SENSOR_ATTRIBUTE }),
    MakeComponent(POLYGON_SHAPE_COMPONENT,      PHYSICS_COMPONENT,  true,   "physics",      { FACTION_ATTRIBUTE, POLYGON_ATTRIBUTE, SENSOR_ATTRIBUTE }),
    MakeComponent(SEGMENT_SHAPE_COMPONENT,      PHYSICS_COMPONENT,  true,   "physics",      { FACTION_ATTRIBUTE, START_ATTRIBUTE, END_ATTRIBUTE, RADIUS_ATTRIBUTE, SENSOR_ATTRIBUTE }),
    MakeComponent(AREA_TRIGGER_COMPONENT,       NULL_COMPONENT,     false,  "triggers",     { TRIGGER_NAME_ATTRIBUTE, SIZE_ATTRIBUTE, FACTION_PICKER_ATTRIBUTE, LOGIC_OP_ATTRIBUTE, N_ENTITIES_ATTRIBUTE }),
    MakeComponent(COUNTER_TRIGGER_COMPONENT,    NULL_COMPONENT,     false,  "triggers",     { TRIGGER_NAME_ATTRIBUTE, TRIGGER_NAME_COMPLETED_ATTRIBUTE, COUNT_ATTRIBUTE, RESET_ON_COMPLETED_ATTRIBUTE }),
    MakeComponent(DEATH_TRIGGER_COMPONENT,      HEALTH_COMPONENT,   false,  "triggers",     { TRIGGER_NAME_ATTRIBUTE }),
    MakeComponent(SHAPE_TRIGGER_COMPONENT,      NULL_COMPONENT,     false,  "triggers",     { TRIGGER_NAME_ATTRIBUTE, TRIGGER_NAME_EXIT_ATTRIBUTE, FACTION_PICKER_ATTRIBUTE }),
    MakeComponent(TIME_TRIGGER_COMPONENT,       NULL_COMPONENT,     false,  "triggers",     { TRIGGER_NAME_ATTRIBUTE, TIME_STAMP_ATTRIBUTE, REPEATING_ATTRIBUTE }),
    MakeComponent(ANIMATION_COMPONENT,          SPRITE_COMPONENT,   true,   "animation",    { TRIGGER_NAME_ATTRIBUTE, ANIMATION_ATTRIBUTE }),
    MakeComponent(ROTATION_COMPONENT,           NULL_COMPONENT,     true,   "animation",    { ANIMATION_MODE_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE, DURATION_ATTRIBUTE, ROTATION_ATTRIBUTE, EASING_FUNC_ATTRIBUTE }),
    MakeComponent(TRANSLATION_COMPONENT,        NULL_COMPONENT,     true,   "animation",    { ANIMATION_MODE_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE, DURATION_ATTRIBUTE, POSITION_ATTRIBUTE, EASING_FUNC_ATTRIBUTE }),
    MakeComponent(CAMERA_POINT_COMPONENT,       NULL_COMPONENT,     false,  "camera",       { TRIGGER_NAME_ATTRIBUTE, POSITION_ATTRIBUTE }),
    MakeComponent(CAMERA_ZOOM_COMPONENT,        NULL_COMPONENT,     false,  "camera",       { TRIGGER_NAME_ATTRIBUTE, ZOOM_LEVEL_ATTRIBUTE }),
    MakeComponent(BEHAVIOUR_COMPONENT,          NULL_COMPONENT,     false,  "logic",        { ENTITY_BEHAVIOUR_ATTRIBUTE }),
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

const char* AttributeTooltipFromHash(uint32_t hash)
{
    for(const DefaultAttribute& hash_string : default_attributes)
    {
        if(hash_string.hash == hash)
            return hash_string.tooltip;
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

int shared::ComponentPriorityForHash(uint32_t hash)
{
    const auto find_func = [hash](const Component& component) {
        return component.hash == hash;
    };
    const auto it = std::find_if(default_components.begin(), default_components.end(), find_func);
    return std::distance(default_components.begin(), it);
}

void shared::SortComponentsByPriority(std::vector<Component>& components)
{
    const auto sort_by_prio = [](const Component& left, const Component& right) {
        return shared::ComponentPriorityForHash(left.hash) < shared::ComponentPriorityForHash(right.hash);
    };
    std::sort(components.begin(), components.end(), sort_by_prio);
}

