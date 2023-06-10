
#include "ComponentDrawFuncs.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Quad.h"
#include "Math/MathFunctions.h"

#include "CollisionConfiguration.h"
#include "TriggerSystem/TriggerTypes.h"

#include "Entity/Component.h"
#include "FontIds.h"

namespace
{
    std::vector<math::Vector> GenerateArc(const math::Vector& position, float start_radians, float end_radians, int segments)
    {
        std::vector<math::Vector> vertices;
        vertices.reserve(segments);

        const float radians_increment = (end_radians - start_radians) / segments;
        
        for(int index = 0; index <= segments; ++index)
        {
            const float radians = index * radians_increment + start_radians;
            const float x = -std::sin(radians) + position.x;
            const float y = std::cos(radians) + position.y;
            vertices.emplace_back(x, y);
        }

        return vertices;
    }

    constexpr mono::Color::RGBA g_collision_color = mono::Color::RGBA(1.0f, 0.0f, 1.0f, 0.25f);
    constexpr mono::Color::RGBA g_sensor_color = mono::Color::RGBA(0.0f, 0.0f, 1.0f, 0.25f);
    constexpr mono::Color::RGBA g_area_trigger_color = mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.25f);
    constexpr mono::Color::RGBA g_trigger_name_color = mono::Color::OFF_WHITE;
    
    constexpr mono::Color::RGBA g_spawn_point_color = mono::Color::RGBA(0.0f, 0.0f, 0.8f, 0.5f);
}

void editor::DrawCircleShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    float radius_value = 1.0f;
    math::Vector offset;
    bool is_sensor;

    FindAttribute(RADIUS_ATTRIBUTE, component_properties, radius_value, FallbackMode::SET_DEFAULT);
    FindAttribute(POSITION_ATTRIBUTE, component_properties, offset, FallbackMode::SET_DEFAULT);
    FindAttribute(SENSOR_ATTRIBUTE, component_properties, is_sensor, FallbackMode::SET_DEFAULT);

    radius_value = std::max(radius_value, 0.0001f);

    const mono::Color::RGBA color = is_sensor ? g_sensor_color : g_collision_color;
    renderer.DrawFilledCircle(offset, math::Vector(radius_value, radius_value), 20, color);
}

void editor::DrawBoxShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    math::Vector width_height;
    math::Vector offset;
    bool is_sensor;

    FindAttribute(SIZE_ATTRIBUTE, component_properties, width_height, FallbackMode::SET_DEFAULT);
    FindAttribute(POSITION_ATTRIBUTE, component_properties, offset, FallbackMode::SET_DEFAULT);
    FindAttribute(SENSOR_ATTRIBUTE, component_properties, is_sensor, FallbackMode::SET_DEFAULT);

    const math::Vector half_size = width_height / 2.0f;

    math::Quad box;
    box.bottom_left = offset - half_size;
    box.top_right = offset + half_size;

    const mono::Color::RGBA color = is_sensor ? g_sensor_color : g_collision_color;
    renderer.DrawFilledQuad(box, color);
    renderer.DrawQuad(box, mono::Color::BLACK, 1.0f);
}

void editor::DrawSegmentShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    math::Vector start;
    math::Vector end;
    float radius = 1.0f;
    bool is_sensor;

    FindAttribute(START_ATTRIBUTE, component_properties, start, FallbackMode::SET_DEFAULT);
    FindAttribute(END_ATTRIBUTE, component_properties, end, FallbackMode::SET_DEFAULT);
    FindAttribute(RADIUS_ATTRIBUTE, component_properties, radius, FallbackMode::SET_DEFAULT);
    FindAttribute(SENSOR_ATTRIBUTE, component_properties, is_sensor, FallbackMode::SET_DEFAULT);

    const std::vector<math::Vector> line = { start, end };
    const mono::Color::RGBA color = is_sensor ? g_sensor_color : g_collision_color;
    renderer.DrawLines(line, color, 10.0f);
}

