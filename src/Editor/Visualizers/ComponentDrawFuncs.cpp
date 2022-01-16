
#include "ComponentDrawFuncs.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Quad.h"
#include "Math/MathFunctions.h"

#include "Entity/Component.h"
#include "FontIds.h"

namespace
{
    std::vector<math::Vector> GenerateArrow(const math::Vector& start, const math::Vector& end, float scale)
    {
        const float angle = math::AngleBetweenPoints(start, end);

        const float x1 = ( std::sin(angle + math::PI() + math::PI_4())) * scale + end.x;
        const float y1 = (-std::cos(angle + math::PI() + math::PI_4())) * scale + end.y;

        const float x2 = ( std::sin(angle - math::PI_4())) * scale + end.x;
        const float y2 = (-std::cos(angle - math::PI_4())) * scale + end.y;

        return {
            math::Vector(x1, y1), end, end, math::Vector(x2, y2)
        };
    }

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

    mono::Color::RGBA color = is_sensor ? mono::Color::BLUE : mono::Color::MAGENTA;
    color.alpha = 0.5f;
    
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
    box.mA = offset - half_size;
    box.mB = offset + half_size;

    mono::Color::RGBA color = is_sensor ? mono::Color::BLUE : mono::Color::MAGENTA;
    color.alpha = 0.5f;

    renderer.DrawFilledQuad(box, color);
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

    if(is_sensor)
    {
        mono::Color::RGBA sensor_color = mono::Color::BLUE;
        sensor_color.alpha = 0.25f;
        renderer.DrawLines(line, sensor_color, 10.0f);
    }
    renderer.DrawLines(line, mono::Color::MAGENTA, std::max(radius, 1.0f));
}

void editor::DrawPolygonShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    std::vector<math::Vector> polygon;
    const bool found_polygon = FindAttribute(POLYGON_ATTRIBUTE, component_properties, polygon, FallbackMode::REQUIRE_ATTRIBUTE);
    if(!found_polygon)
        return;

    bool is_sensor;
    FindAttribute(SENSOR_ATTRIBUTE, component_properties, is_sensor, FallbackMode::SET_DEFAULT);

    renderer.DrawClosedPolyline(polygon, mono::Color::MAGENTA, 1.0f);
    if(is_sensor)
        renderer.RenderText(game::FontId::PIXELETTE_SMALL, "sensor", mono::Color::BLUE, mono::FontCentering::HORIZONTAL_VERTICAL);
}

void editor::DrawSpawnPointDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    float radius = 1.0f;
    FindAttribute(RADIUS_ATTRIBUTE, component_properties, radius, FallbackMode::SET_DEFAULT);
    renderer.DrawCircle(math::ZeroVec, radius, 8, 5.0f, mono::Color::BLUE);
    renderer.DrawLines({ math::ZeroVec, math::Vector(1.0f, 0.0f) }, mono::Color::BLUE, 3.0f);
}

void editor::DrawEntitySpawnPointDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    float radius = 1.0f;
    FindAttribute(RADIUS_ATTRIBUTE, component_properties, radius, FallbackMode::SET_DEFAULT);
    renderer.DrawCircle(math::ZeroVec, radius, 8, 5.0f, mono::Color::BLUE);
    renderer.DrawLines({ math::ZeroVec, math::Vector(1.0f, 0.0f) }, mono::Color::BLUE, 3.0f);
}

void editor::DrawShapeTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    std::string name;
    FindAttribute(TRIGGER_NAME_ATTRIBUTE, component_properties, name, FallbackMode::SET_DEFAULT);
    renderer.RenderText(game::FontId::PIXELETTE_TINY, name.c_str(), mono::Color::BLUE, mono::FontCentering::HORIZONTAL_VERTICAL);
}

void editor::DrawAreaTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    math::Vector width_height;
    FindAttribute(SIZE_ATTRIBUTE, component_properties, width_height, FallbackMode::SET_DEFAULT);

    std::string name;
    FindAttribute(TRIGGER_NAME_ATTRIBUTE, component_properties, name, FallbackMode::SET_DEFAULT);

    const math::Vector half_width_height = width_height / 2.0f;
    renderer.DrawFilledQuad(math::Quad(-half_width_height, half_width_height), mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.5f));
    renderer.RenderText(game::FontId::PIXELETTE_TINY, name.c_str(), mono::Color::BLUE, mono::FontCentering::HORIZONTAL_VERTICAL);
}

