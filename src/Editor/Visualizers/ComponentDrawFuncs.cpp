
#include "ComponentDrawFuncs.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"
#include "Math/MathFunctions.h"

#include "Component.h"
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

void editor::DrawCircleShapeDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    float radius_value = 1.0f;
    math::Vector offset;
    bool is_sensor;

    FindAttribute(RADIUS_ATTRIBUTE, component_properties, radius_value, FallbackMode::SET_DEFAULT);
    FindAttribute(POSITION_ATTRIBUTE, component_properties, offset, FallbackMode::SET_DEFAULT);
    FindAttribute(SENSOR_ATTRIBUTE, component_properties, is_sensor, FallbackMode::SET_DEFAULT);

    if(is_sensor)
    {
        mono::Color::RGBA sensor_color = mono::Color::BLUE;
        sensor_color.alpha = 0.25f;
        renderer.DrawFilledCircle(position + offset, math::Vector(radius_value, radius_value), 20, sensor_color);
    }

    renderer.DrawCircle(position + offset, std::max(radius_value, 0.0001f), 20, 1.0f, mono::Color::MAGENTA);
}

void editor::DrawBoxShapeDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    math::Vector width_height;
    math::Vector offset;
    bool is_sensor;

    FindAttribute(SIZE_ATTRIBUTE, component_properties, width_height, FallbackMode::SET_DEFAULT);
    FindAttribute(POSITION_ATTRIBUTE, component_properties, offset, FallbackMode::SET_DEFAULT);
    FindAttribute(SENSOR_ATTRIBUTE, component_properties, is_sensor, FallbackMode::SET_DEFAULT);

    const math::Vector half_size = width_height / 2.0f;

    math::Quad box;
    box.mA = position + offset - half_size;
    box.mB = position + offset + half_size;

    if(is_sensor)
    {
        mono::Color::RGBA sensor_color = mono::Color::BLUE;
        sensor_color.alpha = 0.25f;
        renderer.DrawFilledQuad(box, sensor_color);
    }
    renderer.DrawQuad(box, mono::Color::MAGENTA, 1.0f);
}

void editor::DrawSegmentShapeDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    math::Vector start;
    math::Vector end;
    float radius = 1.0f;
    bool is_sensor;

    FindAttribute(START_ATTRIBUTE, component_properties, start, FallbackMode::SET_DEFAULT);
    FindAttribute(END_ATTRIBUTE, component_properties, end, FallbackMode::SET_DEFAULT);
    FindAttribute(RADIUS_ATTRIBUTE, component_properties, radius, FallbackMode::SET_DEFAULT);
    FindAttribute(SENSOR_ATTRIBUTE, component_properties, is_sensor, FallbackMode::SET_DEFAULT);

    const std::vector<math::Vector> line = {
        start + position,
        end + position
    };

    if(is_sensor)
    {
        mono::Color::RGBA sensor_color = mono::Color::BLUE;
        sensor_color.alpha = 0.25f;
        renderer.DrawLines(line, sensor_color, 10.0f);
    }
    renderer.DrawLines(line, mono::Color::MAGENTA, std::max(radius, 1.0f));
}

void editor::DrawPolygonShapeDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    std::vector<math::Vector> polygon;
    const bool found_polygon =
        FindAttribute(POLYGON_ATTRIBUTE, component_properties, polygon, FallbackMode::REQUIRE_ATTRIBUTE);
    if(!found_polygon)
        return;

    bool is_sensor;
    FindAttribute(SENSOR_ATTRIBUTE, component_properties, is_sensor, FallbackMode::SET_DEFAULT);

    const auto local_to_world = [position, rotation](const math::Vector& point) {
        math::Matrix local_to_world = math::CreateMatrixFromZRotation(rotation);
        math::Position(local_to_world, position);
        return math::Transform(local_to_world, point);
    };
    std::transform(polygon.begin(), polygon.end(), polygon.begin(), local_to_world);

    renderer.DrawClosedPolyline(polygon, mono::Color::MAGENTA, 1.0f);
    if(is_sensor)
        renderer.DrawText(shared::PIXELETTE_SMALL, "sensor", position, true, mono::Color::BLUE);
}

void editor::DrawSpawnPointDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    const math::Vector unit_vector = math::VectorFromAngle(rotation);

    renderer.DrawCircle(position, 1.0f, 8, 5.0f, mono::Color::BLUE);
    renderer.DrawLines(
        { position, position + unit_vector * 1.5f }, mono::Color::BLUE, 3.0f);
}

void editor::DrawShapeTriggerComponentDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    std::string name;
    FindAttribute(TRIGGER_NAME_ATTRIBUTE, component_properties, name, FallbackMode::SET_DEFAULT);
    renderer.DrawText(shared::FontId::PIXELETTE_TINY, name.c_str(), position, true, mono::Color::BLUE);
}

void editor::DrawAreaTriggerComponentDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    math::Vector width_height;
    FindAttribute(SIZE_ATTRIBUTE, component_properties, width_height, FallbackMode::SET_DEFAULT);

    const math::Vector half_width_height = width_height / 2.0f;
    renderer.DrawFilledQuad(math::Quad(position - half_width_height, position + half_width_height), mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.5f));

    std::string name;
    FindAttribute(TRIGGER_NAME_ATTRIBUTE, component_properties, name, FallbackMode::SET_DEFAULT);
    renderer.DrawText(shared::FontId::PIXELETTE_TINY, name.c_str(), position, true, mono::Color::BLUE);
}

void editor::DrawDeathTriggerComponentDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    std::string name;
    FindAttribute(TRIGGER_NAME_ATTRIBUTE, component_properties, name, FallbackMode::SET_DEFAULT);
    renderer.DrawText(shared::FontId::PIXELETTE_TINY, name.c_str(), position, true, mono::Color::BLUE);
}

void editor::DrawTimeTriggerComponentDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    std::string name;
    FindAttribute(TRIGGER_NAME_ATTRIBUTE, component_properties, name, FallbackMode::SET_DEFAULT);
    renderer.DrawText(shared::FontId::PIXELETTE_TINY, name.c_str(), position, true, mono::Color::BLUE);
}

void editor::DrawSetTranslationDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    math::Vector delta_position;
    FindAttribute(POSITION_ATTRIBUTE, component_properties, delta_position, FallbackMode::SET_DEFAULT);

    const math::Vector end_position = position + delta_position;
    renderer.DrawLines({ position, end_position }, mono::Color::CYAN, 2.0f);

    const std::vector<math::Vector>& arrow_points = GenerateArrow(position, end_position, 0.5f);
    renderer.DrawLines(arrow_points, mono::Color::CYAN, 2.0f);
}

void editor::DrawSetRotationDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    float delta_rotation;
    FindAttribute(ROTATION_ATTRIBUTE, component_properties, delta_rotation, FallbackMode::SET_DEFAULT);

    const std::vector<math::Vector>& arc_points = GenerateArc(position, rotation, rotation + delta_rotation, 20);
    renderer.DrawPolyline(arc_points, mono::Color::CYAN, 2.0f);

    const uint32_t almost_last_point = arc_points.size() -2;
    const uint32_t last_point = arc_points.size() -1;

    const std::vector<math::Vector>& arrow_points = GenerateArrow(arc_points[almost_last_point], arc_points[last_point], 0.5f);
    renderer.DrawLines(arrow_points, mono::Color::CYAN, 2.0f);
}
