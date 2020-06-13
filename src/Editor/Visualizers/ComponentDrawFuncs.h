
#pragma once

#include "Rendering/RenderFwd.h"
#include "Math/MathFwd.h"
#include <vector>

struct Attribute;

namespace editor
{
    void DrawCircleShapeDetails(
        mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties);
    void DrawBoxShapeDetails(
        mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties);
    void DrawSegmentShapeDetails(
        mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties);
    void DrawSpawnPointDetails(
        mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties);
    void DrawTriggerComponentDetails(
        mono::IRenderer& renderer, const math::Vector& position, float rotation, const std::vector<Attribute>& component_properties);
}