void editor::DrawPolygonShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    std::vector<math::Vector> polygon;
    const bool found_polygon = FindAttribute(POLYGON_ATTRIBUTE, component_properties, polygon, FallbackMode::REQUIRE_ATTRIBUTE);
    if(!found_polygon)
        return;

    bool is_sensor;
    FindAttribute(SENSOR_ATTRIBUTE, component_properties, is_sensor, FallbackMode::SET_DEFAULT);

    const mono::Color::RGBA color = mono::Color::CYAN; //is_sensor ? g_sensor_color : g_collision_color;
    renderer.DrawClosedPolyline(polygon, color, 1.0f);
    if(is_sensor)
        renderer.RenderText(game::FontId::PIXELETTE_SMALL, "sensor", mono::Color::BLUE, mono::FontCentering::HORIZONTAL_VERTICAL);
}

void editor::DrawSpawnPointDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    float radius = 1.0f;
    FindAttribute(RADIUS_ATTRIBUTE, component_properties, radius, FallbackMode::SET_DEFAULT);

    std::vector<math::Vector> spawn_points;
    FindAttribute(SPAWN_POINTS_ATTRIBUTE, component_properties, spawn_points, FallbackMode::SET_DEFAULT);

    for(const math::Vector& spawn_point : spawn_points)
        renderer.DrawFilledCircle(spawn_point, math::Vector(radius, radius), 16, g_spawn_point_color);
}

void editor::DrawEntitySpawnPointDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    float radius = 1.0f;
    FindAttribute(RADIUS_ATTRIBUTE, component_properties, radius, FallbackMode::SET_DEFAULT);
    renderer.DrawFilledCircle(math::ZeroVec, math::Vector(radius, radius), 16, g_spawn_point_color);
}

void editor::DrawShapeTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    mono::Event name;
    FindAttribute(TRIGGER_NAME_ATTRIBUTE, component_properties, name, FallbackMode::SET_DEFAULT);
    renderer.RenderText(game::FontId::PIXELETTE_TINY, name.text.c_str(), g_trigger_name_color, mono::FontCentering::HORIZONTAL_VERTICAL);
}

void editor::DrawAreaTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    math::Vector width_height;
    FindAttribute(SIZE_ATTRIBUTE, component_properties, width_height, FallbackMode::SET_DEFAULT);

    mono::Event event;
    FindAttribute(TRIGGER_NAME_ATTRIBUTE, component_properties, event, FallbackMode::SET_DEFAULT);

    uint32_t faction;
    FindAttribute(FACTION_PICKER_ATTRIBUTE, component_properties, faction, FallbackMode::SET_DEFAULT);
    const char* faction_string = game::CollisionCategoryToString(faction);

    int trigger_op;
    FindAttribute(LOGIC_OP_ATTRIBUTE, component_properties, trigger_op, FallbackMode::SET_DEFAULT);
    const char* op_string = game::AreaTriggerOpToString(game::AreaTriggerOperation(trigger_op));

    int n_entities;
    FindAttribute(N_ENTITIES_ATTRIBUTE, component_properties, n_entities, FallbackMode::SET_DEFAULT);

    const math::Vector half_width_height = width_height / 2.0f;
    const math::Quad area = math::Quad(-half_width_height, half_width_height);
    renderer.DrawFilledQuad(area, g_area_trigger_color);
    renderer.DrawQuad(area, mono::Color::BLACK, 1.0f);

    char text_buffer[1024] = {};
    std::snprintf(text_buffer, std::size(text_buffer), "%s %s %u -> %s", faction_string, op_string, n_entities, event.text.c_str());
    renderer.RenderText(game::FontId::PIXELETTE_TINY, text_buffer, g_trigger_name_color, mono::FontCentering::HORIZONTAL_VERTICAL);
}

void editor::DrawDestroyedTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    mono::Event name;
    FindAttribute(TRIGGER_NAME_ATTRIBUTE, component_properties, name, FallbackMode::SET_DEFAULT);
    renderer.RenderText(game::FontId::PIXELETTE_TINY, name.text.c_str(), g_trigger_name_color, mono::FontCentering::HORIZONTAL_VERTICAL);
}

void editor::DrawTimeTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    mono::Event name;
    FindAttribute(TRIGGER_NAME_ATTRIBUTE, component_properties, name, FallbackMode::SET_DEFAULT);
    renderer.RenderText(game::FontId::PIXELETTE_TINY, name.text.c_str(), g_trigger_name_color, mono::FontCentering::HORIZONTAL_VERTICAL);
}

