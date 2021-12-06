
#include "PacketDeliveryGameMode.h"
#include "Hud/PlayerDeathScreen.h"
#include "Hud/PlayerUIElement.h"
#include "Zones/ZoneFlow.h"
#include "Events/GameEvents.h"
#include "Events/GameEventFuncFwd.h"
#include "Player/PlayerDaemon.h"
#include "TriggerSystem/TriggerSystem.h"
#include "Network/ServerManager.h"
#include "RenderLayers.h"


#include "EntitySystem/EntitySystem.h"
#include "Events/QuitEvent.h"
#include "EventHandler/EventHandler.h"
#include "SystemContext.h"
#include "System/Hash.h"
#include "Zone/IZone.h"

namespace
{
    const uint32_t level_completed_hash = hash::Hash("level_completed");

    mono::EventResult GameOverAndQuit(int& next_zone, mono::EventHandler* event_handler)
    {
        next_zone = game::ZoneFlow::GAME_OVER_SCREEN;

        const auto send_quit = [](void* data) {
            mono::EventHandler* event_handler = static_cast<mono::EventHandler*>(data);
            event_handler->DispatchEvent(event::QuitEvent());
        };
        System::CreateTimer(1000, System::TimerProperties::AUTO_DELETE | System::TimerProperties::ONE_SHOT, send_quit, event_handler);

        return mono::EventResult::PASS_ON;
    }

    void GameCompleted(int32_t trigger_id, int& next_zone, mono::EventHandler* event_handler)
    {
        next_zone = game::ZoneFlow::END_SCREEN;
        event_handler->DispatchEvent(event::QuitEvent());
    }
}


using namespace game;

PacketDeliveryGameMode::PacketDeliveryGameMode(const math::Vector& player_spawn_point)
    : m_player_spawn_point(player_spawn_point)
    , m_next_zone(ZoneFlow::TITLE_SCREEN)
{ }

PacketDeliveryGameMode::~PacketDeliveryGameMode() = default;

void PacketDeliveryGameMode::Begin(mono::IZone* zone, mono::SystemContext* system_context, mono::EventHandler* event_handler)
{
    m_event_handler = event_handler;

    mono::EntitySystem* entity_system = system_context->GetSystem<mono::EntitySystem>();
    game::ServerManager* server_manager = system_context->GetSystem<game::ServerManager>();
    m_trigger_system = system_context->GetSystem<game::TriggerSystem>();

    const GameOverFunc on_game_over = std::bind(GameOverAndQuit, std::ref(m_next_zone), m_event_handler);
    m_gameover_token = m_event_handler->AddListener(on_game_over);

    // Player
    m_player_daemon = std::make_unique<PlayerDaemon>(
        server_manager, entity_system, system_context, m_event_handler, m_player_spawn_point);

    using namespace std::placeholders;
    m_level_completed_trigger = m_trigger_system->RegisterTriggerCallback(
        level_completed_hash,
        std::bind(GameCompleted, _1, std::ref(m_next_zone), m_event_handler),
        mono::INVALID_ID);

    m_player_death_screen = std::make_unique<PlayerDeathScreen>(game::g_players[0], m_event_handler);
    m_player_ui = std::make_unique<PlayerUIElement>(game::g_players[0]);

    zone->AddUpdatableDrawable(m_player_death_screen.get(), LayerId::UI);
    zone->AddUpdatableDrawable(m_player_ui.get(), LayerId::UI);
}

int PacketDeliveryGameMode::End(mono::IZone* zone)
{
    zone->RemoveUpdatableDrawable(m_player_death_screen.get());
    zone->RemoveUpdatableDrawable(m_player_ui.get());

    m_event_handler->RemoveListener(m_gameover_token);
    m_trigger_system->RemoveTriggerCallback(level_completed_hash, m_level_completed_trigger, 0);

    return m_next_zone;
}

void PacketDeliveryGameMode::Update(const mono::UpdateContext& update_context)
{

}
