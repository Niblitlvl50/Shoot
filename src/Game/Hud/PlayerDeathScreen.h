
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "System/System.h"

#include "UIElements.h"

namespace game
{
    struct PlayerInfo;

    class PlayerDeathScreen : public game::UIOverlay
    {
    public:

        PlayerDeathScreen(const PlayerInfo& player_info, mono::EventHandler* event_handler);
        void Update(const mono::UpdateContext& update_context) override;

    private:

        const PlayerInfo& m_player_info;
        math::Vector m_screen_position;
        math::Vector m_offscreen_position;
        mono::EventHandler* m_event_handler;

        float m_timer;
        System::ControllerState m_last_state;
    };
}
