
#include "PlayerAuxiliaryDrawer.h"
#include "Math/Quad.h"
#include "Math/MathFunctions.h"
#include "Player/PlayerInfo.h"
#include "Player/PlayerAbilities.h"

#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "Rendering/RenderBuffer/BufferFactory.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/RenderSystem.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;

namespace
{
    AimlineRenderData GenerateAimLine(
        float start_offset, float mid_point_offset, float length, float width, float cutoff_length)
    {
        const float min_mid_point_offset = std::min(mid_point_offset, cutoff_length);
        const float min_length = std::min(length, cutoff_length);

        const math::Vector aim_lines_vertices[] = {
            math::Vector(start_offset, 0.0f),
            math::Vector(start_offset, width),
            math::Vector(min_mid_point_offset, 0.0f),
            math::Vector(min_mid_point_offset, width),
            math::Vector(min_length, 0.0f),
            math::Vector(min_length, width),
        };

        const mono::Color::RGBA aim_lines_colors[] = {
            mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.0f),
            mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.0f),
            mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.6f),
            mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.6f),
            mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.0f),
            mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.0f),
        };

        uint16_t aim_lines_indices[] = {
            0, 1, 2,
            1, 3, 2,
            2, 3, 4,
            3, 5, 4,
        };

        AimlineRenderData render_data;
        render_data.vertices = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 2, std::size(aim_lines_vertices), aim_lines_vertices);
        render_data.colors = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 4, std::size(aim_lines_colors), aim_lines_colors);
        render_data.indices = mono::CreateElementBuffer(mono::BufferType::STATIC, std::size(aim_lines_indices), aim_lines_indices);

        return render_data;
    }

    constexpr float start_offset = 0.25f;
    constexpr float mid_point_offset = 1.0f;
    constexpr float length = 7.0f;
    constexpr float width = 0.025f;
}

PlayerAuxiliaryDrawer::PlayerAuxiliaryDrawer(const mono::TransformSystem* transform_system)
    : m_transform_system(transform_system)
{ }

void PlayerAuxiliaryDrawer::Draw(mono::IRenderer& renderer) const
{
    std::vector<math::Vector> cooldown_lines;
    std::vector<math::Vector> cooldown_points;

    struct AimlineData
    {
        math::Matrix transform;
        uint32_t render_data_index;
    };
    std::vector<AimlineData> aimline_transforms;
    std::vector<math::Vector> aim_target_points;

    mono::Color::RGBA cooldown_color;

    for(const game::PlayerInfo* player_info : GetActivePlayers())
    {
        if(!player_info)
            continue;

        if(player_info->laser_sight)
        {
            const float aim_direction_rad = math::AngleFromVector(player_info->aim_direction);
            const math::Matrix& aimline_transform =
                math::CreateMatrixWithPositionRotation(player_info->position, aim_direction_rad + math::PI_2());

            const uint32_t player_index = FindPlayerIndex(player_info);
            const float laser_length = math::DistanceBetween(player_info->position, player_info->aim_target);

            m_aimline_data[player_index] = GenerateAimLine(start_offset, mid_point_offset, length, width, laser_length);
            aimline_transforms.push_back({ aimline_transform, player_index });
            aim_target_points.push_back(player_info->aim_target);
        }

        const bool ability_on_cooldown = (player_info->cooldown_fraction < 1.0f);
        if(ability_on_cooldown)
        {
            const math::Quad world_bb = m_transform_system->GetWorldBoundingBox(player_info->entity_id);
            const math::Vector bottom_center = math::BottomCenter(world_bb);

            const math::Vector left = bottom_center - math::Vector(0.3f, 0.15f);
            const math::Vector right = bottom_center + math::Vector(0.3f, -0.15f);
            const math::Vector reload_dot = ((right - left) * player_info->cooldown_fraction) + left;

            cooldown_lines.push_back(left);
            cooldown_lines.push_back(right);
            cooldown_points.push_back(reload_dot);

            cooldown_color = g_ability_to_color[player_info->cooldown_id];
        }
    }

    for(const AimlineData& aimline_data : aimline_transforms)
    {
        const AimlineRenderData& render_data = m_aimline_data[aimline_data.render_data_index];

        const auto transform_scope = mono::MakeTransformScope(aimline_data.transform, &renderer);
        renderer.DrawTrianges(
            render_data.vertices.get(),
            render_data.colors.get(),
            render_data.indices.get(),
            0,
            render_data.indices->Size());
    }

    constexpr mono::Color::RGBA laser_red = mono::Color::RGBA(1.0f, 0.0f, 0.0f, 0.7f);
    renderer.DrawPoints(aim_target_points, laser_red, 12.0f);

    renderer.DrawLines(cooldown_lines, mono::Color::OFF_WHITE, 4.0f);
    renderer.DrawPoints(cooldown_points, cooldown_color, 8.0f);
}

math::Quad PlayerAuxiliaryDrawer::BoundingBox() const
{
    return math::InfQuad;
}


PackageAuxiliaryDrawer::PackageAuxiliaryDrawer(const mono::TransformSystem* transform_system)
    : m_transform_system(transform_system)
    , m_package_id(-1)
{
    m_package_sprite = mono::GetSpriteFactory()->CreateSprite("res/sprites/cardboard_box_small.sprite");
    m_sprite_buffers = mono::BuildSpriteDrawBuffers(m_package_sprite->GetSpriteData());

    constexpr uint16_t indices[] = {
        0, 1, 2, 0, 2, 3
    };
    m_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, 6, indices);
}

void PackageAuxiliaryDrawer::Draw(mono::IRenderer& renderer) const
{
    if(m_package_id == uint32_t(-1))
        return;

    const math::Vector package_world_position = m_transform_system->GetWorldPosition(m_package_id);
    const bool is_in_view = renderer.Cull(math::Quad(package_world_position, 0.1f));
    if(!is_in_view)
    {
        const math::Quad viewport = math::ResizeQuad(renderer.GetViewport(), -0.25f);

        const math::Vector top_left = math::TopLeft(viewport);
        const math::Vector top_right = math::TopRight(viewport);
        const math::Vector bottom_left = math::BottomLeft(viewport);
        const math::Vector bottom_right = math::BottomRight(viewport);

        const math::PointOnLineResult results[] = {
            math::ClosestPointOnLine(top_left, top_right, package_world_position),
            math::ClosestPointOnLine(top_right, bottom_right, package_world_position),
            math::ClosestPointOnLine(bottom_right, bottom_left, package_world_position),
            math::ClosestPointOnLine(bottom_left, top_left, package_world_position),
        };

        float closest_distance = math::INF;
        math::Vector closest_point;

        for(const math::PointOnLineResult& result : results)
        {
            const float distance = math::DistanceBetween(package_world_position, result.point);
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
        renderer.DrawSprite(
            m_package_sprite.get(),
            m_sprite_buffers.vertices.get(),
            m_sprite_buffers.offsets.get(),
            m_sprite_buffers.uv.get(),
            m_sprite_buffers.uv_flipped.get(),
            m_sprite_buffers.heights.get(),
            m_indices.get(),
            m_package_sprite->GetTexture(),
            0);
    }
}

math::Quad PackageAuxiliaryDrawer::BoundingBox() const
{
    return math::InfQuad;
}

void PackageAuxiliaryDrawer::SetPackageId(uint32_t package_id)
{
    m_package_id = package_id;
}