void editor::DrawDestroyedTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    std::string name;
    FindAttribute(TRIGGER_NAME_ATTRIBUTE, component_properties, name, FallbackMode::SET_DEFAULT);
    renderer.RenderText(game::FontId::PIXELETTE_TINY, name.c_str(), mono::Color::BLUE, mono::FontCentering::HORIZONTAL_VERTICAL);
}

void editor::DrawTimeTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    std::string name;
    FindAttribute(TRIGGER_NAME_ATTRIBUTE, component_properties, name, FallbackMode::SET_DEFAULT);
    renderer.RenderText(game::FontId::PIXELETTE_TINY, name.c_str(), mono::Color::BLUE, mono::FontCentering::HORIZONTAL_VERTICAL);
}

void editor::DrawCounterTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    std::string name;
    std::string completed_name;
    FindAttribute(TRIGGER_NAME_ATTRIBUTE, component_properties, name, FallbackMode::SET_DEFAULT);
    FindAttribute(TRIGGER_NAME_COMPLETED_ATTRIBUTE, component_properties, completed_name, FallbackMode::SET_DEFAULT);

    int count;
    FindAttribute(COUNT_ATTRIBUTE, component_properties, count, FallbackMode::SET_DEFAULT);

    const std::string output = std::to_string(count) + " x " + name + " -> " + completed_name;
    renderer.RenderText(game::FontId::PIXELETTE_TINY, output.c_str(), mono::Color::ORANGE, mono::FontCentering::HORIZONTAL_VERTICAL);
}

void editor::DrawSetTranslationDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    math::Vector delta_position;
    FindAttribute(POSITION_ATTRIBUTE, component_properties, delta_position, FallbackMode::SET_DEFAULT);

    const math::Vector end_position = delta_position;
    renderer.DrawLines({ math::ZeroVec, end_position }, mono::Color::CYAN, 2.0f);

    const std::vector<math::Vector>& arrow_points = GenerateArrow(math::ZeroVec, end_position, 0.5f);
    renderer.DrawLines(arrow_points, mono::Color::CYAN, 2.0f);
}

void editor::DrawSetRotationDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    float delta_rotation;
    FindAttribute(ROTATION_ATTRIBUTE, component_properties, delta_rotation, FallbackMode::SET_DEFAULT);

    const std::vector<math::Vector>& arc_points = GenerateArc(math::ZeroVec, 0.0f, delta_rotation, 20);
    renderer.DrawPolyline(arc_points, mono::Color::CYAN, 2.0f);

    const uint32_t almost_last_point = arc_points.size() -2;
    const uint32_t last_point = arc_points.size() -1;

    const std::vector<math::Vector>& arrow_points = GenerateArrow(arc_points[almost_last_point], arc_points[last_point], 0.5f);
    renderer.DrawLines(arrow_points, mono::Color::CYAN, 2.0f);
}

void editor::DrawSpriteDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    float sort_offset = 0.0f;
    FindAttribute(SORT_OFFSET_ATTRIBUTE, component_properties, sort_offset, FallbackMode::SET_DEFAULT);

    const float half_width = math::Width(entity_bb) / 2.0f;
    const float bottom = math::Bottom(entity_bb);

    const std::vector<math::Vector> line = {
        { -half_width, bottom + sort_offset },
        { +half_width, bottom + sort_offset }
    };
    renderer.DrawLines(line, mono::Color::CYAN, 1.0f);
}

void editor::DrawAreaEmitterDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    math::Vector area_size;
    FindAttribute(SIZE_ATTRIBUTE, component_properties, area_size, FallbackMode::SET_DEFAULT);

    const mono::Color::RGBA color = { 1.0f, 0.0f, 0.5f, 0.25f};
    renderer.DrawFilledQuad(math::Quad(-area_size / 2.0, area_size / 2.0f), color);
}

void editor::DrawPath(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb)
{
    std::vector<math::Vector> vertices;
    const bool found_polygon = FindAttribute(PATH_POINTS_ATTRIBUTE, component_properties, vertices, FallbackMode::REQUIRE_ATTRIBUTE);
    if(!found_polygon)
        return;

    renderer.DrawPolyline(vertices, mono::Color::MAGENTA, 1.0f);
}