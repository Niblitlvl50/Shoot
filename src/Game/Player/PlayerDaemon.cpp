
#include "PlayerDaemon.h"
#include "PlayerLogic.h"
#include "Player/PlayerInfo.h"

#include "SystemContext.h"
#include "EntitySystem/Entity.h"
#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"

#include "DamageSystem.h"
#include "GameCamera/CameraSystem.h"
#include "Entity/EntityLogicSystem.h"

#include "EventHandler/EventHandler.h"
#include "Events/EventFuncFwd.h"
#include "Events/ControllerEvent.h"
#include "Events/PlayerConnectedEvent.h"
#include "Events/ScoreEvent.h"

#include "Network/NetworkMessage.h"
#include "Network/INetworkPipe.h"
#include "Network/NetworkSerialize.h"

#include "Component.h"

#include <functional>

using namespace game;

namespace
{
    uint32_t SpawnPlayer(
        game::PlayerInfo* player_info,
        const math::Vector& spawn_position,
        const System::ControllerState& controller,
        mono::IEntityManager* entity_system,
        mono::SystemContext* system_context,
        mono::EventHandler* event_handler,
        game::DamageCallback damage_callback)
    {
        mono::Entity player_entity = entity_system->CreateEntity("res/entities/player_entity.entity");

        mono::TransformSystem* transform_system = system_context->GetSystem<mono::TransformSystem>();
        math::Matrix& transform = transform_system->GetTransform(player_entity.id);
        math::Position(transform, spawn_position);
        transform_system->SetTransformState(player_entity.id, mono::TransformState::CLIENT);

        // No need to store the callback id, when destroyed this callback will be cleared up.
        game::DamageSystem* damage_system = system_context->GetSystem<DamageSystem>();
        damage_system->PreventReleaseOnDeath(player_entity.id, true);
        const uint32_t callback_id = damage_system->SetDamageCallback(player_entity.id, game::DamageType::ALL, damage_callback);
        (void)callback_id;

        game::EntityLogicSystem* logic_system = system_context->GetSystem<EntityLogicSystem>();
        entity_system->AddComponent(player_entity.id, BEHAVIOUR_COMPONENT);

        IEntityLogic* player_logic = new PlayerLogic(player_entity.id, player_info, event_handler, controller, system_context);
        logic_system->AddLogic(player_entity.id, player_logic);

        player_info->entity_id = player_entity.id;
        player_info->player_state = game::PlayerState::ALIVE;

        return player_entity.id;
    }
}

PlayerDaemon::PlayerDaemon(
    INetworkPipe* remote_connection,
    mono::IEntityManager* entity_system,
    mono::SystemContext* system_context,
    mono::EventHandler* event_handler,
    const math::Vector& player_spawn)
    : m_remote_connection(remote_connection)
    , m_entity_system(entity_system)
    , m_system_context(system_context)
    , m_event_handler(event_handler)
    , m_player_spawn(player_spawn)
{
    m_camera_system = m_system_context->GetSystem<CameraSystem>();

    using namespace std::placeholders;
    const event::ControllerAddedFunc& added_func = std::bind(&PlayerDaemon::OnControllerAdded, this, _1);
    const event::ControllerRemovedFunc& removed_func = std::bind(&PlayerDaemon::OnControllerRemoved, this, _1);

    m_added_token = m_event_handler->AddListener(added_func);
    m_removed_token = m_event_handler->AddListener(removed_func);

    const PlayerConnectedFunc& connected_func = std::bind(&PlayerDaemon::RemotePlayerConnected, this, _1);
    const PlayerDisconnectedFunc& disconnected_func = std::bind(&PlayerDaemon::RemotePlayerDisconnected, this, _1);
    const SpawnPlayerFunc& spawn_player_func = std::bind(&PlayerDaemon::OnSpawnPlayer, this, _1);
    const RespawnPlayerFunc& respawn_player_func = std::bind(&PlayerDaemon::OnRespawnPlayer, this, _1);
    const ScoreFunc& score_func = std::bind(&PlayerDaemon::PlayerScore, this, _1);

    m_player_connected_token = m_event_handler->AddListener(connected_func);
    m_player_disconnected_token = m_event_handler->AddListener(disconnected_func);
    m_spawn_player_token = m_event_handler->AddListener(spawn_player_func);
    m_respawn_player_token = m_event_handler->AddListener(respawn_player_func);

    const std::function<mono::EventResult (const RemoteInputMessage&)>& remote_input_func = std::bind(&PlayerDaemon::RemotePlayerInput, this, _1);
    m_remote_input_token = m_event_handler->AddListener(remote_input_func);

    const std::function<mono::EventResult (const ViewportMessage&)>& remote_viewport_func = std::bind(&PlayerDaemon::RemotePlayerViewport, this, _1);
    m_remote_viewport_token = m_event_handler->AddListener(remote_viewport_func);

    m_score_token = m_event_handler->AddListener(score_func);

    if(System::IsControllerActive(System::ControllerId::Primary))
        SpawnLocalPlayer(System::GetControllerId(System::ControllerId::Primary));

    if(System::IsControllerActive(System::ControllerId::Secondary))
        SpawnLocalPlayer(System::GetControllerId(System::ControllerId::Secondary));
}

