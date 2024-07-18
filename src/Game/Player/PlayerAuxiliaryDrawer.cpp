
#include "PlayerAuxiliaryDrawer.h"

#include "GameCamera/CameraSystem.h"
#include "Math/Quad.h"
#include "Math/MathFunctions.h"
#include "Math/CriticalDampedSpring.h"
#include "Player/PlayerInfo.h"
#include "Player/PlayerAbilities.h"
#include "Weapons/WeaponModifierTypes.h"

#include "Camera/ICamera.h"
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
    struct AimlineRenderData
    {
        std::unique_ptr<mono::IRenderBuffer> vertices;
        std::unique_ptr<mono::IRenderBuffer> colors;
        std::unique_ptr<mono::IElementBuffer> indices;
    };

    AimlineRenderData GenerateAimLine(
        float start_offset, float mid_point_offset, float length, float width, float cutoff_length, const mono::Color::RGBA& color)
    {
        const float min_length = std::min(length, cutoff_length);
        const float min_mid_point_offset = std::min(mid_point_offset, min_length);

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
            mono::Color::MakeWithAlpha(color, 0.3f),
            mono::Color::MakeWithAlpha(color, 0.3f),
            mono::Color::MakeWithAlpha(color, 0.0f),
            mono::Color::MakeWithAlpha(color, 0.0f),
        };

        const uint16_t aim_lines_indices[] = {
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

    AimlineRenderData GenerateStaminaBar(float player_stamina_fraction, const math::Vector& stamina_bar_center, float length, float width, const mono::Color::RGBA& color)
    {
        const float stamina_value = length * player_stamina_fraction;
        const float half_length = length / 2.0f;
        const float half_width = width / 2.0f;

        const math::Vector left_center = stamina_bar_center - math::Vector(half_length, 0.0f);

        const math::Vector vertices[] = {
            left_center + math::Vector(0.0f, -half_width),
            left_center + math::Vector(0.0f, half_width),
            left_center + math::Vector(stamina_value, half_width),
            left_center + math::Vector(stamina_value, -half_width),

            left_center + math::Vector(stamina_value, -half_width),
            left_center + math::Vector(stamina_value, half_width),
            left_center + math::Vector(length, -half_width),
            left_center + math::Vector(length, half_width),
        };

        const mono::Color::RGBA colors[] = {
            mono::Color::MakeWithAlpha(color, 1.0f),
            mono::Color::MakeWithAlpha(color, 1.0f),
            mono::Color::MakeWithAlpha(color, 1.0f),
            mono::Color::MakeWithAlpha(color, 1.0f),
            mono::Color::GRAY,
            mono::Color::GRAY,
            mono::Color::GRAY,
            mono::Color::GRAY,
        };

        const uint16_t indices[] = {
            0, 1, 2,
            0, 2, 3,
            5, 4, 6,
            5, 6, 7,
        };

        AimlineRenderData render_data;
        render_data.vertices = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 2, std::size(vertices), vertices);
        render_data.colors = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 4, std::size(colors), colors);
        render_data.indices = mono::CreateElementBuffer(mono::BufferType::STATIC, std::size(indices), indices);

        return render_data;
    }

    constexpr float laser_start_offset = 0.25f;
    constexpr float laser_mid_point_offset = 1.0f;
    constexpr float laser_cutoff_length = 7.0f;
    constexpr float laser_width = 0.025f;

    constexpr float stamina_bar_length = 0.6f;
    constexpr float stamina_bar_width = 0.05f;
}

