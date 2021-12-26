
#include "PlayerAuxiliaryDrawer.h"
#include "Math/Quad.h"
#include "Player/PlayerInfo.h"

#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;

PlayerAuxiliaryDrawer::PlayerAuxiliaryDrawer(const mono::TransformSystem* transform_system)
    : m_transform_system(transform_system)
{}

void PlayerAuxiliaryDrawer::Draw(mono::IRenderer& renderer) const
{
    std::vector<math::Vector> aim_lines;
    std::vector<math::Vector> reload_lines;
    std::vector<math::Vector> points;

    for(const game::PlayerInfo* player_info : GetActivePlayers())
    {
        if(!player_info)
            continue;

        const math::Vector aim_vector = math::VectorFromAngle(player_info->aim_direction);

        aim_lines.push_back(player_info->position + aim_vector * 0.25f);
        aim_lines.push_back(player_info->position + aim_vector * 3.0f);

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

    renderer.DrawLines(aim_lines, mono::Color::RED, 4.0f);
    renderer.DrawLines(reload_lines, mono::Color::OFF_WHITE, 4.0f);
    renderer.DrawPoints(points, mono::Color::BLACK, 8.0f);
}

math::Quad PlayerAuxiliaryDrawer::BoundingBox() const
{
    return math::InfQuad;
}
