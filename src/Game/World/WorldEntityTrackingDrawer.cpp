
#include "WorldEntityTrackingDrawer.h"
#include "WorldEntityTrackingSystem.h"

#include "Math/Quad.h"
#include "Math/Matrix.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/RenderBuffer/BufferFactory.h"
#include "Rendering/Sprite/SpriteFactory.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;

WorldEntityTrackingDrawer::WorldEntityTrackingDrawer(
    const WorldEntityTrackingSystem* entity_tracking_system, const mono::TransformSystem* transform_system)
    : m_entity_tracking_system(entity_tracking_system)
    , m_transform_system(transform_system)
{
    m_package_sprite = mono::RenderSystem::GetSpriteFactory()->CreateSprite("res/sprites/cardboard_box_small.sprite");
    m_sprite_buffers = mono::BuildSpriteDrawBuffers(m_package_sprite->GetSpriteData());

    constexpr uint16_t indices[] = {
        0, 1, 2, 0, 2, 3
    };
    m_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, 6, indices);
}

void WorldEntityTrackingDrawer::Draw(mono::IRenderer& renderer) const
{
    const math::Quad viewport = math::ResizeQuad(renderer.GetViewport(), -0.25f);

    const math::Vector top_left = math::TopLeft(viewport);
    const math::Vector top_right = math::TopRight(viewport);
    const math::Vector bottom_left = math::BottomLeft(viewport);
    const math::Vector bottom_right = math::BottomRight(viewport);

    const std::vector<uint32_t>& entities_to_track = m_entity_tracking_system->GetTrackedEntities();
    for(uint32_t entity_id : entities_to_track)
    {
        const math::Vector entity_world_position = m_transform_system->GetWorldPosition(entity_id);
        const bool is_in_view = (renderer.Cull(math::Quad(entity_world_position, 0.1f)) == mono::CullResult::IN_VIEW);
        if(is_in_view)
            continue;

        const math::PointOnLineResult results[] = {
            math::ClosestPointOnLine(top_left, top_right, entity_world_position),
            math::ClosestPointOnLine(top_right, bottom_right, entity_world_position),
            math::ClosestPointOnLine(bottom_right, bottom_left, entity_world_position),
            math::ClosestPointOnLine(bottom_left, top_left, entity_world_position),
        };

        float closest_distance = math::INF;
        math::Vector closest_point;

        for(const math::PointOnLineResult& result : results)
        {
            const float distance = math::DistanceBetween(entity_world_position, result.point);
            if(distance < closest_distance)
            {
                closest_distance = distance;
                closest_point = result.point;
            }
        }

        const math::Matrix& transform = math::CreateMatrixWithPosition(closest_point);
        const auto transform_scope = mono::MakeTransformScope(transform, &renderer);

        renderer.DrawFilledCircle(math::ZeroVec, math::Vector(0.275f, 0.275f), 32, mono::Color::BLACK);
        renderer.DrawFilledCircle(math::ZeroVec, math::Vector(0.25f, 0.25f), 32, mono::Color::GRAY);
        renderer.DrawSprite(m_package_sprite.get(), &m_sprite_buffers, m_indices.get(), 0);
    }
}

math::Quad WorldEntityTrackingDrawer::BoundingBox() const
{
    return math::InfQuad;
}