PlayerAuxiliaryDrawer::PlayerAuxiliaryDrawer(const game::CameraSystem* camera_system, const mono::TransformSystem* transform_system)
    : m_camera_system(camera_system)
    , m_transform_system(transform_system)
{
    for(const char* sprite_file : g_ability_to_sprite)
    {
        AbilityRenderData render_data;
        render_data.sprite = mono::RenderSystem::GetSpriteFactory()->CreateSprite(sprite_file);
        render_data.sprite_buffers = mono::BuildSpriteDrawBuffers(render_data.sprite->GetSpriteData());
        m_ability_render_datas.push_back(std::move(render_data));
    }

    for(const char* sprite_file : g_weapon_modifier_to_sprite)
    {
        AbilityRenderData render_data;
        render_data.sprite = mono::RenderSystem::GetSpriteFactory()->CreateSprite(sprite_file);
        render_data.sprite_buffers = mono::BuildSpriteDrawBuffers(render_data.sprite->GetSpriteData());
        m_powerup_render_datas.push_back(std::move(render_data));
    }

    m_crosshair_render_data.sprite = mono::RenderSystem::GetSpriteFactory()->CreateSprite(g_player_crosshair_sprite);
    m_crosshair_render_data.sprite_buffers = mono::BuildSpriteDrawBuffers(m_crosshair_render_data.sprite->GetSpriteData());

    m_crosshair_render_data.sprite->SetShade(mono::Color::RGBA(1.0f, 0.65f, 0.65f));

    constexpr uint16_t indices[] = {
        0, 1, 2, 0, 2, 3
    };
    m_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, 6, indices);

    std::memset(m_ability_data, 0, sizeof(m_ability_data));
}

void PlayerAuxiliaryDrawer::Draw(mono::IRenderer& renderer) const
{
    std::vector<const game::PlayerInfo*> active_players;

    for(const game::PlayerInfo* player_info : GetActivePlayers())
    {
        if(player_info)
            active_players.push_back(player_info);
    }

    DrawLasers(active_players, renderer);
    DrawStaminaBar(active_players, renderer);
    DrawAbilities(active_players, renderer);
    DrawPowerups(active_players, renderer);
    DrawCrosshair(active_players, renderer);
}

math::Quad PlayerAuxiliaryDrawer::BoundingBox() const
{
    return math::InfQuad;
}

void PlayerAuxiliaryDrawer::DrawLasers(const std::vector<const game::PlayerInfo*>& players, mono::IRenderer& renderer) const
{
    struct AimlineData
    {
        math::Matrix transform;
        AimlineRenderData render_data;
    };
    std::vector<AimlineData> aimline_transforms;
    std::vector<math::Vector> aim_target_points;
    std::vector<mono::Color::RGBA> aim_target_colors;

    constexpr mono::Color::RGBA aim_line_colors[3] = {
        mono::Color::RED,
        mono::Color::GREEN,
        mono::Color::BLUE
    };

    for(const game::PlayerInfo* player_info : players)
    {
        if(!player_info->laser_sight)
            continue;

        const uint32_t player_index = FindPlayerIndex(player_info);

        const float aim_direction_rad = math::AngleFromVector(player_info->aim_direction);
        const float laser_length = math::DistanceBetween(player_info->position, player_info->aim_target);
        const mono::Color::RGBA& laser_color = aim_line_colors[player_index];

        aimline_transforms.push_back({
            math::CreateMatrixWithPositionRotation(player_info->position, aim_direction_rad + math::PI_2()),
            GenerateAimLine(laser_start_offset, laser_mid_point_offset, laser_length, laser_width, laser_cutoff_length, laser_color)
        });

        aim_target_points.push_back(player_info->aim_target);
        aim_target_colors.push_back(mono::Color::MakeWithAlpha(laser_color, 0.5f));
    }

    for(const AimlineData& aimline_data : aimline_transforms)
    {
        const auto transform_scope = mono::MakeTransformScope(aimline_data.transform, &renderer);
        renderer.DrawTrianges(
            aimline_data.render_data.vertices.get(),
            aimline_data.render_data.colors.get(),
            aimline_data.render_data.indices.get(),
            0,
            aimline_data.render_data.indices->Size());
    }

    renderer.DrawPoints(aim_target_points, aim_target_colors, 12.0f);
}

void PlayerAuxiliaryDrawer::DrawStaminaBar(const std::vector<const game::PlayerInfo*>& players, mono::IRenderer& renderer) const
{
    std::vector<AimlineRenderData> stamina_render_data;

    for(const game::PlayerInfo* player_info : players)
    {
        if(player_info->stamina_fraction >= 1.0f)
            continue;

        const math::Quad world_bb = m_transform_system->GetWorldBoundingBox(player_info->entity_id);
        const math::Vector bottom_center = math::BottomCenter(world_bb);
        stamina_render_data.push_back(GenerateStaminaBar(player_info->stamina_fraction, bottom_center + math::Vector(0.0f, -0.05f), stamina_bar_length, stamina_bar_width, mono::Color::BLUE_GRAY));
    }

    for(const AimlineRenderData& render_data : stamina_render_data)
    {
        renderer.DrawTrianges(
            render_data.vertices.get(), render_data.colors.get(), render_data.indices.get(), 0, render_data.indices->Size());
    }
}

