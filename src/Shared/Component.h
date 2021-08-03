
#pragma once

#include "EntitySystem/ObjectAttribute.h"

#include <vector>
#include <algorithm>

extern const uint32_t POSITION_ATTRIBUTE;
extern const uint32_t ROTATION_ATTRIBUTE;
extern const uint32_t RADIUS_ATTRIBUTE;
extern const uint32_t SIZE_ATTRIBUTE;
extern const uint32_t TIME_STAMP_ATTRIBUTE;
extern const uint32_t SPAWN_TAG_ATTRIBUTE;
extern const uint32_t PATH_FILE_ATTRIBUTE;
extern const uint32_t TRIGGER_RADIUS_ATTRIBUTE;
extern const uint32_t COLOR_ATTRIBUTE;
extern const uint32_t SHADOW_OFFSET_ATTRIBUTE;
extern const uint32_t SHADOW_SIZE_ATTRIBUTE;

extern const uint32_t PICKUP_TYPE_ATTRIBUTE;
extern const uint32_t AMOUNT_ATTRIBUTE;

extern const uint32_t BODY_TYPE_ATTRIBUTE;
extern const uint32_t MASS_ATTRIBUTE;
// extern const uint32_t INERTIA_ATTRIBUTE;
extern const uint32_t PREVENT_ROTATION_ATTRIBUTE;

extern const uint32_t FACTION_ATTRIBUTE;
extern const uint32_t FACTION_PICKER_ATTRIBUTE;
extern const uint32_t ZOOM_LEVEL_ATTRIBUTE;
extern const uint32_t WIDTH_ATTRIBUTE;
extern const uint32_t START_ATTRIBUTE;
extern const uint32_t END_ATTRIBUTE;
extern const uint32_t SENSOR_ATTRIBUTE;

extern const uint32_t HEALTH_ATTRIBUTE;
extern const uint32_t SCORE_ATTRIBUTE;
extern const uint32_t BOSS_HEALTH_ATTRIBUTE;

extern const uint32_t SPRITE_ATTRIBUTE;
extern const uint32_t ANIMATION_ATTRIBUTE;
//extern const uint32_t FLIP_VERTICAL_ATTRIBUTE;
extern const uint32_t SORT_OFFSET_ATTRIBUTE;
extern const uint32_t LAYER_ATTRIBUTE;
extern const uint32_t RANDOM_START_FRAME_ATTRIBUTE;
extern const uint32_t SPRITE_PROPERTIES_ATTRIBUTE;

extern const uint32_t ENTITY_BEHAVIOUR_ATTRIBUTE;

extern const uint32_t SPAWN_SCORE_ATTRIBUTE;
extern const uint32_t TRIGGER_NAME_ATTRIBUTE;
extern const uint32_t TRIGGER_NAME_EXIT_ATTRIBUTE;

extern const uint32_t ENABLE_TRIGGER_ATTRIBUTE;
extern const uint32_t DISABLE_TRIGGER_ATTRIBUTE;

extern const uint32_t DURATION_ATTRIBUTE;
extern const uint32_t EASING_FUNC_ATTRIBUTE;
extern const uint32_t LOGIC_OP_ATTRIBUTE;
extern const uint32_t N_ENTITIES_ATTRIBUTE;

extern const uint32_t DESTROYED_TRIGGER_TYPE_ATTRIBUTE;

extern const uint32_t FONT_ID_ATTRIBUTE;
extern const uint32_t TEXT_ATTRIBUTE;
extern const uint32_t TEXT_SHADOW_ATTRIBUTE;

extern const uint32_t ANIMATION_MODE_ATTRIBUTE;
extern const uint32_t REPEATING_ATTRIBUTE;
extern const uint32_t POLYGON_ATTRIBUTE;

extern const uint32_t TRIGGER_NAME_COMPLETED_ATTRIBUTE;
extern const uint32_t COUNT_ATTRIBUTE;
extern const uint32_t RESET_ON_COMPLETED_ATTRIBUTE;
extern const uint32_t CENTER_FLAGS_ATTRIBUTE;

extern const uint32_t INTERACTION_TYPE_ATTRIBUTE;

extern const uint32_t PATH_TYPE_ATTRIBUTE;
extern const uint32_t PATH_POINTS_ATTRIBUTE;
extern const uint32_t PATH_CLOSED_ATTRIBUTE;

extern const uint32_t ENTITY_REFERENCE_ATTRIBUTE;
extern const uint32_t TEXTURE_ATTRIBUTE;

