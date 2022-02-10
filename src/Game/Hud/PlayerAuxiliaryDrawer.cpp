
#include "PlayerAuxiliaryDrawer.h"
#include "Math/Quad.h"
#include "Player/PlayerInfo.h"
#include "Player/PlayerAbilities.h"

#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "Rendering/RenderBuffer/BufferFactory.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;

namespace
{
    void GenerateAimLine(
        std::vector<math::Vector>& aim_lines_vertices, std::vector<mono::Color::RGBA>& aim_lines_colors, std::vector<uint16_t>& aim_lines_indices)
    {
        constexpr float offset = 0.25f;
        constexpr float mid_point_offset = 1.0f;
        constexpr float length = 7.0f;
        constexpr float width = 0.025f;

        aim_lines_vertices = {
            math::Vector(offset, 0.0f),
            math::Vector(offset, width),
            math::Vector(mid_point_offset, 0.0f),
            math::Vector(mid_point_offset, width),
            math::Vector(length, 0.0f),
            math::Vector(length, width),
        };

        aim_lines_colors = {
            mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.0f),
            mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.0f),
            mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.6f),
            mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.6f),
            mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.0f),
            mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.0f),
        };

        aim_lines_indices = {
            0, 1, 2,
            1, 3, 2,
            2, 3, 4,
            3, 5, 4,
        };
    }
}

PlayerAuxiliaryDrawer::PlayerAuxiliaryDrawer(const mono::TransformSystem* transform_system)
    : m_transform_system(transform_system)
{
    std::vector<math::Vector> aim_lines_vertices;
    std::vector<mono::Color::RGBA> aim_lines_colors;
    std::vector<uint16_t> aim_lines_indices;
    GenerateAimLine(aim_lines_vertices, aim_lines_colors, aim_lines_indices);

    m_laser_vertices = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 2, aim_lines_vertices.size(), aim_lines_vertices.data());
    m_laser_colors = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 4, aim_lines_colors.size(), aim_lines_colors.data());
    m_laser_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, aim_lines_indices.size(), aim_lines_indices.data());
}

void PlayerAuxiliaryDrawer::Draw(mono::IRenderer& renderer) const
{
    std::vector<math::Vector> reload_lines;
    std::vector<math::Vector> points;
    std::vector<math::Matrix> aimline_transforms;

    mono::Color::RGBA cooldown_color;

    for(const game::PlayerInfo* player_info : GetActivePlayers())
    {
        if(!player_info)
            continue;

        if(player_info->laser_sight)
        {
            const math::Matrix& aimline_transform = math::CreateMatrixWithPositionRotation(player_info->position, player_info->aim_direction + math::PI_2());
            aimline_transforms.push_back(aimline_transform);
        }

        const bool ability_on_cooldown = (player_info->cooldown_fraction < 1.0f);
        if(ability_on_cooldown)
        {
            const math::Quad world_bb = m_transform_system->GetWorldBoundingBox(player_info->entity_id);
            const math::Vector bottom_center = math::BottomCenter(world_bb);

            const math::Vector left = bottom_center - math::Vector(0.3f, 0.15f);
            const math::Vector right = bottom_center + math::Vector(0.3f, -0.15f);
            const math::Vector reload_dot = ((right - left) * player_info->cooldown_fraction) + left;

            reload_lines.push_back(left);
            reload_lines.push_back(right);
            points.push_back(reload_dot);

            cooldown_color = g_ability_to_color[player_info->cooldown_id];
        }
    }

    for(const math::Matrix& transform : aimline_transforms)
    {
        const auto transform_scope = mono::MakeTransformScope(transform, &renderer);
        renderer.DrawTrianges(m_laser_vertices.get(), m_laser_colors.get(), m_laser_indices.get(), 0, m_laser_indices->Size());
    }

    renderer.DrawLines(reload_lines, mono::Color::OFF_WHITE, 4.0f);
    renderer.DrawPoints(points, cooldown_color, 8.0f);
}

math::Quad PlayerAuxiliaryDrawer::BoundingBox() const
{
    return math::InfQuad;
}
