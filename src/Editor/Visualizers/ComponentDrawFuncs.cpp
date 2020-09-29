
#include "ComponentDrawFuncs.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Quad.h"
#include "Math/MathFunctions.h"

#include "Component.h"
#include "FontIds.h"


void editor::DrawCircleShapeDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    float radius_value = 1.0f;
    math::Vector offset;

    FindAttribute(RADIUS_ATTRIBUTE, component_properties, radius_value, FallbackMode::SET_DEFAULT);
    FindAttribute(POSITION_ATTRIBUTE, component_properties, offset, FallbackMode::SET_DEFAULT);

    constexpr mono::Color::RGBA color(1.0f, 0.0f, 1.0f);
    renderer.DrawCircle(position + offset, std::max(radius_value, 0.0001f), 20, 1.0f, color);
}

void editor::DrawBoxShapeDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    math::Vector width_height;
    math::Vector offset;

    FindAttribute(SIZE_ATTRIBUTE, component_properties, width_height, FallbackMode::SET_DEFAULT);
    FindAttribute(POSITION_ATTRIBUTE, component_properties, offset, FallbackMode::SET_DEFAULT);

    const math::Vector half_size = width_height / 2.0f;

    math::Quad box;
    box.mA = position + offset - half_size;
    box.mB = position + offset + half_size;

    constexpr mono::Color::RGBA color(1.0f, 0.0f, 1.0f);
    renderer.DrawQuad(box, color, 1.0f);
}

void editor::DrawSegmentShapeDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    math::Vector start;
    math::Vector end;
    float radius = 1.0f;

    FindAttribute(START_ATTRIBUTE, component_properties, start, FallbackMode::SET_DEFAULT);
    FindAttribute(END_ATTRIBUTE, component_properties, end, FallbackMode::SET_DEFAULT);
    FindAttribute(RADIUS_ATTRIBUTE, component_properties, radius, FallbackMode::SET_DEFAULT);

    const std::vector<math::Vector> line = {
        start + position,
        end + position
    };

    constexpr mono::Color::RGBA color(1.0f, 0.0f, 1.0f);
    renderer.DrawLines(line, color, std::max(radius, 1.0f));
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
    renderer.DrawText(shared::FontId::PIXELETTE_SMALL, name.c_str(), position, true, mono::Color::BLUE);
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
    renderer.DrawText(shared::FontId::PIXELETTE_SMALL, name.c_str(), position, true, mono::Color::BLUE);
}

void editor::DrawDeathTriggerComponentDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    std::string name;
    FindAttribute(TRIGGER_NAME_ATTRIBUTE, component_properties, name, FallbackMode::SET_DEFAULT);
    renderer.DrawText(shared::FontId::PIXELETTE_SMALL, name.c_str(), position, true, mono::Color::BLUE);
}

void editor::DrawTimeTriggerComponentDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    std::string name;
    FindAttribute(TRIGGER_NAME_ATTRIBUTE, component_properties, name, FallbackMode::SET_DEFAULT);
    renderer.DrawText(shared::FontId::PIXELETTE_SMALL, name.c_str(), position, true, mono::Color::BLUE);
}

void editor::DrawSetTranslationDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    math::Vector delta_position;
    FindAttribute(POSITION_ATTRIBUTE, component_properties, delta_position, FallbackMode::SET_DEFAULT);

    renderer.DrawLines({ position, position + delta_position }, mono::Color::BLUE, 3.0f);
}

void editor::DrawSetRotationDetails(
    mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties)
{
    float delta_rotation;
    FindAttribute(ROTATION_ATTRIBUTE, component_properties, delta_rotation, FallbackMode::SET_DEFAULT);

    const math::Vector start_vector = math::VectorFromAngle(rotation);
    const math::Vector end_vector = math::VectorFromAngle(rotation + delta_rotation);

    renderer.DrawClosedPolyline({ position, position + start_vector, position + end_vector }, mono::Color::CYAN, 2.0f);
}
