
#include "Component.h"
#include "System/Hash.h"
#include "System/System.h"
#include "System/Debug.h"

#include "EntitySystem/Entity.h"


struct DefaultAttribute
{
    DefaultAttribute(const char* string, const Variant& default_value, const char* tooltip = "")
        : hash(hash::Hash(string))
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

static const std::vector<math::Vector> spawn_points_default = {
    { 0.0f, 0.0f }
};

static const mono::Event event_type_input_default = {
    mono::EventType::Global, mono::EventDirection::Input, std::string()
};

static const mono::Event event_type_output_default = {
    mono::EventType::Global, mono::EventDirection::Output, std::string()
};

const DefaultAttribute default_attributes[] = {
    { "position",                   Variant(math::ZeroVec) },
    { "rotation",                   Variant(0.0f) },
    { "scale",                      Variant(1.0f) },
    { "radius",                     Variant(1.0f) },
    { "size",                       Variant(math::Vector(1.0f, 1.0f)) },
    { "time_stamp",                 Variant(1000) },
    { "tag",                        Variant(std::string()) },
    { "enable",                     Variant(true) },
    { "trigger_radius",             Variant(1.0f) },
    { "color",                      Variant(mono::Color::WHITE) },
    { "shadow_offset",              Variant(math::ZeroVec) },
    { "shadow_size",                Variant(1.0f) },
    { "pickup_type",                Variant(0) },
    { "amount",                     Variant(10) },
    { "body_type",                  Variant(0) },
    { "mass",                       Variant(10.0f) },
    { "inertia",                    Variant(1.0f) },
    { "prevent_rotation",           Variant(false) },
    { "faction",                    Variant(0) },
    { "faction_picker",             Variant(0u) },
    { "zoom_level",                 Variant(1.0f), "multiples, 0.5 means half zoom, 2.0f means double zoom." },
    { "width",                      Variant(1.0f) },
    { "start",                      Variant(math::ZeroVec) },
    { "end",                        Variant(math::ZeroVec) },
    { "sensor",                     Variant(false) },
    { "health",                     Variant(100) },
    { "score",                      Variant(90) },
    { "boss_health",                Variant(false) },
    { "sprite_file",                Variant(std::string()), "File..." },
    { "animation",                  Variant(0) },
    { "draw_name",                  Variant(false) },
    { "sort_offset",                Variant(0.0f) },
    { "layer",                      Variant(0) },
    { "sprite_properties",          Variant(0u) },
    { "behaviour",                  Variant(0) },
    { "spawn_score",                Variant(10) },
    { "spawn_limit",                Variant(0), "Limits active spawns, zero means no limit." },
    { "trigger_name",               Variant(event_type_input_default), "Trigger Event" },
    { "trigger_name_exit",          Variant(event_type_input_default), "Exit Event" },
    { "enable_trigger",             Variant(event_type_input_default), "Enable Event" },
    { "disable_trigger",            Variant(event_type_input_default), "Disable Event" },
    { "duration",                   Variant(1.0f) },
    { "easing_func",                Variant(0) },
    { "logic_op",                   Variant(0) },
    { "num_entities",               Variant(0) },
    { "font_id",                    Variant(0) },
    { "text",                       Variant(std::string()) },
    { "text_shadow",                Variant(false) },
    { "animation_mode",             Variant(0) },
    { "repeating",                  Variant(false) },
    { "polygon",                    Variant(polygon_default) },
    { "random_start_frame",         Variant(false) },
    { "trigger_name_completed",     Variant(event_type_output_default), "Completed Event" },
    { "count",                      Variant(2) },
    { "reset_on_compeleted",        Variant(false) },
    { "center_flags",               Variant(0u) },
    { "interaction_type",           Variant(0) },
    { "path_type",                  Variant(0) },
    { "path_points",                Variant(polygon_default) },
    { "path_closed",                Variant(false) },
    { "entity_reference",           Variant(mono::INVALID_ID) },
    { "texture",                    Variant(std::string()), "File..." },
    { "name",                       Variant(std::string()) },
    { "folder",                     Variant(std::string()) },
    { "flicker",                    Variant(false) },
    { "frequency",                  Variant(1.0f) },
    { "percentage",                 Variant(0.5f) },
    { "destroyed_type",             Variant(0) },
    { "shadow_color",               Variant(mono::Color::BLACK) },
    { "emit_once",                  Variant(false) },
    { "offset",                     Variant(math::ZeroVec) },
    { "entity_file",                Variant(std::string()), "File..." },
    { "pool_size",                  Variant(10) },
    { "blend_mode",                 Variant(0) },
    { "emit_rate",                  Variant(1.0f) },
    { "emitter_type",               Variant(0) },
    { "start_size_spread",          Variant(math::ValueSpread(32.0f, 0.0f, 0.0f)) },
    { "end_size_spread",            Variant(math::ValueSpread(32.0f, 0.0f, 0.0f)) },
    { "life_interval",              Variant(math::Interval(1.0f, 2.0f)) },
    { "direction_interval",         Variant(math::Interval(0.0f, 1.0f)) },
    { "magnitude_interval",         Variant(math::Interval(0.0f, 1.0f)) },
    { "angular_velocity_interval",  Variant(math::Interval()) },
    { "gradient_4",                 Variant(mono::Color::Gradient<4>()) },
    { "damping",                    Variant(0.0f) },
    { "transform_space",            Variant(0) },
    { "particle_draw_layer",        Variant(0) },
    { "editor_properties",          Variant(0u) },
    { "spawn_points",               Variant(spawn_points_default) },
    { "weapon_primary",             Variant(std::string()) },
    { "weapon_secondary",           Variant(std::string()) },
    { "weapon_tertiary",            Variant(std::string()) },
    { "sound_file",                 Variant(std::string()), "File..." },
    { "sound_properties",           Variant(0u) },
    { "sub_text",                   Variant(std::string()) },
    { "polygon_draw_layer",         Variant(0) },
    { "ui_group",                   Variant(0) },
    { "ui_item_state",              Variant(0) },
    { "ui_left_item_id",            Variant(mono::INVALID_ID) },
    { "ui_right_item_id",           Variant(mono::INVALID_ID) },
    { "ui_above_item_id",           Variant(mono::INVALID_ID) },
    { "ui_below_item_id",           Variant(mono::INVALID_ID) },
    { "uniform_direction",          Variant(false) },
    { "emitter_mode",               Variant(0) },
    { "entity_type",                Variant(0) },
    { "priority",                   Variant(0) },
    { "target_faction",             Variant(0) },
    { "spawn_limit_concurrent",     Variant(0) },
};

extern const uint32_t POSITION_ATTRIBUTE            = default_attributes[0].hash;
extern const uint32_t ROTATION_ATTRIBUTE            = default_attributes[1].hash;
extern const uint32_t SCALE_ATTRIBUTE               = default_attributes[2].hash;
extern const uint32_t RADIUS_ATTRIBUTE              = default_attributes[3].hash;
extern const uint32_t SIZE_ATTRIBUTE                = default_attributes[4].hash;
extern const uint32_t TIME_STAMP_ATTRIBUTE          = default_attributes[5].hash;
extern const uint32_t TAG_ATTRIBUTE                 = default_attributes[6].hash;
extern const uint32_t ENABLE_ATTRIBUTE              = default_attributes[7].hash;
extern const uint32_t TRIGGER_RADIUS_ATTRIBUTE      = default_attributes[8].hash;
extern const uint32_t COLOR_ATTRIBUTE               = default_attributes[9].hash;
extern const uint32_t SHADOW_OFFSET_ATTRIBUTE       = default_attributes[10].hash;
extern const uint32_t SHADOW_SIZE_ATTRIBUTE         = default_attributes[11].hash;

extern const uint32_t PICKUP_TYPE_ATTRIBUTE         = default_attributes[12].hash;
extern const uint32_t AMOUNT_ATTRIBUTE              = default_attributes[13].hash;

extern const uint32_t BODY_TYPE_ATTRIBUTE           = default_attributes[14].hash;
extern const uint32_t MASS_ATTRIBUTE                = default_attributes[15].hash;
extern const uint32_t INERTIA_ATTRIBUTE             = default_attributes[16].hash;
extern const uint32_t PREVENT_ROTATION_ATTRIBUTE    = default_attributes[17].hash;

extern const uint32_t FACTION_ATTRIBUTE             = default_attributes[18].hash;
extern const uint32_t FACTION_PICKER_ATTRIBUTE      = default_attributes[19].hash;

extern const uint32_t ZOOM_LEVEL_ATTRIBUTE          = default_attributes[20].hash;
extern const uint32_t WIDTH_ATTRIBUTE               = default_attributes[21].hash;
extern const uint32_t START_ATTRIBUTE               = default_attributes[22].hash;
extern const uint32_t END_ATTRIBUTE                 = default_attributes[23].hash;
extern const uint32_t SENSOR_ATTRIBUTE              = default_attributes[24].hash;

extern const uint32_t HEALTH_ATTRIBUTE              = default_attributes[25].hash;
extern const uint32_t SCORE_ATTRIBUTE               = default_attributes[26].hash;
extern const uint32_t BOSS_HEALTH_ATTRIBUTE         = default_attributes[27].hash;

extern const uint32_t SPRITE_ATTRIBUTE              = default_attributes[28].hash;
extern const uint32_t ANIMATION_ATTRIBUTE           = default_attributes[29].hash;
extern const uint32_t DRAW_NAME_ATTRIBUTE           = default_attributes[30].hash;
extern const uint32_t SORT_OFFSET_ATTRIBUTE         = default_attributes[31].hash;
extern const uint32_t LAYER_ATTRIBUTE               = default_attributes[32].hash;
extern const uint32_t SPRITE_PROPERTIES_ATTRIBUTE   = default_attributes[33].hash;

extern const uint32_t ENTITY_BEHAVIOUR_ATTRIBUTE    = default_attributes[34].hash;
extern const uint32_t SPAWN_SCORE_ATTRIBUTE         = default_attributes[35].hash;
extern const uint32_t SPAWN_LIMIT_ATTRIBUTE         = default_attributes[36].hash;

extern const uint32_t TRIGGER_NAME_ATTRIBUTE        = default_attributes[37].hash;
extern const uint32_t TRIGGER_NAME_EXIT_ATTRIBUTE   = default_attributes[38].hash;
extern const uint32_t ENABLE_TRIGGER_ATTRIBUTE      = default_attributes[39].hash;
extern const uint32_t DISABLE_TRIGGER_ATTRIBUTE     = default_attributes[40].hash;

extern const uint32_t DURATION_ATTRIBUTE            = default_attributes[41].hash;
extern const uint32_t EASING_FUNC_ATTRIBUTE         = default_attributes[42].hash;
extern const uint32_t LOGIC_OP_ATTRIBUTE            = default_attributes[43].hash;
extern const uint32_t N_ENTITIES_ATTRIBUTE          = default_attributes[44].hash;

extern const uint32_t FONT_ID_ATTRIBUTE             = default_attributes[45].hash;
extern const uint32_t TEXT_ATTRIBUTE                = default_attributes[46].hash;
extern const uint32_t TEXT_SHADOW_ATTRIBUTE         = default_attributes[47].hash;
extern const uint32_t ANIMATION_MODE_ATTRIBUTE      = default_attributes[48].hash;
extern const uint32_t REPEATING_ATTRIBUTE           = default_attributes[49].hash;
extern const uint32_t POLYGON_ATTRIBUTE             = default_attributes[50].hash;
extern const uint32_t RANDOM_START_FRAME_ATTRIBUTE  = default_attributes[51].hash;

extern const uint32_t TRIGGER_NAME_COMPLETED_ATTRIBUTE  = default_attributes[52].hash;
extern const uint32_t COUNT_ATTRIBUTE                   = default_attributes[53].hash;
extern const uint32_t RESET_ON_COMPLETED_ATTRIBUTE      = default_attributes[54].hash;
extern const uint32_t CENTER_FLAGS_ATTRIBUTE            = default_attributes[55].hash;
extern const uint32_t INTERACTION_TYPE_ATTRIBUTE        = default_attributes[56].hash;

extern const uint32_t PATH_TYPE_ATTRIBUTE           = default_attributes[57].hash;
extern const uint32_t PATH_POINTS_ATTRIBUTE         = default_attributes[58].hash;
extern const uint32_t PATH_CLOSED_ATTRIBUTE         = default_attributes[59].hash;

extern const uint32_t ENTITY_REFERENCE_ATTRIBUTE    = default_attributes[60].hash;
extern const uint32_t TEXTURE_ATTRIBUTE             = default_attributes[61].hash;

extern const uint32_t NAME_ATTRIBUTE                = default_attributes[62].hash;
extern const uint32_t FOLDER_ATTRIBUTE              = default_attributes[63].hash;

extern const uint32_t FLICKER_ATTRIBUTE             = default_attributes[64].hash;
extern const uint32_t FREQUENCY_ATTRIBUTE           = default_attributes[65].hash;
extern const uint32_t PERCENTAGE_ATTRIBUTE          = default_attributes[66].hash;

extern const uint32_t DESTROYED_TRIGGER_TYPE_ATTRIBUTE  = default_attributes[67].hash;
extern const uint32_t SHADOW_COLOR_ATTRIBUTE            = default_attributes[68].hash;
extern const uint32_t EMIT_ONCE_ATTRIBUTE               = default_attributes[69].hash;

extern const uint32_t OFFSET_ATTRIBUTE                  = default_attributes[70].hash;
extern const uint32_t ENTITY_FILE_ATTRIBUTE             = default_attributes[71].hash;

extern const uint32_t POOL_SIZE_ATTRIBUTE               = default_attributes[72].hash;
extern const uint32_t BLEND_MODE_ATTRIBUTE              = default_attributes[73].hash;
extern const uint32_t EMIT_RATE_ATTRIBUTE               = default_attributes[74].hash;
extern const uint32_t EMITTER_TYPE_ATTRIBUTE            = default_attributes[75].hash;
extern const uint32_t START_SIZE_SPREAD_ATTRIBUTE       = default_attributes[76].hash;
extern const uint32_t END_SIZE_SPREAD_ATTRIBUTE         = default_attributes[77].hash;
extern const uint32_t LIFE_INTERVAL_ATTRIBUTE           = default_attributes[78].hash;
extern const uint32_t DIRECTION_INTERVAL_ATTRIBUTE      = default_attributes[79].hash;
extern const uint32_t MAGNITUDE_INTERVAL_ATTRIBUTE      = default_attributes[80].hash;
extern const uint32_t ANGLAR_VELOCITY_INTERVAL_ATTRIBUTE    = default_attributes[81].hash;
extern const uint32_t GRADIENT4_ATTRIBUTE                   = default_attributes[82].hash;
extern const uint32_t DAMPING_ATTRIBUTE                     = default_attributes[83].hash;
extern const uint32_t TRANSFORM_SPACE_ATTRIBUTE             = default_attributes[84].hash;
extern const uint32_t PARTICLE_DRAW_LAYER                   = default_attributes[85].hash;
extern const uint32_t EDITOR_PROPERTIES_ATTRIBUTE           = default_attributes[86].hash;
extern const uint32_t SPAWN_POINTS_ATTRIBUTE                = default_attributes[87].hash;
extern const uint32_t WEAPON_PRIMARY_ATTRIBUTE              = default_attributes[88].hash;
extern const uint32_t WEAPON_SECONDARY_ATTRIBUTE            = default_attributes[89].hash;
extern const uint32_t WEAPON_TERTIARY_ATTRIBUTE             = default_attributes[90].hash;

extern const uint32_t SOUND_ATTRIBUTE                       = default_attributes[91].hash;
extern const uint32_t SOUND_PLAY_PARAMETERS                 = default_attributes[92].hash;
extern const uint32_t SUB_TEXT_ATTRIBUTE                    = default_attributes[93].hash;
extern const uint32_t POLYGON_DRAW_LAYER_ATTRIBUTE          = default_attributes[94].hash;
extern const uint32_t UI_GROUP_ATTRIBUTE                    = default_attributes[95].hash;
extern const uint32_t UI_ITEM_STATE_ATTRIBUTE               = default_attributes[96].hash;
extern const uint32_t UI_LEFT_ITEM_ID_ATTRIBUTE             = default_attributes[97].hash;
extern const uint32_t UI_RIGHT_ITEM_ID_ATTRIBUTE            = default_attributes[98].hash;
extern const uint32_t UI_ABOVE_ITEM_ID_ATTRIBUTE            = default_attributes[99].hash;
extern const uint32_t UI_BELOW_ITEM_ID_ATTRIBUTE            = default_attributes[100].hash;
extern const uint32_t UNIFORM_DIRECTION_ATTRIBUTE           = default_attributes[101].hash;
extern const uint32_t EMITTER_MODE_ATTRIBUTE                = default_attributes[102].hash;
extern const uint32_t ENTITY_TYPE_ATTRIBUTE                 = default_attributes[103].hash;
extern const uint32_t PRIORITY_ATTRIBUTE                    = default_attributes[104].hash;
extern const uint32_t TARGET_FACTION_ATTRIBUTE              = default_attributes[105].hash;
extern const uint32_t SPAWN_LIMIT_CONCURRENT_ATTRIBUTE      = default_attributes[106].hash;


extern const uint32_t NULL_COMPONENT                = hash::Hash("null");
extern const uint32_t NAME_FOLDER_COMPONENT         = hash::Hash("name_folder");
extern const uint32_t TRANSFORM_COMPONENT           = hash::Hash("transform");
extern const uint32_t LAYER_COMPONENT               = hash::Hash("layer");
extern const uint32_t TAG_COMPONENT                 = hash::Hash("tag");
extern const uint32_t SPRITE_COMPONENT              = hash::Hash("sprite");
extern const uint32_t TEXT_COMPONENT                = hash::Hash("text");
extern const uint32_t PHYSICS_COMPONENT             = hash::Hash("physics");
extern const uint32_t CIRCLE_SHAPE_COMPONENT        = hash::Hash("circle_shape");
extern const uint32_t BOX_SHAPE_COMPONENT           = hash::Hash("box_shape");
extern const uint32_t SEGMENT_SHAPE_COMPONENT       = hash::Hash("segment_shape");
extern const uint32_t POLYGON_SHAPE_COMPONENT       = hash::Hash("polygon_shape");
extern const uint32_t HEALTH_COMPONENT              = hash::Hash("health");
extern const uint32_t BEHAVIOUR_COMPONENT           = hash::Hash("entity_behaviour");
extern const uint32_t SPAWN_POINT_COMPONENT         = hash::Hash("spawn_point");
extern const uint32_t ENTITY_SPAWN_POINT_COMPONENT  = hash::Hash("entity_spawn_point");
extern const uint32_t DESPAWN_ENTITY_COMPONENT      = hash::Hash("despawn_entity");
extern const uint32_t SHAPE_TRIGGER_COMPONENT       = hash::Hash("shape_trigger");
extern const uint32_t DESTROYED_TRIGGER_COMPONENT   = hash::Hash("destroyed_trigger");
extern const uint32_t AREA_TRIGGER_COMPONENT        = hash::Hash("area_entity_trigger");
extern const uint32_t TIME_TRIGGER_COMPONENT        = hash::Hash("time_trigger");
extern const uint32_t COUNTER_TRIGGER_COMPONENT     = hash::Hash("counter_trigger");
extern const uint32_t RELAY_TRIGGER_COMPONENT       = hash::Hash("relay_trigger");
extern const uint32_t PICKUP_COMPONENT              = hash::Hash("pickup");
extern const uint32_t LOOTBOX_COMPONENT             = hash::Hash("loot_box");
extern const uint32_t ANIMATION_COMPONENT           = hash::Hash("set_animation");
extern const uint32_t TRANSLATION_COMPONENT         = hash::Hash("set_translation");
extern const uint32_t ROTATION_COMPONENT            = hash::Hash("set_rotation");
extern const uint32_t SCALE_COMPONENT               = hash::Hash("set_scale");
extern const uint32_t CAMERA_ZOOM_COMPONENT         = hash::Hash("camera_zoom");
extern const uint32_t CAMERA_POINT_COMPONENT        = hash::Hash("camera_point");
extern const uint32_t CAMERA_TRACK_ENTITY_COMPONENT = hash::Hash("camera_track_entity");
extern const uint32_t CAMERA_RESTORE_COMPONENT      = hash::Hash("camera_restore");
extern const uint32_t INTERACTION_COMPONENT         = hash::Hash("interaction");
extern const uint32_t INTERACTION_SWITCH_COMPONENT  = hash::Hash("interaction_switch");
extern const uint32_t PATH_COMPONENT                = hash::Hash("path");
extern const uint32_t ROAD_COMPONENT                = hash::Hash("road");
extern const uint32_t LIGHT_COMPONENT               = hash::Hash("light");
extern const uint32_t DIALOG_COMPONENT              = hash::Hash("message");
extern const uint32_t PARTICLE_SYSTEM_COMPONENT     = hash::Hash("particle_system");
extern const uint32_t AREA_EMITTER_COMPONENT        = hash::Hash("area_emitter");
extern const uint32_t TEXTURED_POLYGON_COMPONENT    = hash::Hash("textured_polygon");
extern const uint32_t WEAPON_LOADOUT_COMPONENT      = hash::Hash("weapon_loadout");
extern const uint32_t SOUND_COMPONENT               = hash::Hash("sound");
extern const uint32_t UI_ITEM_COMPONENT             = hash::Hash("ui_item");
extern const uint32_t UI_SET_GROUP_STATE_COMPONENT  = hash::Hash("ui_set_group_state");
extern const uint32_t REGION_COMPONENT              = hash::Hash("region");
extern const uint32_t TELEPORT_PLAYER_COMPONENT     = hash::Hash("teleport_player");
extern const uint32_t ENTITY_TRACKING_COMPONENT     = hash::Hash("entity_tracker");
extern const uint32_t TARGET_COMPONENT              = hash::Hash("target");

const char* component::ComponentNameFromHash(uint32_t hash)
{
    if(hash == NULL_COMPONENT)
        return "null";
    if(hash == NAME_FOLDER_COMPONENT)
        return "name_folder";
    else if(hash == TRANSFORM_COMPONENT)
        return "transform";
    else if(hash == LAYER_COMPONENT)
        return "layer";
    else if(hash == TAG_COMPONENT)
        return "tag";
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
    else if(hash == ENTITY_SPAWN_POINT_COMPONENT)
        return "entity_spawn_point";
    else if(hash == DESPAWN_ENTITY_COMPONENT)
        return "despawn_entity";
    else if(hash == SHAPE_TRIGGER_COMPONENT)
        return "shape_trigger";
    else if(hash == DESTROYED_TRIGGER_COMPONENT)
        return "destroyed_trigger";
    else if(hash == AREA_TRIGGER_COMPONENT)
        return "area_entity_trigger";
    else if(hash == TIME_TRIGGER_COMPONENT)
        return "time_trigger";
    else if(hash == COUNTER_TRIGGER_COMPONENT)
        return "counter_trigger";
    else if(hash == RELAY_TRIGGER_COMPONENT)
        return "relay_trigger";
    else if(hash == PICKUP_COMPONENT)
        return "pickup";
    else if(hash == LOOTBOX_COMPONENT)
        return "loot_box";
    else if(hash == ANIMATION_COMPONENT)
        return "set_animation";
    else if(hash == TRANSLATION_COMPONENT)
        return "set_translation";
    else if(hash == ROTATION_COMPONENT)
        return "set_rotation";
    else if(hash == SCALE_COMPONENT)
        return "set_scale";
    else if(hash == CAMERA_ZOOM_COMPONENT)
        return "camera_zoom";
    else if(hash == CAMERA_POINT_COMPONENT)
        return "camera_point";
    else if(hash == CAMERA_TRACK_ENTITY_COMPONENT)
        return "camera_track_entity";
    else if(hash == CAMERA_RESTORE_COMPONENT)
        return "camera_restore";
    else if(hash == INTERACTION_COMPONENT)
        return "interaction";
    else if(hash == INTERACTION_SWITCH_COMPONENT)
        return "interaction_switch";
    else if(hash == PATH_COMPONENT)
        return "path";
    else if(hash == ROAD_COMPONENT)
        return "road";
    else if(hash == LIGHT_COMPONENT)
        return "light";
    else if(hash == DIALOG_COMPONENT)
        return "dialog";
    else if(hash == PARTICLE_SYSTEM_COMPONENT)
        return "particle_system";
    else if(hash == AREA_EMITTER_COMPONENT)
        return "area_emitter";
    else if(hash == TEXTURED_POLYGON_COMPONENT)
        return "textured_polygon";
    else if(hash == WEAPON_LOADOUT_COMPONENT)
        return "weapon_loadout";
    else if(hash == SOUND_COMPONENT)
        return "sound";
    else if(hash == UI_ITEM_COMPONENT)
        return "ui_item";
    else if(hash == UI_SET_GROUP_STATE_COMPONENT)
        return "ui_set_group_state";
    else if(hash == REGION_COMPONENT)
        return "region";
    else if(hash == TELEPORT_PLAYER_COMPONENT)
        return "teleport_player";
    else if(hash == ENTITY_TRACKING_COMPONENT)
        return "entity_tracker";
    else if(hash == TARGET_COMPONENT)
        return "target";

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

const Component default_components[] = {
    MakeComponent(NAME_FOLDER_COMPONENT,        NULL_COMPONENT,             false,  "general",      { NAME_ATTRIBUTE, FOLDER_ATTRIBUTE, EDITOR_PROPERTIES_ATTRIBUTE }),
    MakeComponent(TRANSFORM_COMPONENT,          NULL_COMPONENT,             false,  "general",      { POSITION_ATTRIBUTE, ROTATION_ATTRIBUTE, SCALE_ATTRIBUTE, ENTITY_REFERENCE_ATTRIBUTE }),
    MakeComponent(TAG_COMPONENT,                NULL_COMPONENT,             false,  "general",      { TAG_ATTRIBUTE }),
    MakeComponent(HEALTH_COMPONENT,             NULL_COMPONENT,             false,  "general",      { HEALTH_ATTRIBUTE, BOSS_HEALTH_ATTRIBUTE }),
    MakeComponent(INTERACTION_COMPONENT,        NULL_COMPONENT,             false,  "general",      { INTERACTION_TYPE_ATTRIBUTE, SOUND_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE, DRAW_NAME_ATTRIBUTE }),
    MakeComponent(INTERACTION_SWITCH_COMPONENT, NULL_COMPONENT,             false,  "general",      { INTERACTION_TYPE_ATTRIBUTE, SOUND_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE, TRIGGER_NAME_EXIT_ATTRIBUTE, DRAW_NAME_ATTRIBUTE }),
    MakeComponent(PATH_COMPONENT,               NULL_COMPONENT,             false,  "general",      { PATH_TYPE_ATTRIBUTE, PATH_POINTS_ATTRIBUTE, PATH_CLOSED_ATTRIBUTE }),
    MakeComponent(SOUND_COMPONENT,              NULL_COMPONENT,             false,  "general",      { SOUND_ATTRIBUTE, SOUND_PLAY_PARAMETERS, ENABLE_TRIGGER_ATTRIBUTE, DISABLE_TRIGGER_ATTRIBUTE }),
    MakeComponent(ENTITY_TRACKING_COMPONENT,    NULL_COMPONENT,             false,  "general",      { ENTITY_TYPE_ATTRIBUTE } ),

    MakeComponent(PICKUP_COMPONENT,             PHYSICS_COMPONENT,          false,  "pickups",      { PICKUP_TYPE_ATTRIBUTE, AMOUNT_ATTRIBUTE }),
    MakeComponent(LOOTBOX_COMPONENT,            NULL_COMPONENT,             false,  "pickups",      { }),

    MakeComponent(LAYER_COMPONENT,              NULL_COMPONENT,             false,  "rendering",    { LAYER_ATTRIBUTE, SORT_OFFSET_ATTRIBUTE }),
    MakeComponent(SPRITE_COMPONENT,             NULL_COMPONENT,             false,  "rendering",    { SPRITE_ATTRIBUTE, ANIMATION_ATTRIBUTE, COLOR_ATTRIBUTE, SPRITE_PROPERTIES_ATTRIBUTE, SHADOW_OFFSET_ATTRIBUTE, SHADOW_SIZE_ATTRIBUTE, RANDOM_START_FRAME_ATTRIBUTE }),
    MakeComponent(TEXT_COMPONENT,               NULL_COMPONENT,             false,  "rendering",    { TEXT_ATTRIBUTE, FONT_ID_ATTRIBUTE, COLOR_ATTRIBUTE, CENTER_FLAGS_ATTRIBUTE, TEXT_SHADOW_ATTRIBUTE, OFFSET_ATTRIBUTE, SHADOW_COLOR_ATTRIBUTE }),
    MakeComponent(ROAD_COMPONENT,               PATH_COMPONENT,             false,  "rendering",    { WIDTH_ATTRIBUTE, COLOR_ATTRIBUTE, TEXTURE_ATTRIBUTE }),
    MakeComponent(LIGHT_COMPONENT,              NULL_COMPONENT,             false,  "rendering",    { RADIUS_ATTRIBUTE, OFFSET_ATTRIBUTE, COLOR_ATTRIBUTE, FLICKER_ATTRIBUTE, FREQUENCY_ATTRIBUTE, PERCENTAGE_ATTRIBUTE }),
    MakeComponent(DIALOG_COMPONENT,             NULL_COMPONENT,             false,  "rendering",    { TEXT_ATTRIBUTE, DURATION_ATTRIBUTE }),
    MakeComponent(PARTICLE_SYSTEM_COMPONENT,    NULL_COMPONENT,             false,  "rendering",    { POOL_SIZE_ATTRIBUTE, TEXTURE_ATTRIBUTE, BLEND_MODE_ATTRIBUTE, PARTICLE_DRAW_LAYER, TRANSFORM_SPACE_ATTRIBUTE, DAMPING_ATTRIBUTE }),
    MakeComponent(AREA_EMITTER_COMPONENT,       PARTICLE_SYSTEM_COMPONENT,  false,  "rendering",    { EMITTER_MODE_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE, DURATION_ATTRIBUTE, EMIT_RATE_ATTRIBUTE, EMITTER_TYPE_ATTRIBUTE, SIZE_ATTRIBUTE, DIRECTION_INTERVAL_ATTRIBUTE, UNIFORM_DIRECTION_ATTRIBUTE, MAGNITUDE_INTERVAL_ATTRIBUTE, ANGLAR_VELOCITY_INTERVAL_ATTRIBUTE, LIFE_INTERVAL_ATTRIBUTE, GRADIENT4_ATTRIBUTE, START_SIZE_SPREAD_ATTRIBUTE, END_SIZE_SPREAD_ATTRIBUTE }),

    MakeComponent(UI_ITEM_COMPONENT,            NULL_COMPONENT,             false,  "ui",           { UI_GROUP_ATTRIBUTE, UI_ITEM_STATE_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE, UI_LEFT_ITEM_ID_ATTRIBUTE, UI_RIGHT_ITEM_ID_ATTRIBUTE, UI_ABOVE_ITEM_ID_ATTRIBUTE, UI_BELOW_ITEM_ID_ATTRIBUTE} ),
    MakeComponent(UI_SET_GROUP_STATE_COMPONENT, NULL_COMPONENT,             false,  "ui",           { UI_GROUP_ATTRIBUTE, UI_ITEM_STATE_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE } ),

    MakeComponent(PHYSICS_COMPONENT,            NULL_COMPONENT,             false,  "physics",      { BODY_TYPE_ATTRIBUTE, MASS_ATTRIBUTE, INERTIA_ATTRIBUTE, PREVENT_ROTATION_ATTRIBUTE }),
    MakeComponent(BOX_SHAPE_COMPONENT,          PHYSICS_COMPONENT,          true,   "physics",      { FACTION_ATTRIBUTE, SIZE_ATTRIBUTE, POSITION_ATTRIBUTE, SENSOR_ATTRIBUTE }),
    MakeComponent(CIRCLE_SHAPE_COMPONENT,       PHYSICS_COMPONENT,          true,   "physics",      { FACTION_ATTRIBUTE, RADIUS_ATTRIBUTE, POSITION_ATTRIBUTE, SENSOR_ATTRIBUTE }),
    MakeComponent(POLYGON_SHAPE_COMPONENT,      PHYSICS_COMPONENT,          true,   "physics",      { FACTION_ATTRIBUTE, POLYGON_ATTRIBUTE, SENSOR_ATTRIBUTE }),
    MakeComponent(SEGMENT_SHAPE_COMPONENT,      PHYSICS_COMPONENT,          true,   "physics",      { FACTION_ATTRIBUTE, START_ATTRIBUTE, END_ATTRIBUTE, RADIUS_ATTRIBUTE, SENSOR_ATTRIBUTE }),

    MakeComponent(AREA_TRIGGER_COMPONENT,       NULL_COMPONENT,             false,  "triggers",     { SIZE_ATTRIBUTE, FACTION_PICKER_ATTRIBUTE, LOGIC_OP_ATTRIBUTE, N_ENTITIES_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE }),
    MakeComponent(COUNTER_TRIGGER_COMPONENT,    NULL_COMPONENT,             false,  "triggers",     { COUNT_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE, TRIGGER_NAME_COMPLETED_ATTRIBUTE, RESET_ON_COMPLETED_ATTRIBUTE }),
    MakeComponent(DESTROYED_TRIGGER_COMPONENT,  NULL_COMPONENT,             false,  "triggers",     { DESTROYED_TRIGGER_TYPE_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE }),
    MakeComponent(SHAPE_TRIGGER_COMPONENT,      NULL_COMPONENT,             false,  "triggers",     { FACTION_PICKER_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE, TRIGGER_NAME_EXIT_ATTRIBUTE, EMIT_ONCE_ATTRIBUTE }),
    MakeComponent(TIME_TRIGGER_COMPONENT,       NULL_COMPONENT,             false,  "triggers",     { TIME_STAMP_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE, REPEATING_ATTRIBUTE }),
    MakeComponent(RELAY_TRIGGER_COMPONENT,      NULL_COMPONENT,             false,  "triggers",     { TRIGGER_NAME_ATTRIBUTE, TIME_STAMP_ATTRIBUTE, TRIGGER_NAME_COMPLETED_ATTRIBUTE }),

    MakeComponent(SPAWN_POINT_COMPONENT,        NULL_COMPONENT,             false,  "spawning",     { SPAWN_SCORE_ATTRIBUTE, SPAWN_LIMIT_ATTRIBUTE, SPAWN_LIMIT_CONCURRENT_ATTRIBUTE, RADIUS_ATTRIBUTE, TIME_STAMP_ATTRIBUTE, ENABLE_TRIGGER_ATTRIBUTE, DISABLE_TRIGGER_ATTRIBUTE, SPAWN_POINTS_ATTRIBUTE }),
    MakeComponent(ENTITY_SPAWN_POINT_COMPONENT, NULL_COMPONENT,             false,  "spawning",     { ENTITY_FILE_ATTRIBUTE, RADIUS_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE }),
    MakeComponent(DESPAWN_ENTITY_COMPONENT,     NULL_COMPONENT,             false,  "spawning",     { TRIGGER_NAME_ATTRIBUTE } ),

    MakeComponent(ANIMATION_COMPONENT,          SPRITE_COMPONENT,           true,   "animation",    { TRIGGER_NAME_ATTRIBUTE, ANIMATION_ATTRIBUTE }),
    MakeComponent(TRANSLATION_COMPONENT,        NULL_COMPONENT,             true,   "animation",    { ANIMATION_MODE_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE, DURATION_ATTRIBUTE, POSITION_ATTRIBUTE, EASING_FUNC_ATTRIBUTE }),
    MakeComponent(ROTATION_COMPONENT,           NULL_COMPONENT,             true,   "animation",    { ANIMATION_MODE_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE, DURATION_ATTRIBUTE, ROTATION_ATTRIBUTE, EASING_FUNC_ATTRIBUTE }),
    MakeComponent(SCALE_COMPONENT,              NULL_COMPONENT,             true,   "animation",    { ANIMATION_MODE_ATTRIBUTE, TRIGGER_NAME_ATTRIBUTE, DURATION_ATTRIBUTE, SCALE_ATTRIBUTE, EASING_FUNC_ATTRIBUTE }),

    MakeComponent(CAMERA_POINT_COMPONENT,       NULL_COMPONENT,             false,  "camera",       { TRIGGER_NAME_ATTRIBUTE }),
    MakeComponent(CAMERA_ZOOM_COMPONENT,        NULL_COMPONENT,             false,  "camera",       { TRIGGER_NAME_ATTRIBUTE, ZOOM_LEVEL_ATTRIBUTE }),
    MakeComponent(CAMERA_TRACK_ENTITY_COMPONENT,NULL_COMPONENT,             false,  "camera",       { TRIGGER_NAME_ATTRIBUTE, ENTITY_REFERENCE_ATTRIBUTE }),
    MakeComponent(CAMERA_RESTORE_COMPONENT,     NULL_COMPONENT,             false,  "camera",       { TRIGGER_NAME_ATTRIBUTE }),

    MakeComponent(TEXTURED_POLYGON_COMPONENT,   NULL_COMPONENT,             false,  "world",        { TEXTURE_ATTRIBUTE, COLOR_ATTRIBUTE, POLYGON_DRAW_LAYER_ATTRIBUTE, POLYGON_ATTRIBUTE }),
    MakeComponent(REGION_COMPONENT,             PHYSICS_COMPONENT,          false,  "world",        { TEXT_ATTRIBUTE, SUB_TEXT_ATTRIBUTE }),

    MakeComponent(TELEPORT_PLAYER_COMPONENT,    NULL_COMPONENT,             false,  "logic",        { TRIGGER_NAME_ATTRIBUTE } ),
    MakeComponent(WEAPON_LOADOUT_COMPONENT,     NULL_COMPONENT,             false,  "logic",        { WEAPON_PRIMARY_ATTRIBUTE, WEAPON_SECONDARY_ATTRIBUTE, WEAPON_TERTIARY_ATTRIBUTE } ),
    MakeComponent(TARGET_COMPONENT,             NULL_COMPONENT,             false,  "logic",        { TARGET_FACTION_ATTRIBUTE, PRIORITY_ATTRIBUTE } ),
    MakeComponent(BEHAVIOUR_COMPONENT,          NULL_COMPONENT,             false,  "logic",        { ENTITY_BEHAVIOUR_ATTRIBUTE }),
};

const char* AttributeNameFromHash(uint32_t hash)
{
    for(const DefaultAttribute& hash_string : default_attributes)
    {
        if(hash_string.hash == hash)
            return hash_string.string;
    }

    MONO_ASSERT(false);
    return "Unknown";
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
    System::Log("Component|Unable to find default attribute for hash: %u (%s)", hash, attribute_name);
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
        else
            result_attributes.push_back(input);
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

Component component::DefaultComponentFromHash(uint32_t hash)
{
    const auto find_template = [hash](const Component& component_template) {
        return component_template.hash == hash;
    };

    const auto it = std::find_if(std::begin(default_components), std::end(default_components), find_template);
    if(it != std::end(default_components))
        return *it;

    const char* component_name = component::ComponentNameFromHash(hash);
    System::Log("Component|Unable to find default component for hash: %u (%s)", hash, component_name);

    return Component();
}

Component* component::FindComponentFromHash(uint32_t hash, std::vector<Component>& components)
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

const Component* component::FindComponentFromHash(uint32_t hash, const std::vector<Component>& components)
{
    return FindComponentFromHash(hash, (std::vector<Component>&)components);
}

void component::StripUnknownProperties(Component& component)
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
        Component new_component = component::DefaultComponentFromHash(component_hash);

        if(new_component.depends_on != NULL_COMPONENT)
        {
            const Component* found_dependency = component::FindComponentFromHash(new_component.depends_on, components);
            if(!found_dependency)
                add_component_recursivly(new_component.depends_on, components);
        }

        components.push_back(std::move(new_component));
    };
}

std::vector<const Component*> component::GetAllDefaultComponents()
{
    std::vector<const Component*> components;

    for(const Component& component : default_components)
        components.push_back(&component);

    return components;
}

std::vector<Component*> component::AddComponent(uint32_t hash, std::vector<Component>& components)
{
    const uint32_t num_components = components.size();
    add_component_recursivly(hash, components);
    const uint32_t n_added = components.size() - num_components;

    std::vector<Component*> added_components;
    added_components.reserve(n_added);

    for(uint32_t index = num_components; index < components.size(); ++index)
        added_components.push_back(&components[index]);

    return added_components;
}

int component::ComponentPriorityForHash(uint32_t hash)
{
    const auto find_func = [hash](const Component& component) {
        return component.hash == hash;
    };
    const auto it = std::find_if(std::begin(default_components), std::end(default_components), find_func);
    return std::distance(std::begin(default_components), it);
}

void component::SortComponentsByPriority(std::vector<Component>& components)
{
    const auto sort_by_prio = [](const Component& left, const Component& right) {
        return ComponentPriorityForHash(left.hash) < ComponentPriorityForHash(right.hash);
    };
    std::sort(components.begin(), components.end(), sort_by_prio);
}

