
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Rendering/RenderFwd.h"
#include <cstdint>
#include <vector>

struct Attribute;

namespace editor
{
    void DrawCircleShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawBoxShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawSegmentShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawPolygonShapeDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawSpawnPointDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawEntitySpawnPointDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);

    void DrawShapeTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawAreaTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawDestroyedTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawTimeTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawCounterTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawRelayTriggerComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);

    void DrawSetTranslationDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawSetRotationDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawLayerDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawAreaEmitterDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);

    void DrawPath(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawCameraPoint(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawTeleportPlayerPoint(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);

    void DrawShockwaveComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawPhysicsImpulseComponentDetails(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);

    void DrawRailwaySwitch(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);
    void DrawRailwayStation(mono::IRenderer& renderer, const std::vector<Attribute>& component_properties, const math::Quad& entity_bb, uint32_t entity_id);

    // Must be called once before any DrawPath call, so it can look up notifiers for the
    // path entity currently being drawn.
    void SetPathSystemForDebugDraw(const mono::PathSystem* path_system);
}
