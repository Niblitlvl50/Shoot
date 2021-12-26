
#include "PlayerAuxiliaryDrawer.h"
#include "Math/Quad.h"
#include "Player/PlayerInfo.h"

#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "Rendering/RenderBuffer/BufferFactory.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;

namespace
{
    struct OffsetCount
    {
        uint32_t offset;
        uint32_t count;
    };

    OffsetCount GenerateAimLine(
        std::vector<math::Vector>& aim_lines_vertices, std::vector<mono::Color::RGBA>& aim_lines_colors, std::vector<uint16_t>& aim_lines_indices)
    {
        constexpr float offset = 0.25f;
        constexpr float mid_point_offset = 1.0f;
        constexpr float length = 5.0f;
        constexpr float width = 0.025f;

        const uint32_t index_offset = aim_lines_vertices.size();

        aim_lines_vertices.emplace_back(offset, 0.0f);
        aim_lines_vertices.emplace_back(offset, width);
        aim_lines_vertices.emplace_back(mid_point_offset, 0.0f);
        aim_lines_vertices.emplace_back(mid_point_offset, width);
        aim_lines_vertices.emplace_back(length, 0.0f);
        aim_lines_vertices.emplace_back(length, width);

        aim_lines_colors.emplace_back(1.0f, 0.0f, 0.0f, 0.0f);
        aim_lines_colors.emplace_back(1.0f, 0.0f, 0.0f, 0.0f);
        aim_lines_colors.emplace_back(1.0f, 0.0f, 0.0f, 0.6f);
        aim_lines_colors.emplace_back(1.0f, 0.0f, 0.0f, 0.6f);
        aim_lines_colors.emplace_back(1.0f, 0.0f, 0.0f, 0.0f);
        aim_lines_colors.emplace_back(1.0f, 0.0f, 0.0f, 0.0f);


        aim_lines_indices.push_back(index_offset + 0);
        aim_lines_indices.push_back(index_offset + 1);
        aim_lines_indices.push_back(index_offset + 2);

        aim_lines_indices.push_back(index_offset + 1);
        aim_lines_indices.push_back(index_offset + 3);
        aim_lines_indices.push_back(index_offset + 2);

        aim_lines_indices.push_back(index_offset + 2);
        aim_lines_indices.push_back(index_offset + 3);
        aim_lines_indices.push_back(index_offset + 4);

        aim_lines_indices.push_back(index_offset + 3);
        aim_lines_indices.push_back(index_offset + 5);
        aim_lines_indices.push_back(index_offset + 4);

        return { index_offset, 12 };
    }
}

PlayerAuxiliaryDrawer::PlayerAuxiliaryDrawer(const mono::TransformSystem* transform_system)
    : m_transform_system(transform_system)
{ }

void PlayerAuxiliaryDrawer::Draw(mono::IRenderer& renderer) const
{
    std::vector<math::Vector> aim_lines_vertices;
    std::vector<mono::Color::RGBA> aim_lines_colors;
    std::vector<uint16_t> aim_lines_indices;

    std::vector<math::Vector> reload_lines;
    std::vector<math::Vector> points;

    struct AimLineTransform
    {
        math::Matrix transform;
        uint32_t offset;
        uint32_t count;
    };

    std::vector<AimLineTransform> aimline_commands;

    for(const game::PlayerInfo* player_info : GetActivePlayers())
    {
        if(!player_info)
            continue;

        const OffsetCount& offset_count = GenerateAimLine(aim_lines_vertices, aim_lines_colors, aim_lines_indices);
        aimline_commands.push_back({ m_transform_system->GetWorld(player_info->entity_id), offset_count.offset, offset_count.count });

        const bool is_reloading = (player_info->weapon_state == game::WeaponState::RELOADING);
        if(is_reloading)
        {
            const math::Quad world_bb = m_transform_system->GetWorldBoundingBox(player_info->entity_id);
            const math::Vector bottom_center = math::BottomCenter(world_bb);

            const math::Vector left = bottom_center - math::Vector(0.3f, 0.15f);
            const math::Vector right = bottom_center + math::Vector(0.3f, -0.15f);
            const math::Vector reload_dot = ((right - left) * float(player_info->weapon_reload_percentage) / 100.0f) + left;

            reload_lines.push_back(left);
            reload_lines.push_back(right);
            points.push_back(reload_dot);
        }
    }

    if(!aimline_commands.empty())
    {
        auto vertices = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 2, aim_lines_vertices.size(), aim_lines_vertices.data());
        auto color_buffer = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 4, aim_lines_colors.size(), aim_lines_colors.data());
        auto index_buffer = mono::CreateElementBuffer(mono::BufferType::STATIC, aim_lines_indices.size(), aim_lines_indices.data());

        for(const AimLineTransform& cmd : aimline_commands)
        {
            const auto transform_scope = mono::MakeTransformScope(cmd.transform, &renderer);
            renderer.DrawTrianges(vertices.get(), color_buffer.get(), index_buffer.get(), cmd.offset, cmd.count);
        }
    }

    renderer.DrawLines(reload_lines, mono::Color::OFF_WHITE, 4.0f);
    renderer.DrawPoints(points, mono::Color::BLACK, 8.0f);
}

math::Quad PlayerAuxiliaryDrawer::BoundingBox() const
{
    return math::InfQuad;
}