PlayerDaemon::~PlayerDaemon()
{
    m_event_handler->RemoveListener(m_added_token);
    m_event_handler->RemoveListener(m_removed_token);

    m_event_handler->RemoveListener(m_player_connected_token);
    m_event_handler->RemoveListener(m_player_disconnected_token);
    m_event_handler->RemoveListener(m_spawn_player_token);
    m_event_handler->RemoveListener(m_respawn_player_token);
    m_event_handler->RemoveListener(m_remote_input_token);
    m_event_handler->RemoveListener(m_remote_viewport_token);
    m_event_handler->RemoveListener(m_score_token);

    for(int index = 0; index < game::n_players; ++index)
    {
        game::PlayerInfo& player_info = g_players[index];
        if(player_info.player_state == game::PlayerState::ALIVE)
            DespawnPlayer(&player_info);
    }

    m_camera_system->Unfollow();
}

std::vector<uint32_t> PlayerDaemon::GetPlayerIds() const
{
    std::vector<uint32_t> ids;
    for(const PlayerInfo* player : game::GetActivePlayers())
    {
        if(player)
            ids.push_back(player->entity_id);
    }

    return ids;
}

void PlayerDaemon::SpawnLocalPlayer(int controller_id)
{
    game::PlayerInfo* allocated_player_info = AllocatePlayerInfo();
    if(!allocated_player_info)
    {
        System::Log("Unable to allocate player info for local player.\n");
        return;
    }

    allocated_player_info->lives = 3;
    m_controller_id_to_player_info[controller_id] = allocated_player_info;

    const auto destroyed_func = [this, allocated_player_info](uint32_t entity_id, int damage, uint32_t id_who_did_damage, DamageType type) {

        if(type == DamageType::DESTROYED)
        {
            allocated_player_info->player_state = game::PlayerState::DEAD;
            allocated_player_info->lives--;
            if(allocated_player_info->lives <= 0)
            {
                DespawnPlayer(allocated_player_info);
                m_camera_system->Unfollow();
            }
        }
        else if(type == DamageType::DAMAGED)
        {
            m_camera_system->AddCameraShake(250);
        }
    };

    const uint32_t spawned_id = SpawnPlayer(
        allocated_player_info,
        m_player_spawn,
        System::GetController(System::ControllerId(controller_id)),
        m_entity_system,
        m_system_context,
        m_event_handler,
        destroyed_func);
    
    m_camera_system->Follow(spawned_id, math::Vector(0.0f, 0.0f));
}

void PlayerDaemon::DespawnPlayer(PlayerInfo* player_info)
{
    m_entity_system->ReleaseEntity(player_info->entity_id);
    ReleasePlayerInfo(player_info);
}

mono::EventResult PlayerDaemon::OnControllerAdded(const event::ControllerAddedEvent& event)
{
    SpawnLocalPlayer(event.controller_id);
    return mono::EventResult::PASS_ON;
}

