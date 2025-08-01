
#pragma once

#include "EntitySystem/ObjectAttribute.h"

#include <vector>
#include <algorithm>

extern const uint32_t POSITION_ATTRIBUTE;
extern const uint32_t ROTATION_ATTRIBUTE;
extern const uint32_t SCALE_ATTRIBUTE;
extern const uint32_t RADIUS_ATTRIBUTE;
extern const uint32_t SIZE_ATTRIBUTE;
extern const uint32_t OFFSET_ATTRIBUTE;
extern const uint32_t TIME_STAMP_ATTRIBUTE;
extern const uint32_t TAG_ATTRIBUTE;
extern const uint32_t ENABLE_ATTRIBUTE;
extern const uint32_t TRIGGER_RADIUS_ATTRIBUTE;
extern const uint32_t COLOR_ATTRIBUTE;
extern const uint32_t SHADOW_OFFSET_ATTRIBUTE;
extern const uint32_t SHADOW_SIZE_ATTRIBUTE;

extern const uint32_t PICKUP_TYPE_ATTRIBUTE;
extern const uint32_t AMOUNT_ATTRIBUTE;

extern const uint32_t BODY_TYPE_ATTRIBUTE;
extern const uint32_t MASS_ATTRIBUTE;
extern const uint32_t INERTIA_ATTRIBUTE;
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
extern const uint32_t RELEASE_ON_DEATH_ATTRIBUTE;

extern const uint32_t SPRITE_ATTRIBUTE;
extern const uint32_t ANIMATION_ATTRIBUTE;
extern const uint32_t DRAW_NAME_ATTRIBUTE;
extern const uint32_t SORT_OFFSET_ATTRIBUTE;
extern const uint32_t LAYER_ATTRIBUTE;
extern const uint32_t RANDOM_START_FRAME_ATTRIBUTE;
extern const uint32_t SPRITE_PROPERTIES_ATTRIBUTE;

extern const uint32_t ENTITY_BEHAVIOUR_ATTRIBUTE;

extern const uint32_t SPAWN_SCORE_ATTRIBUTE;
extern const uint32_t SPAWN_LIMIT_ATTRIBUTE;
extern const uint32_t SPAWN_LIMIT_CONCURRENT_ATTRIBUTE;
extern const uint32_t SPAWN_POINTS_ATTRIBUTE;
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

extern const uint32_t SHADOW_COLOR_ATTRIBUTE;
extern const uint32_t EMIT_ONCE_ATTRIBUTE;
extern const uint32_t ENTITY_FILE_ATTRIBUTE;

extern const uint32_t POOL_SIZE_ATTRIBUTE;
extern const uint32_t BLEND_MODE_ATTRIBUTE;
extern const uint32_t EMIT_RATE_ATTRIBUTE;
extern const uint32_t EMITTER_TYPE_ATTRIBUTE;
extern const uint32_t START_SIZE_SPREAD_ATTRIBUTE;
extern const uint32_t END_SIZE_SPREAD_ATTRIBUTE;
extern const uint32_t LIFE_INTERVAL_ATTRIBUTE;
extern const uint32_t DIRECTION_INTERVAL_ATTRIBUTE;
extern const uint32_t MAGNITUDE_INTERVAL_ATTRIBUTE;
extern const uint32_t ANGLAR_VELOCITY_INTERVAL_ATTRIBUTE;
extern const uint32_t GRADIENT4_ATTRIBUTE;
extern const uint32_t DAMPING_ATTRIBUTE;
extern const uint32_t USE_CUSTOM_DAMPING;

extern const uint32_t TRANSFORM_SPACE_ATTRIBUTE;
extern const uint32_t PARTICLE_DRAW_LAYER;

extern const uint32_t EDITOR_PROPERTIES_ATTRIBUTE;
extern const uint32_t WEAPON_PRIMARY_ATTRIBUTE;
extern const uint32_t WEAPON_SECONDARY_ATTRIBUTE;
extern const uint32_t WEAPON_TERTIARY_ATTRIBUTE;

extern const uint32_t SOUND_ATTRIBUTE;
extern const uint32_t SOUND_PLAY_PARAMETERS;

extern const uint32_t SUB_TEXT_ATTRIBUTE;

extern const uint32_t UI_GROUP_ATTRIBUTE;
extern const uint32_t UI_ITEM_STATE_ATTRIBUTE;
extern const uint32_t UI_LEFT_ITEM_ID_ATTRIBUTE;
extern const uint32_t UI_RIGHT_ITEM_ID_ATTRIBUTE;
extern const uint32_t UI_ABOVE_ITEM_ID_ATTRIBUTE;
extern const uint32_t UI_BELOW_ITEM_ID_ATTRIBUTE;

extern const uint32_t POLYGON_DRAW_LAYER_ATTRIBUTE;
extern const uint32_t UNIFORM_DIRECTION_ATTRIBUTE;
extern const uint32_t EMITTER_MODE_ATTRIBUTE;

