
#pragma once

#include "MonoFwd.h"
#include "IUpdatable.h"
#include "Math/Vector.h"
#include "System/System.h"

namespace game
{
    struct PlayerInfo;

    class GameOverScreen : public mono::IUpdatable
    {
    public:

        GameOverScreen(
            const PlayerInfo& player_info,
            const math::Vector& position,
            const math::Vector& offscreen_position,
            mono::SpriteSystem* sprite_system,
            mono::IEntityManager* entity_manager,
            mono::EventHandler* event_handler);
        void Update(const mono::UpdateContext& update_context) override;
        //void EntityDraw(mono::IRenderer& renderer) const override;

    private:

        const PlayerInfo& m_player_info;
        math::Vector m_position;
        const math::Vector m_screen_position;
        const math::Vector m_offscreen_position;
        mono::EventHandler* m_event_handler;

        float m_timer;
        System::ControllerState m_last_state;
    };
}
