
#pragma once

#include "Zone/EntityBase.h"
#include "Math/Vector.h"

namespace game
{
    struct PlayerInfo;

    class PlayerScoreElement : public mono::EntityBase
    {
    public:

        PlayerScoreElement(const PlayerInfo& player_info, const math::Vector& position);
        ~PlayerScoreElement();
        void Draw(mono::IRenderer& renderer) const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        const PlayerInfo& m_player_info;
        int m_current_score;
        // const math::Vector m_screen_position;
        // const math::Vector m_offscreen_position;
        // float m_timer;
    };
}
