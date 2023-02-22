
#include "PlayerAuxiliaryDrawer.h"
#include "Math/Quad.h"
#include "Math/MathFunctions.h"
#include "Math/CriticalDampedSpring.h"
#include "Player/PlayerInfo.h"
#include "Player/PlayerAbilities.h"

#include "EntitySystem/Entity.h"
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
        float start_offset, float mid_point_offset, float length, float width, float cutoff_length, const mono::Color::RGBA& color)
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
            mono::Color::MakeWithAlpha(color, 0.0f),
            mono::Color::MakeWithAlpha(color, 0.0f),
            mono::Color::MakeWithAlpha(color, 0.6f),
            mono::Color::MakeWithAlpha(color, 0.6f),
            mono::Color::MakeWithAlpha(color, 0.0f),
            mono::Color::MakeWithAlpha(color, 0.0f),
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
{
    for(const char* sprite_file : g_ability_to_sprite)
    {
        AbilityRenderData render_data;
        render_data.sprite = mono::RenderSystem::GetSpriteFactory()->CreateSprite(sprite_file);
        render_data.sprite_buffers = mono::BuildSpriteDrawBuffers(render_data.sprite->GetSpriteData());
        m_ability_render_datas.push_back(std::move(render_data));
    }

    constexpr uint16_t indices[] = {
        0, 1, 2, 0, 2, 3
    };
    m_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, 6, indices);

    std::memset(m_ability_data, 0, sizeof(m_ability_data));
}

void PlayerAuxiliaryDrawer::Draw(mono::IRenderer& renderer) const
{
    std::vector<math::Vector> cooldown_lines;

    struct AbilityPoint
    {
        math::Vector point;
        uint32_t render_data_index;
    };
    std::vector<AbilityPoint> ability_points;

    struct AimlineData
    {
        math::Matrix transform;
        uint32_t render_data_index;
    };
    std::vector<AimlineData> aimline_transforms;
    std::vector<math::Vector> aim_target_points;
    std::vector<mono::Color::RGBA> aim_target_colors;

    constexpr mono::Color::RGBA aim_line_colors[3] = {
        mono::Color::RED,
        mono::Color::GREEN,
        mono::Color::BLUE
    };

    for(const game::PlayerInfo* player_info : GetActivePlayers())
    {
        if(!player_info)
            continue;

        const uint32_t player_index = FindPlayerIndex(player_info);
        
        if(player_info->laser_sight)
        {
            const float aim_direction_rad = math::AngleFromVector(player_info->aim_direction);
            const math::Matrix& aimline_transform =
                math::CreateMatrixWithPositionRotation(player_info->position, aim_direction_rad + math::PI_2());

            const float laser_length = math::DistanceBetween(player_info->position, player_info->aim_target);
            const mono::Color::RGBA& laser_color = aim_line_colors[player_index];

            m_aimline_data[player_index] =
                GenerateAimLine(start_offset, mid_point_offset, length, width, laser_length, laser_color);
            aimline_transforms.push_back({ aimline_transform, player_index });
            aim_target_points.push_back(player_info->aim_target);
            aim_target_colors.push_back(laser_color);
        }

        const uint32_t current_timestamp = renderer.GetTimestamp();
        const bool ability_on_cooldown = (player_info->cooldown_fraction < 1.0f);

        AbilityInstanceData& instance_data = m_ability_data[player_index];
        const bool has_changed = (instance_data.last_cooldown_fraction != player_info->cooldown_fraction);
        const bool less_than_5s = (current_timestamp - instance_data.timestamp < 2000);

        if((ability_on_cooldown && has_changed) || (ability_on_cooldown && less_than_5s))
        {
            instance_data.last_cooldown_fraction = player_info->cooldown_fraction;
            if(has_changed)
                instance_data.timestamp = current_timestamp;

            const math::Quad world_bb = m_transform_system->GetWorldBoundingBox(player_info->entity_id);
            const math::Vector bottom_center = math::BottomCenter(world_bb);

            const math::Vector left = bottom_center - math::Vector(0.3f, 0.15f);
            const math::Vector right = bottom_center + math::Vector(0.3f, -0.15f);
            const math::Vector reload_dot = ((right - left) * player_info->cooldown_fraction) + left;

            math::simple_spring_damper_implicit(
                instance_data.cooldown_position, instance_data.cooldown_velocity, reload_dot.x, 0.01f, renderer.GetDeltaTime());

            cooldown_lines.push_back(left);
            cooldown_lines.push_back(right);

            AbilityPoint ability_point;
            ability_point.point = math::Vector(std::clamp(instance_data.cooldown_position, left.x, right.x), reload_dot.y);
            ability_point.render_data_index = player_info->cooldown_id;
            ability_points.push_back(ability_point);
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

    renderer.DrawPoints(aim_target_points, aim_target_colors, 12.0f);
    renderer.DrawLines(cooldown_lines, mono::Color::RGBA(0.8f, 0.8f, 0.8f), 4.0f);

    for(const AbilityPoint& ability_point : ability_points)
    {
        const math::Matrix& transform = math::CreateMatrixWithPositionScale(ability_point.point, 0.35f);
        const auto transform_scope = mono::MakeTransformScope(transform, &renderer);

        const AbilityRenderData& render_data = m_ability_render_datas[ability_point.render_data_index];
        renderer.DrawSprite(render_data.sprite.get(), &render_data.sprite_buffers, m_indices.get(), 0);
    }
}

math::Quad PlayerAuxiliaryDrawer::BoundingBox() const
{
    return math::InfQuad;
}


PackageAuxiliaryDrawer::PackageAuxiliaryDrawer(const mono::TransformSystem* transform_system)
    : m_transform_system(transform_system)
{
    m_package_sprite = mono::RenderSystem::GetSpriteFactory()->CreateSprite("res/sprites/cardboard_box_small.sprite");
    m_sprite_buffers = mono::BuildSpriteDrawBuffers(m_package_sprite->GetSpriteData());

    constexpr uint16_t indices[] = {
        0, 1, 2, 0, 2, 3
    };
    m_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, 6, indices);
}

void PackageAuxiliaryDrawer::Draw(mono::IRenderer& renderer) const
{
    if(g_package_info.entity_id == mono::INVALID_ID)
        return;

    const math::Vector package_world_position = m_transform_system->GetWorldPosition(g_package_info.entity_id);
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
        renderer.DrawSprite(m_package_sprite.get(), &m_sprite_buffers, m_indices.get(), 0);
    }

    if(g_package_info.cooldown_fraction > 0.0f)
    {
        const math::Quad world_bb = m_transform_system->GetWorldBoundingBox(g_package_info.entity_id);
        const math::Vector bottom_center = math::BottomCenter(world_bb);

        const math::Vector left = bottom_center - math::Vector(0.3f, 0.15f);
        const math::Vector right = bottom_center + math::Vector(0.3f, -0.15f);
        const math::Vector reload_dot = ((right - left) * g_package_info.cooldown_fraction) + left;

        renderer.DrawLines({ left, right }, mono::Color::OFF_WHITE, 4.0f);
        renderer.DrawPoints({ reload_dot }, mono::Color::RED, 8.0f);
    }
}

math::Quad PackageAuxiliaryDrawer::BoundingBox() const
{
    return math::InfQuad;
}
