
#pragma once

#include "Rendering/RenderFwd.h"
#include <vector>

struct Attribute;

namespace editor
{
    void DrawCircleShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties);
    void DrawBoxShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties);
    void DrawSegmentShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties);
    void DrawPolygonShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties);
    void DrawSpawnPointDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties);
    void DrawShapeTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties);
    void DrawAreaTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties);
    void DrawDeathTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties);
    void DrawTimeTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties);
    void DrawCounterTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties);
    void DrawSetTranslationDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties);
    void DrawSetRotationDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties);
}
