
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
#include "System/File.h"
#include "System/System.h"

#include "nlohmann/json.hpp"

#include <cstring>
#include <string>

using namespace game;

WorldEntityTrackingDrawer::WorldEntityTrackingDrawer(
    const WorldEntityTrackingSystem* entity_tracking_system, const mono::TransformSystem* transform_system)
    : m_entity_tracking_system(entity_tracking_system)
    , m_transform_system(transform_system)
{
    file::FilePtr config_file = file::OpenAsciiFile("res/configs/entity_tracking_config.json");
    if(config_file)
    {
        const std::vector<byte>& file_data = file::FileRead(config_file);
        const nlohmann::json& json = nlohmann::json::parse(file_data);

        for(const auto& [type_name, sprite_file] : json["tracking_sprites"].items())
        {
            uint32_t type_index = 0;
            for(; type_index < N_ENTITY_TYPES; ++type_index)
            {
                if(std::strcmp(g_entity_type_strings[type_index], type_name.c_str()) == 0)
                    break;
            }

            if(type_index == N_ENTITY_TYPES)
            {
                System::Log("WorldEntityTrackingDrawer|Unknown entity type '%s' in config.", type_name.c_str());
                continue;
            }

            const std::string sprite_file_string = sprite_file;
            m_type_sprites[type_index] = mono::RenderSystem::GetSpriteFactory()->CreateSprite(sprite_file_string.c_str());
            m_type_sprite_buffers[type_index] =
                mono::BuildSpriteDrawBuffers(m_type_sprites[type_index]->GetSpriteData(), "sprite_buffer-world_entity_tracking");
        }
    }

    constexpr uint16_t indices[] = {
        0, 1, 2, 0, 2, 3
    };
    m_sprite_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, 6, indices, "world_entity_tracking_drawer");

    m_circle_draw_buffers = mono::BuildCircleDrawBuffers(math::Vector(0.25f, 0.25f), 32, mono::Color::BLACK);
    m_circle_outline_draw_buffers = mono::BuildCircleDrawBuffers(math::Vector(0.225f, 0.225f), 32, mono::Color::GRAY);
}

void WorldEntityTrackingDrawer::Draw(mono::IRenderer& renderer) const
{
    const math::Quad viewport = math::ResizeQuad(renderer.GetViewport(), -0.25f);

    const math::Vector top_left = math::TopLeft(viewport);
    const math::Vector top_right = math::TopRight(viewport);
    const math::Vector bottom_left = math::BottomLeft(viewport);
    const math::Vector bottom_right = math::BottomRight(viewport);

    const std::vector<EntityTrackingComponent>& entities_to_track = m_entity_tracking_system->GetTrackedEntities();
    for(const EntityTrackingComponent& tracking_entity : entities_to_track)
    {
        const bool is_active_type = m_entity_tracking_system->IsActiveType(tracking_entity.type);
        if(!tracking_entity.enabled || !is_active_type)
            continue;

        const math::Vector entity_world_position = m_transform_system->GetWorldPosition(tracking_entity.entity_id);
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

        renderer.DrawTrianges(
            m_circle_draw_buffers.vertices.get(),
            m_circle_draw_buffers.colors.get(),
            m_circle_draw_buffers.indices.get(),
            0,
            m_circle_draw_buffers.indices->Size());
        
        renderer.DrawTrianges(
            m_circle_outline_draw_buffers.vertices.get(),
            m_circle_outline_draw_buffers.colors.get(),
            m_circle_outline_draw_buffers.indices.get(),
            0,
            m_circle_outline_draw_buffers.indices->Size());

        const uint32_t type_index = static_cast<uint32_t>(tracking_entity.type);
        if(type_index < N_ENTITY_TYPES && m_type_sprites[type_index])
            renderer.DrawSprite(m_type_sprites[type_index].get(), &m_type_sprite_buffers[type_index], m_sprite_indices.get(), 0);
    }
}

math::Quad WorldEntityTrackingDrawer::BoundingBox() const
{
    return math::InfQuad;
}
