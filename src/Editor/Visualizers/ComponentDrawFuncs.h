
#pragma once

#include "Math/MathFwd.h"
#include "Rendering/RenderFwd.h"
#include <vector>

struct Attribute;

namespace editor
{
    void DrawCircleShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);
    void DrawBoxShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);
    void DrawSegmentShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);
    void DrawPolygonShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);
    void DrawSpawnPointDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);
    void DrawEntitySpawnPointDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);
    void DrawShapeTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);
    void DrawAreaTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);
    void DrawDestroyedTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);
    void DrawTimeTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);
    void DrawCounterTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);
    void DrawSetTranslationDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);
    void DrawSetRotationDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);
    void DrawSpriteDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);
    void DrawAreaEmitterDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);

    void DrawPath(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);
    void DrawCameraPoint(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb);
}
