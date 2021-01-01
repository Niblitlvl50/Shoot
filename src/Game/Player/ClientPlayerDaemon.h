
#pragma once

#include "MonoFwd.h"
#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"

namespace game
{
    struct ClientPlayerSpawned;

    class ClientPlayerDaemon
    {
    public:

        ClientPlayerDaemon(class CameraSystem* camera_system, mono::EventHandler* event_handler);
        ~ClientPlayerDaemon();

        void SpawnPlayer1();

        mono::EventResult OnControllerAdded(const event::ControllerAddedEvent& event);
        mono::EventResult OnControllerRemoved(const event::ControllerRemovedEvent& event);
        mono::EventResult ClientSpawned(const ClientPlayerSpawned& message);

        class CameraSystem* m_camera_system;
        mono::EventHandler* m_event_handler;

        mono::EventToken<event::ControllerAddedEvent> m_added_token;
        mono::EventToken<event::ControllerRemovedEvent> m_removed_token;
        mono::EventToken<ClientPlayerSpawned> m_client_spawned_token;

        int m_player_one_controller_id = -1;
    };
}
