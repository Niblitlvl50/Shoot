
#include "PlayerStatsElement.h"

#include "FontIds.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Vector.h"

#include <cstdio>

using namespace game;

PlayerStatsElement::PlayerStatsElement(const PlayerInfo& info, const math::Vector& position)
    : m_info(info),
      m_position(position)
{ }

void PlayerStatsElement::Draw(mono::IRenderer& renderer) const
{
    if(!m_info.is_active)
        return;

    constexpr mono::Color::RGBA color(1, 0, 0);

    char text[32] = { '\0' };
    std::snprintf(text, 32, "Player - ammo: %u", m_info.ammunition);

    renderer.DrawText(game::PIXELETTE_LARGE, text, m_position, false, color);
}
