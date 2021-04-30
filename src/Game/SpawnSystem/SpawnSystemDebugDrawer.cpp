
#include "SpawnSystemDebugDrawer.h"
#include "SpawnSystem.h"
#include "GameDebug.h"
#include "FontIds.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "TransformSystem/TransformSystem.h"
#include "Math/Quad.h"

using namespace game;

SpawnSystemDebugDrawer::SpawnSystemDebugDrawer(class SpawnSystem* spawn_system, mono::TransformSystem* transform_system)
    : m_spawn_system(spawn_system)
    , m_transform_system(transform_system)
{ }

void SpawnSystemDebugDrawer::Draw(mono::IRenderer& renderer) const
{
    if(!game::g_draw_spawn_points)
        return;

    const auto draw_spawn_points = [&renderer, this](uint32_t entity_id, const SpawnSystem::SpawnPoint& spawn_point) {

        const char* active_string = spawn_point.active ? "Active" : "Inactive";

        char text_buffer[128] = { };
        std::snprintf(text_buffer, std::size(text_buffer), "%s|%d/%d", active_string, spawn_point.counter, spawn_point.interval);

        const math::Matrix& world_transform = m_transform_system->GetWorld(entity_id);
        const auto scope = mono::MakeTransformScope(world_transform, &renderer);
        renderer.DrawCircle(math::ZeroVec, spawn_point.radius, 16, 1.0f, mono::Color::RED);
        renderer.RenderText(shared::PIXELETTE_TINY, text_buffer, mono::Color::CYAN, mono::FontCentering::HORIZONTAL);
    };

    m_spawn_system->ForEeach(draw_spawn_points);
}

math::Quad SpawnSystemDebugDrawer::BoundingBox() const
{
    return math::InfQuad;
}