void PlayerAuxiliaryDrawer::DrawAbilities(const std::vector<const game::PlayerInfo*>& players, mono::IRenderer& renderer) const
{
    std::vector<math::Vector> cooldown_lines;

    struct AbilityPoint
    {
        math::Vector point;
        uint32_t render_data_index;
    };
    std::vector<AbilityPoint> ability_points;

    const uint32_t current_timestamp = renderer.GetTimestamp();

    for(const game::PlayerInfo* player_info : players)
    {
        const uint32_t player_index = FindPlayerIndex(player_info);

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

    renderer.DrawLines(cooldown_lines, mono::Color::RGBA(0.8f, 0.8f, 0.8f), 4.0f);

    for(const AbilityPoint& ability_point : ability_points)
    {
        const math::Matrix& transform = math::CreateMatrixWithPositionScale(ability_point.point, 0.35f);
        const auto transform_scope = mono::MakeTransformScope(transform, &renderer);

        const AbilityRenderData& render_data = m_ability_render_datas[ability_point.render_data_index];
        renderer.DrawSprite(render_data.sprite.get(), &render_data.sprite_buffers, m_indices.get(), 0);
    }
}

void PlayerAuxiliaryDrawer::DrawPowerups(const std::vector<const game::PlayerInfo*>& players, mono::IRenderer& renderer) const
{
    struct ActivePowerups
    {
        math::Vector point;
        uint32_t render_data_index;
    };
    std::vector<ActivePowerups> active_powerups;

    for(const game::PlayerInfo* player_info : players)
    {
        const bool is_active_powerups = (player_info->powerup_fraction > 0.0f);
        if(!is_active_powerups)
            continue;

        const math::Quad world_bb = m_transform_system->GetWorldBoundingBox(player_info->entity_id);
        const math::Vector powerup_draw_point = math::TopRight(world_bb);

        active_powerups.push_back({ powerup_draw_point, player_info->powerup_id });
    }

    for(const ActivePowerups& powerup : active_powerups)
    {
        const math::Matrix& transform = math::CreateMatrixWithPositionScale(powerup.point, 0.65f);
        const auto transform_scope = mono::MakeTransformScope(transform, &renderer);

        const AbilityRenderData& render_data = m_powerup_render_datas[powerup.render_data_index];
        renderer.DrawSprite(render_data.sprite.get(), &render_data.sprite_buffers, m_indices.get(), 0);
    }
}

void PlayerAuxiliaryDrawer::DrawCrosshair(const std::vector<const game::PlayerInfo*>& players, mono::IRenderer& renderer) const
{
    for(const game::PlayerInfo* player_info : players)
    {
        const bool keyboard_or_mouse = 
            player_info->last_used_input == mono::InputContextType::Keyboard || player_info->last_used_input == mono::InputContextType::Mouse;
        if(!keyboard_or_mouse)
            continue;

        const math::Vector crosshair_world_position =
            m_camera_system->GetActiveCamera()->ScreenToWorld(player_info->aim_crosshair_screen_position);

        const math::Matrix& transform = math::CreateMatrixWithPosition(crosshair_world_position);
        const auto transform_scope = mono::MakeTransformScope(transform, &renderer);
        renderer.DrawSprite(
            m_crosshair_render_data.sprite.get(), &m_crosshair_render_data.sprite_buffers, m_indices.get(), 0);
    }
}


PackageAuxiliaryDrawer::PackageAuxiliaryDrawer(const mono::TransformSystem* transform_system)
    : m_transform_system(transform_system)
{ }

void PackageAuxiliaryDrawer::Draw(mono::IRenderer& renderer) const
{
    if(g_package_info.entity_id == mono::INVALID_ID)
        return;

    if(g_package_info.cooldown_fraction > 0.0f)
    {
        const math::Quad world_bb = m_transform_system->GetWorldBoundingBox(g_package_info.entity_id);
        const bool is_outside_view = (renderer.Cull(world_bb) == mono::CullResult::OUTSIDE_VIEW);
        if(is_outside_view)
            return;

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

