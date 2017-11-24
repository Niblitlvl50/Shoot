
#pragma once

#include "Overlay.h"
#include "AIKnowledge.h"

#include "Math/Vector.h"

namespace game
{
    class PlayerStatsElement : public UIElement
    {
    public:

        PlayerStatsElement(const PlayerInfo& info, const math::Vector& position);
        void Draw(mono::IRenderer& renderer) const override;

        const PlayerInfo& m_info;
        const math::Vector m_position;
    };
}