void editor::DrawCounterTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    mono::Event name;
    mono::Event completed_name;
    FindAttribute(TRIGGER_NAME_ATTRIBUTE, component_properties, name, FallbackMode::SET_DEFAULT);
    FindAttribute(TRIGGER_NAME_COMPLETED_ATTRIBUTE, component_properties, completed_name, FallbackMode::SET_DEFAULT);

    int count;
    FindAttribute(COUNT_ATTRIBUTE, component_properties, count, FallbackMode::SET_DEFAULT);

    const std::string output = std::to_string(count) + " x " + name.text + " -> " + completed_name.text;
    renderer.RenderText(game::FontId::PIXELETTE_TINY, output.c_str(), g_trigger_name_color, mono::FontCentering::HORIZONTAL_VERTICAL);
}

void editor::DrawSetTranslationDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    math::Vector delta_position;
    FindAttribute(POSITION_ATTRIBUTE, component_properties, delta_position, FallbackMode::SET_DEFAULT);

    renderer.DrawLines({ math::ZeroVec, delta_position }, mono::Color::CYAN, 2.0f);
    renderer.DrawPoints({ delta_position }, mono::Color::CYAN, 10.0f);
}

void editor::DrawSetRotationDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    float delta_rotation;
    FindAttribute(ROTATION_ATTRIBUTE, component_properties, delta_rotation, FallbackMode::SET_DEFAULT);

    const std::vector<math::Vector>& arc_points = GenerateArc(math::ZeroVec, 0.0f, delta_rotation, 20);
    renderer.DrawPolyline(arc_points, mono::Color::CYAN, 2.0f);
    renderer.DrawPoints({ arc_points.back() }, mono::Color::CYAN, 10.0f);
}

void editor::DrawLayerDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    float sort_offset = 0.0f;
    FindAttribute(SORT_OFFSET_ATTRIBUTE, component_properties, sort_offset, FallbackMode::SET_DEFAULT);

    const bool is_zero = math::IsPrettyMuchEquals(sort_offset, 0.0f);
    if(is_zero)
        return;

    const float half_width = math::Width(entity_bb) / 2.0f;
    const float bottom = math::Bottom(entity_bb);

    const std::vector<math::Vector> line = {
        { -half_width, bottom + sort_offset },
        { +half_width, bottom + sort_offset }
    };
    renderer.DrawLines(line, mono::Color::GREEN, 1.0f);
}

void editor::DrawAreaEmitterDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    math::Vector area_size;
    FindAttribute(SIZE_ATTRIBUTE, component_properties, area_size, FallbackMode::SET_DEFAULT);

    constexpr mono::Color::RGBA color = { 1.0f, 0.0f, 0.5f, 0.1f};
    const math::Quad area = math::Quad(-area_size / 2.0, area_size / 2.0f);

    renderer.DrawFilledQuad(area, color);
    renderer.DrawQuad(area, mono::Color::BLACK, 1.0f);
}

void editor::DrawPath(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    std::vector<math::Vector> vertices;
    const bool found_polygon = FindAttribute(PATH_POINTS_ATTRIBUTE, component_properties, vertices, FallbackMode::REQUIRE_ATTRIBUTE);
    if(!found_polygon)
        return;

    renderer.DrawPolyline(vertices, mono::Color::MAGENTA, 1.0f);
}

void editor::DrawCameraPoint(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    math::Vector point;
    FindAttribute(POSITION_ATTRIBUTE, component_properties, point, FallbackMode::SET_DEFAULT);
    renderer.DrawPoints({ math::ZeroVec }, mono::Color::CYAN, 10.0f);
}

void editor::DrawTeleportPlayerPoint(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    const float radius = 0.25f;
    renderer.DrawFilledCircle(math::ZeroVec, math::Vector(radius, radius), 16, mono::Color::MakeWithAlpha(mono::Color::GRAY, 0.5f));
    renderer.DrawCircle(math::ZeroVec, radius, 16, 1.0f, mono::Color::GOLDEN_YELLOW);
    renderer.DrawPoints({ math::ZeroVec }, mono::Color::GOLDEN_YELLOW, 16.0f);
}
