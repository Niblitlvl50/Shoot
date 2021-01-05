
#include "ClientPlayerDaemon.h"
#include "GameCamera/CameraSystem.h"
#include "AIKnowledge.h"
#include "Network/NetworkMessage.h"

#include "EntitySystem/Entity.h"

#include "EventHandler/EventHandler.h"
#include "Events/EventFuncFwd.h"
#include "Events/ControllerEvent.h"
#include "Events/PlayerConnectedEvent.h"

#include "System/System.h"

#include <functional>

using namespace game;

ClientPlayerDaemon::ClientPlayerDaemon(CameraSystem* camera_system, mono::EventHandler* event_handler)
    : m_camera_system(camera_system)
    , m_event_handler(event_handler)
{
    using namespace std::placeholders;
    const event::ControllerAddedFunc& added_func = std::bind(&ClientPlayerDaemon::OnControllerAdded, this, _1);
    const event::ControllerRemovedFunc& removed_func = std::bind(&ClientPlayerDaemon::OnControllerRemoved, this, _1);
    const std::function<mono::EventResult (const ClientPlayerSpawned&)>& client_spawned = std::bind(&ClientPlayerDaemon::ClientSpawned, this, _1);

    m_added_token = m_event_handler->AddListener(added_func);
    m_removed_token = m_event_handler->AddListener(removed_func);
    m_client_spawned_token = m_event_handler->AddListener(client_spawned);

    if(System::IsControllerActive(System::ControllerId::Primary))
    {
        m_player_one_controller_id = System::GetControllerId(System::ControllerId::Primary);
        SpawnPlayer1();
    }
}

ClientPlayerDaemon::~ClientPlayerDaemon()
{
    m_event_handler->RemoveListener(m_added_token);
    m_event_handler->RemoveListener(m_removed_token);
    m_event_handler->RemoveListener(m_client_spawned_token);

    m_camera_system->Unfollow();
}

void ClientPlayerDaemon::SpawnPlayer1()
{
    System::Log("ClientPlayerDaemon|Spawn player 1\n");
    //game::g_player_one.entity_id = player_entity.id;
    //game::g_player_one.is_active = true;
}

mono::EventResult ClientPlayerDaemon::OnControllerAdded(const event::ControllerAddedEvent& event)
{
    if(game::g_player_one.player_state == game::PlayerState::NOT_SPAWNED)
    {
        SpawnPlayer1();
        m_player_one_controller_id = event.id;
    }

    return mono::EventResult::PASS_ON;
}

mono::EventResult ClientPlayerDaemon::OnControllerRemoved(const event::ControllerRemovedEvent& event)
{
    if(event.id == m_player_one_controller_id)
    {
        game::g_player_one.entity_id = mono::INVALID_ID;
        game::g_player_one.player_state = game::PlayerState::NOT_SPAWNED;
        m_camera_system->Unfollow();
    }

    return mono::EventResult::PASS_ON;
}

mono::EventResult ClientPlayerDaemon::ClientSpawned(const ClientPlayerSpawned& message)
{
    game::g_player_one.entity_id = message.client_entity_id;
    game::g_player_one.player_state = game::PlayerState::ALIVE;

    m_camera_system->Follow(g_player_one.entity_id, math::Vector(0.0f, 3.0f));

    return mono::EventResult::PASS_ON;
}
