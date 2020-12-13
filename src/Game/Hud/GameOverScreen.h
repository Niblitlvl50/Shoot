
#pragma once

#include "MonoFwd.h"
#include "Zone/EntityBase.h"
#include "Math/Vector.h"
#include "System/System.h"

namespace game
{
    struct PlayerInfo;

    class GameOverScreen : public mono::EntityBase
    {
    public:

        GameOverScreen(
            const PlayerInfo& player_info,
            const math::Vector& position,
            const math::Vector& offscreen_position,
            mono::EventHandler* event_handler);
        void EntityDraw(mono::IRenderer& renderer) const override;
        void EntityUpdate(const mono::UpdateContext& update_context) override;

    private:

        const PlayerInfo& m_player_info;
        const math::Vector m_screen_position;
        const math::Vector m_offscreen_position;
        mono::EventHandler* m_event_handler;

        float m_timer;
        System::ControllerState m_last_state;
    };
}