extern const uint32_t ENTITY_TYPE_ATTRIBUTE;
extern const uint32_t PRIORITY_ATTRIBUTE;
extern const uint32_t TARGET_FACTION_ATTRIBUTE;
extern const uint32_t COMPLETED_TRIGGER_ATTRIBUTE;
extern const uint32_t FAILED_TRIGGER_ATTRIBUTE;
extern const uint32_t TIME_BASED_ATTRIBUTE;
extern const uint32_t TIME_ATTRIBUTE;
extern const uint32_t FAIL_ON_TIMEOUT_ATTRIBUTE;

extern const uint32_t PHYSICS_MATERIAL_ATTRIBUTE;
extern const uint32_t WEAPON_MODIFIER_TYPE_ATTRIBUTE;

extern const uint32_t STRENGTH_ATTRIBUTE;

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
    if(found_attribute && std::holds_alternative<T>(it->value))
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
extern const uint32_t LAYER_COMPONENT;
extern const uint32_t TAG_COMPONENT;
extern const uint32_t SPRITE_COMPONENT;
extern const uint32_t TEXT_COMPONENT;
extern const uint32_t PHYSICS_COMPONENT;
extern const uint32_t CIRCLE_SHAPE_COMPONENT;
extern const uint32_t BOX_SHAPE_COMPONENT;
extern const uint32_t SEGMENT_SHAPE_COMPONENT;
extern const uint32_t POLYGON_SHAPE_COMPONENT;
extern const uint32_t HEALTH_COMPONENT;
extern const uint32_t SHOCKWAVE_COMPONENT;
extern const uint32_t BEHAVIOUR_COMPONENT;
extern const uint32_t SPAWN_POINT_COMPONENT;
extern const uint32_t ENTITY_SPAWN_POINT_COMPONENT;
extern const uint32_t DESPAWN_ENTITY_COMPONENT;
extern const uint32_t SHAPE_TRIGGER_COMPONENT;
extern const uint32_t DESTROYED_TRIGGER_COMPONENT;
extern const uint32_t AREA_TRIGGER_COMPONENT;
extern const uint32_t TIME_TRIGGER_COMPONENT;
extern const uint32_t COUNTER_TRIGGER_COMPONENT;
extern const uint32_t RELAY_TRIGGER_COMPONENT;
extern const uint32_t PICKUP_COMPONENT;
extern const uint32_t WEAPON_PICKUP_COMPONENT;
extern const uint32_t LOOTBOX_COMPONENT;
extern const uint32_t ANIMATION_COMPONENT;
extern const uint32_t TRANSLATION_COMPONENT;
extern const uint32_t ROTATION_COMPONENT;
extern const uint32_t SCALE_COMPONENT;
extern const uint32_t CAMERA_ZOOM_COMPONENT;
extern const uint32_t CAMERA_POINT_COMPONENT;
extern const uint32_t CAMERA_TRACK_ENTITY_COMPONENT;
extern const uint32_t CAMERA_RESTORE_COMPONENT;
extern const uint32_t INTERACTION_COMPONENT;
extern const uint32_t INTERACTION_SWITCH_COMPONENT;
extern const uint32_t PATH_COMPONENT;
extern const uint32_t ROAD_COMPONENT;
extern const uint32_t LIGHT_COMPONENT;
extern const uint32_t DIALOG_COMPONENT;
extern const uint32_t PARTICLE_SYSTEM_COMPONENT;
extern const uint32_t AREA_EMITTER_COMPONENT;
extern const uint32_t TEXTURED_POLYGON_COMPONENT;
extern const uint32_t WEAPON_LOADOUT_COMPONENT;
extern const uint32_t SOUND_COMPONENT;
extern const uint32_t UI_ITEM_COMPONENT;
extern const uint32_t UI_SET_GROUP_STATE_COMPONENT;
extern const uint32_t REGION_COMPONENT;
extern const uint32_t TELEPORT_PLAYER_COMPONENT;
extern const uint32_t ENTITY_TRACKING_COMPONENT;
extern const uint32_t TARGET_COMPONENT;
extern const uint32_t MISSION_TRACKER_COMPONENT;
extern const uint32_t MISSION_ACTIVATION_COMPONENT;
extern const uint32_t MISSION_LOCATION_COMPONENT;
extern const uint32_t PHYSICS_IMPULSE_COMPONENT;

struct Component
{
    uint32_t hash;
    uint32_t depends_on;
    bool allow_multiple;
    const char* category;
    std::vector<Attribute> properties;
};

namespace component
{
    const char* ComponentNameFromHash(uint32_t hash);
    Component DefaultComponentFromHash(uint32_t hash);
    Component* FindComponentFromHash(uint32_t hash, std::vector<Component>& components);
    const Component* FindComponentFromHash(uint32_t hash, const std::vector<Component>& components);
    void StripUnknownProperties(Component& component);
    std::vector<const Component*> GetAllDefaultComponents();
    std::vector<Component*> AddComponent(uint32_t hash, std::vector<Component>& components);
    int ComponentPriorityForHash(uint32_t hash);
    void SortComponentsByPriority(std::vector<Component>& components);
}
