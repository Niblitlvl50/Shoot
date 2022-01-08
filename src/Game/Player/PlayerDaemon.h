
#pragma once

#include "MonoFwd.h"
#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"
#include "System/System.h"
#include "System/Network.h"

#include "Events/GameEventFuncFwd.h"
#include "Player/PlayerInfo.h"
#include "DamageSystem.h"

#include <vector>
#include <unordered_map>

namespace game
{
    class INetworkPipe;
    struct RemoteInputMessage;
    struct ViewportMessage;
    struct ClientPlayerSpawned;

    enum class PlayerSpawnState
    {
        SPAWNED,
        RESPAWNED
    };
    using PlayerSpawnedCallback = std::function<void (PlayerSpawnState spawn_state, uint32_t entity_id, const math::Vector& position)>;

    class PlayerDaemon
    {
    public:

        PlayerDaemon(
            INetworkPipe* remote_connection,
            mono::IEntityManager* entity_system,
            mono::SystemContext* system_context,
            mono::EventHandler* event_handler,
            const math::Vector& player_spawn,
            const PlayerSpawnedCallback& player_spawned_cb);
        ~PlayerDaemon();

        void SpawnLocalPlayer(int player_index, int controller_id);
        void DespawnPlayer(PlayerInfo* player_info);
        std::vector<uint32_t> GetPlayerIds() const;

    private:

        uint32_t SpawnPlayer(
            game::PlayerInfo* player_info,
            const math::Vector& spawn_position,
            const System::ControllerState& controller,
            mono::IEntityManager* entity_system,
            mono::SystemContext* system_context,
            mono::EventHandler* event_handler,
            const game::DamageCallback& damage_callback);

        mono::EventResult OnControllerAdded(const event::ControllerAddedEvent& event);
        mono::EventResult OnControllerRemoved(const event::ControllerRemovedEvent& event);
        mono::EventResult RemotePlayerConnected(const PlayerConnectedEvent& event);
        mono::EventResult RemotePlayerDisconnected(const PlayerDisconnectedEvent& event);
        mono::EventResult RemotePlayerInput(const RemoteInputMessage& event);
        mono::EventResult RemotePlayerViewport(const ViewportMessage& message);

        mono::EventResult OnSpawnPlayer(const SpawnPlayerEvent& event);
        mono::EventResult OnDespawnPlayer(const DespawnPlayerEvent& event);
        mono::EventResult OnRespawnPlayer(const RespawnPlayerEvent& event);

        class CameraSystem* m_camera_system;
        INetworkPipe* m_remote_connection;
        mono::IEntityManager* m_entity_system;
        mono::SystemContext* m_system_context;
        mono::EventHandler* m_event_handler;

        math::Vector m_player_spawn;
        PlayerSpawnedCallback m_player_spawned_callback;

        std::vector<const char*> m_player_entities;

        mono::EventToken<event::ControllerAddedEvent> m_added_token;
        mono::EventToken<event::ControllerRemovedEvent> m_removed_token;
        mono::EventToken<PlayerConnectedEvent> m_player_connected_token;
        mono::EventToken<PlayerDisconnectedEvent> m_player_disconnected_token;
        mono::EventToken<RemoteInputMessage> m_remote_input_token;
        mono::EventToken<ViewportMessage> m_remote_viewport_token;
        mono::EventToken<SpawnPlayerEvent> m_spawn_player_token;
        mono::EventToken<DespawnPlayerEvent> m_despawn_player_token;
        mono::EventToken<RespawnPlayerEvent> m_respawn_player_token;

        std::unordered_map<int, PlayerInfo*> m_controller_id_to_player_info;

        struct RemotePlayerData
        {
            PlayerInfo* player_info;
            System::ControllerState controller_state;
        };
        std::unordered_map<network::Address, RemotePlayerData> m_remote_players;

    };
}