extern const uint32_t NAME_ATTRIBUTE;
extern const uint32_t FOLDER_ATTRIBUTE;

extern const uint32_t FLICKER_ATTRIBUTE;
extern const uint32_t FREQUENCY_ATTRIBUTE;
extern const uint32_t PERCENTAGE_ATTRIBUTE;

extern const uint32_t CONDITION_ATTRIBUTE;
extern const uint32_t CONDITION_STATE_ATTRIBUTE;


const char* AttributeNameFromHash(uint32_t hash);
const char* AttributeTooltipFromHash(uint32_t hash);
const Variant& DefaultAttributeFromHash(uint32_t hash);
bool FindAttribute(uint32_t id, std::vector<Attribute>& attributes, Attribute*& output);
bool FindAttribute(uint32_t id, const std::vector<Attribute>& attributes, const Attribute*& output);
void MergeAttributes(std::vector<Attribute>& result_attributes, const std::vector<Attribute>& other_attributes);
void UnionAttributes(std::vector<Attribute>& result_attributes, const std::vector<Attribute>& other_attributes);

enum class FallbackMode
{
    SET_DEFAULT,
    REQUIRE_ATTRIBUTE,
};

template <typename T>
inline bool FindAttribute(uint32_t id, const std::vector<Attribute>& attributes, T& value, FallbackMode fallback_mode)
{
    const auto find_func = [id](const Attribute& attribute) {
        return id == attribute.id;
    };

    const auto it = std::find_if(attributes.begin(), attributes.end(), find_func);
    const bool found_attribute = (it != attributes.end());
    if(found_attribute)
    {
        value = std::get<T>(it->value);
    }
    else if(fallback_mode == FallbackMode::SET_DEFAULT)
    {
        value = std::get<T>(DefaultAttributeFromHash(id));
        return true;
    }

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
        it->value = value;
}

extern const uint32_t NULL_COMPONENT;
extern const uint32_t NAME_FOLDER_COMPONENT;
extern const uint32_t TRANSFORM_COMPONENT;
extern const uint32_t SPRITE_COMPONENT;
extern const uint32_t TEXT_COMPONENT;
extern const uint32_t PHYSICS_COMPONENT;
extern const uint32_t CIRCLE_SHAPE_COMPONENT;
extern const uint32_t BOX_SHAPE_COMPONENT;
extern const uint32_t SEGMENT_SHAPE_COMPONENT;
extern const uint32_t POLYGON_SHAPE_COMPONENT;
extern const uint32_t HEALTH_COMPONENT;
extern const uint32_t BEHAVIOUR_COMPONENT;
extern const uint32_t SPAWN_POINT_COMPONENT;
extern const uint32_t SHAPE_TRIGGER_COMPONENT;
extern const uint32_t DESTROYED_TRIGGER_COMPONENT;
extern const uint32_t AREA_TRIGGER_COMPONENT;
extern const uint32_t TIME_TRIGGER_COMPONENT;
extern const uint32_t COUNTER_TRIGGER_COMPONENT;
extern const uint32_t PICKUP_COMPONENT;
extern const uint32_t ANIMATION_COMPONENT;
extern const uint32_t TRANSLATION_COMPONENT;
extern const uint32_t ROTATION_COMPONENT;
extern const uint32_t CAMERA_ZOOM_COMPONENT;
extern const uint32_t CAMERA_POINT_COMPONENT;
extern const uint32_t INTERACTION_COMPONENT;
extern const uint32_t INTERACTION_SWITCH_COMPONENT;
extern const uint32_t PATH_COMPONENT;
extern const uint32_t ROAD_COMPONENT;
extern const uint32_t LIGHT_COMPONENT;
extern const uint32_t DIALOG_COMPONENT;
extern const uint32_t SET_CONDITION_COMPONENT;

struct Component
{
    uint32_t hash;
    uint32_t depends_on;
    bool allow_multiple;
    const char* category;
    std::vector<Attribute> properties;
};

const char* ComponentNameFromHash(uint32_t hash);
Component DefaultComponentFromHash(uint32_t hash);
Component* FindComponentFromHash(uint32_t hash, std::vector<Component>& components);
const Component* FindComponentFromHash(uint32_t hash, const std::vector<Component>& components);
void StripUnknownProperties(Component& component);

namespace shared
{
    std::vector<const Component*> GetAllDefaultComponents();
    std::vector<Component*> AddComponent(uint32_t hash, std::vector<Component>& components);
    int ComponentPriorityForHash(uint32_t hash);
    void SortComponentsByPriority(std::vector<Component>& components);
}
