
#pragma once

#include "MonoFwd.h"
#include "IGameMode.h"
#include "EventHandler/EventToken.h"
#include "Math/Vector.h"

#include <memory>

namespace game
{
    class PacketDeliveryGameMode : public IGameMode
    {
    public:

        PacketDeliveryGameMode(const math::Vector& player_spawn_point);
        ~PacketDeliveryGameMode();

        void Begin(mono::IZone* zone, mono::SystemContext* system_context, mono::EventHandler* event_handler) override;
        int End(mono::IZone* zone) override;
        void Update(const mono::UpdateContext& update_context) override;

        mono::SystemContext* m_system_context;
        mono::EventHandler* m_event_handler;
        math::Vector m_player_spawn_point;
        class TriggerSystem* m_trigger_system;

        std::unique_ptr<class PlayerDaemon> m_player_daemon;
        std::unique_ptr<class PlayerDeathScreen> m_player_death_screen;
        std::unique_ptr<class PlayerUIElement> m_player_ui;

        mono::EventToken<struct GameOverEvent> m_gameover_token;

        int m_next_zone;
        uint32_t m_level_completed_trigger;
    };
}