mono::EventResult PlayerDaemon::OnControllerRemoved(const event::ControllerRemovedEvent& event)
{
    const auto it = m_controller_id_to_player_info.find(event.controller_id);
    if(it != m_controller_id_to_player_info.end())
    {
        DespawnPlayer(it->second);
        m_controller_id_to_player_info.erase(event.controller_id);
        m_camera_system->Unfollow();
    }

    return mono::EventResult::PASS_ON;
}

mono::EventResult PlayerDaemon::RemotePlayerConnected(const PlayerConnectedEvent& event)
{
    auto it = m_remote_players.find(event.address);
    if(it != m_remote_players.end())
        return mono::EventResult::HANDLED;

    System::Log("PlayerDaemon|Remote player connected, %s\n", network::AddressToString(event.address).c_str());

    game::PlayerInfo* allocated_player_info = AllocatePlayerInfo();
    if(!allocated_player_info)
    {
        System::Log("Unable to allocate player info for local player.\n");
        return mono::EventResult::HANDLED;
    }

    allocated_player_info->lives = 3;
    PlayerDaemon::RemotePlayerData& remote_player_data = m_remote_players[event.address];
    remote_player_data.player_info = allocated_player_info;

    const auto remote_player_destroyed = [this, allocated_player_info](uint32_t entity_id, int damage, uint32_t id_who_did_damage, DamageType type) {

        allocated_player_info->player_state = game::PlayerState::DEAD;
        allocated_player_info->lives--;
        if(allocated_player_info->lives <= 0)
            DespawnPlayer(allocated_player_info);
    };

    const uint32_t spawned_id = SpawnPlayer(
        remote_player_data.player_info,
        m_player_spawn,
        remote_player_data.controller_state,
        m_entity_system,
        m_system_context,
        m_event_handler,
        remote_player_destroyed);

    ClientPlayerSpawned client_spawned_message;
    client_spawned_message.client_entity_id = spawned_id;

    NetworkMessage reply_message;
    reply_message.payload = SerializeMessage(client_spawned_message);

    m_remote_connection->SendMessageTo(reply_message, event.address);

    return mono::EventResult::HANDLED;
}

mono::EventResult PlayerDaemon::RemotePlayerDisconnected(const PlayerDisconnectedEvent& event)
{
    auto it = m_remote_players.find(event.address);
    if(it != m_remote_players.end())
    {
        DespawnPlayer(it->second.player_info);
        m_remote_players.erase(it);
    }

    return mono::EventResult::HANDLED;
}

mono::EventResult PlayerDaemon::RemotePlayerInput(const RemoteInputMessage& event)
{
    auto it = m_remote_players.find(event.sender);
    if(it != m_remote_players.end())
        it->second.controller_state = event.controller_state;

    return mono::EventResult::HANDLED;
}

mono::EventResult PlayerDaemon::RemotePlayerViewport(const ViewportMessage& message)
{
    auto it = m_remote_players.find(message.sender);
    if(it != m_remote_players.end())
        it->second.player_info->viewport = message.viewport;

    return mono::EventResult::PASS_ON;
}

mono::EventResult PlayerDaemon::PlayerScore(const ScoreEvent& event)
{
    game::PlayerInfo* player_info = FindPlayerInfoFromEntityId(event.entity_id);
    if(player_info)
        player_info->score += event.score;

    return mono::EventResult::PASS_ON;
}

mono::EventResult PlayerDaemon::OnSpawnPlayer(const SpawnPlayerEvent& event)
{
    //SpawnLocalPlayer();
    return mono::EventResult::HANDLED;
}

mono::EventResult PlayerDaemon::OnRespawnPlayer(const RespawnPlayerEvent& event)
{
    game::PlayerInfo* player_info = game::FindPlayerInfoFromEntityId(event.entity_id);
    if(player_info)
    {
        mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
        math::Matrix& transform = transform_system->GetTransform(event.entity_id);
        math::Position(transform, m_player_spawn);
        transform_system->SetTransformState(event.entity_id, mono::TransformState::CLIENT);

        game::DamageSystem* damage_system = m_system_context->GetSystem<game::DamageSystem>();
        game::DamageRecord* record = damage_system->GetDamageRecord(event.entity_id);
        record->health = record->full_health;
        record->last_damaged_timestamp = 0;

        player_info->player_state = game::PlayerState::ALIVE;
    }

    return mono::EventResult::HANDLED;
}
