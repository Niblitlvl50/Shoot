
#include "PlayerDaemonSystem.h"
#include "Player/PlayerLogic.h"
#include "Player/PackageLogic.h"
#include "Player/PlayerInfo.h"
#include "PlayerFamiliarLogic.h"
#include "DamageSystem/DamageSystem.h"

#include "SystemContext.h"
#include "EntitySystem/Entity.h"
#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Random.h"

#include "DamageSystem/DamageSystem.h"
#include "GameCamera/CameraSystem.h"
#include "Entity/EntityLogicSystem.h"
#include "Input/InputSystem.h"

#include "EventHandler/EventHandler.h"
#include "Events/EventFuncFwd.h"
#include "Events/ControllerEvent.h"
#include "Events/PlayerEvents.h"
#include "Events/GameEvents.h"
#include "Events/RemoteEventFuncFwd.h"

#include "Network/NetworkMessage.h"
#include "Network/INetworkPipe.h"
#include "Network/NetworkSerialize.h"

#include "System/Hash.h"
#include "Entity/Component.h"

#include "System/File.h"
#include "nlohmann/json.hpp"

#include <functional>

using namespace game;

PlayerDaemonSystem::PlayerDaemonSystem(
    INetworkPipe* remote_connection,
    mono::IEntityManager* entity_system,
    mono::SystemContext* system_context,
    mono::EventHandler* event_handler)
    : m_remote_connection(remote_connection)
    , m_entity_system(entity_system)
    , m_system_context(system_context)
    , m_event_handler(event_handler)
    , m_spawn_players(false)
{
    const std::vector<byte> file_data = file::FileReadAll("res/player_config.json");
    const nlohmann::json& json = nlohmann::json::parse(file_data);

    m_player_entities = json["player_entities"];
    m_familiar_entities = json["familiar_entities"];
    m_package_entities = json["package_entities"];

    //mono::UniformRandomBitGenerator random_bit_generator(System::GetMilliseconds());
    //std::shuffle(m_player_entities.begin(), m_player_entities.end(), random_bit_generator);
    //std::shuffle(m_familiar_entities.begin(), m_familiar_entities.end(), random_bit_generator);
    //std::shuffle(m_package_entities.begin(), m_package_entities.end(), random_bit_generator);

    m_camera_system = m_system_context->GetSystem<CameraSystem>();

    using namespace std::placeholders;
    const event::ControllerAddedFunc& added_func = std::bind(&PlayerDaemonSystem::OnControllerAdded, this, _1);
    const event::ControllerRemovedFunc& removed_func = std::bind(&PlayerDaemonSystem::OnControllerRemoved, this, _1);

    m_added_token = m_event_handler->AddListener(added_func);
    m_removed_token = m_event_handler->AddListener(removed_func);

    const PlayerConnectedFunc& connected_func = std::bind(&PlayerDaemonSystem::RemotePlayerConnected, this, _1);
    const PlayerDisconnectedFunc& disconnected_func = std::bind(&PlayerDaemonSystem::RemotePlayerDisconnected, this, _1);
    const SpawnPlayerFunc& spawn_player_func = std::bind(&PlayerDaemonSystem::OnSpawnPlayer, this, _1);
    const DespawnPlayerFunc& despawn_player_func = std::bind(&PlayerDaemonSystem::OnDespawnPlayer, this, _1);
    const RespawnPlayerFunc& respawn_player_func = std::bind(&PlayerDaemonSystem::OnRespawnPlayer, this, _1);

    m_player_connected_token = m_event_handler->AddListener(connected_func);
    m_player_disconnected_token = m_event_handler->AddListener(disconnected_func);
    m_spawn_player_token = m_event_handler->AddListener(spawn_player_func);
    m_despawn_player_token = m_event_handler->AddListener(despawn_player_func);
    m_respawn_player_token = m_event_handler->AddListener(respawn_player_func);

    const RemoteInputMessageFunc& remote_input_func = std::bind(&PlayerDaemonSystem::RemotePlayerInput, this, _1);
    const ViewportMessageFunc& remote_viewport_func = std::bind(&PlayerDaemonSystem::RemotePlayerViewport, this, _1);

    m_remote_input_token = m_event_handler->AddListener(remote_input_func);
    m_remote_viewport_token = m_event_handler->AddListener(remote_viewport_func);
}

PlayerDaemonSystem::~PlayerDaemonSystem()
{
    m_event_handler->RemoveListener(m_added_token);
    m_event_handler->RemoveListener(m_removed_token);

    m_event_handler->RemoveListener(m_player_connected_token);
    m_event_handler->RemoveListener(m_player_disconnected_token);
    m_event_handler->RemoveListener(m_spawn_player_token);
    m_event_handler->RemoveListener(m_despawn_player_token);
    m_event_handler->RemoveListener(m_respawn_player_token);
    m_event_handler->RemoveListener(m_remote_input_token);
    m_event_handler->RemoveListener(m_remote_viewport_token);
}

const char* PlayerDaemonSystem::Name() const
{
    return "playerdaemonsystem";
}

void PlayerDaemonSystem::Update(const mono::UpdateContext& update_context)
{

}

void PlayerDaemonSystem::Begin()
{
    SpawnLocalPlayer(game::ANY_PLAYER_INFO, System::GetControllerId(System::ControllerId::Primary));

    if(System::IsControllerActive(System::ControllerId::Secondary))
        SpawnLocalPlayer(game::ANY_PLAYER_INFO, System::GetControllerId(System::ControllerId::Secondary));

    if(m_spawn_players)
        m_spawned_player_familiar = SpawnPlayerFamiliar(m_entity_system, m_system_context, m_event_handler);
}

void PlayerDaemonSystem::Reset()
{
    for(int index = 0; index < game::n_players; ++index)
    {
        game::PlayerInfo& player_info = g_players[index];
        if(player_info.player_state != game::PlayerState::NOT_SPAWNED)
            DespawnPlayer(&player_info);
    
        m_camera_system->Unfollow(player_info.entity_id);
    }

    if(m_spawn_players)
        m_entity_system->ReleaseEntity(m_spawned_player_familiar);

    m_player_spawned_callback = nullptr;
    m_spawn_players = false;
}

void PlayerDaemonSystem::SpawnPlayersAt(const math::Vector& spawn_position, const PlayerSpawnedCallback& callback)
{
    m_spawn_players = true;
    m_player_spawn = spawn_position;
    m_player_spawned_callback = callback;
}

uint32_t PlayerDaemonSystem::SpawnPackageAt(const math::Vector& spawn_position)
{
    const std::string& package_entity_file = m_package_entities.front();
    const mono::Entity package_entity = m_entity_system->CreateEntity(package_entity_file.c_str());

    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    transform_system->SetTransform(package_entity.id, math::CreateMatrixWithPosition(spawn_position));
    transform_system->SetTransformState(package_entity.id, mono::TransformState::CLIENT);

    m_entity_system->AddComponent(package_entity.id, BEHAVIOUR_COMPONENT);

    game::EntityLogicSystem* logic_system = m_system_context->GetSystem<EntityLogicSystem>();
    logic_system->AddLogic(package_entity.id, new PackageLogic(package_entity.id, &g_package_info, m_event_handler, m_system_context));

    game::g_package_info.entity_id = package_entity.id;
    game::g_package_info.state = PackageState::SPAWNED;

    const mono::ReleaseCallback release_callback = [](uint32_t entity_id) {
        game::g_package_info.entity_id = mono::INVALID_ID;
        game::g_package_info.state = PackageState::NOT_SPAWNED;
    };
    m_entity_system->AddReleaseCallback(package_entity.id, release_callback);

    return package_entity.id;
}

std::vector<uint32_t> PlayerDaemonSystem::GetPlayerIds() const
{
    std::vector<uint32_t> ids;
    for(const PlayerInfo* player : game::GetActivePlayers())
    {
        if(player)
            ids.push_back(player->entity_id);
    }

    return ids;
}

void PlayerDaemonSystem::SpawnLocalPlayer(int player_index, int controller_id)
{
    if(!m_spawn_players)
        return;

    game::PlayerInfo* allocated_player_info = AllocatePlayerInfo(player_index);
    if(!allocated_player_info)
    {
        System::Log("Unable to allocate player info for local player.");
        return;
    }

    allocated_player_info->controller_id = controller_id;
    m_controller_id_to_player_info[controller_id] = allocated_player_info;

    const auto destroyed_func = [this, allocated_player_info](uint32_t entity_id, int damage, uint32_t id_who_did_damage, DamageType type) {

        if(type == DamageType::DESTROYED)
        {
            m_camera_system->Unfollow(entity_id);
            allocated_player_info->player_state = game::PlayerState::DEAD;
            allocated_player_info->killer_entity_id = id_who_did_damage;
        }
        else if(type == DamageType::DAMAGED)
        {
            m_camera_system->AddCameraShake(0.25f);
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
    
    m_camera_system->FollowEntity(spawned_id);
}

void PlayerDaemonSystem::DespawnPlayer(PlayerInfo* player_info)
{
    m_camera_system->Unfollow(player_info->entity_id);
    m_entity_system->ReleaseEntity(player_info->entity_id);
    ReleasePlayerInfo(player_info);

/*
    const auto is_null = [](const game::PlayerInfo* player_info) {
        return player_info == nullptr;
    };

    const game::PlayerArray active_players = GetActivePlayers();
    const bool all_players_dead = std::all_of(active_players.begin(), active_players.end(), is_null);
    if(all_players_dead)
        m_event_handler->DispatchEvent(game::GameOverEvent());
*/
}

uint32_t PlayerDaemonSystem::SpawnPlayer(
    game::PlayerInfo* player_info,
    const math::Vector& spawn_position,
    const System::ControllerState& controller,
    mono::IEntityManager* entity_system,
    mono::SystemContext* system_context,
    mono::EventHandler* event_handler,
    const game::DamageCallback& damage_callback)
{
    const uint32_t player_index = game::FindPlayerIndex(player_info);
    const std::string player_entity_file = m_player_entities[player_index];
    mono::Entity player_entity = entity_system->CreateEntity(player_entity_file.c_str());

    mono::TransformSystem* transform_system = system_context->GetSystem<mono::TransformSystem>();
    transform_system->SetTransform(player_entity.id, math::CreateMatrixWithPosition(spawn_position));
    transform_system->SetTransformState(player_entity.id, mono::TransformState::CLIENT);

    // No need to store the callback id, when destroyed this callback will be cleared up.
    game::DamageSystem* damage_system = system_context->GetSystem<DamageSystem>();
    damage_system->PreventReleaseOnDeath(player_entity.id, true);
    const uint32_t callback_id = damage_system->SetDamageCallback(player_entity.id, DamageType::DT_ALL, damage_callback);
    (void)callback_id;

    game::EntityLogicSystem* logic_system = system_context->GetSystem<EntityLogicSystem>();
    entity_system->AddComponent(player_entity.id, BEHAVIOUR_COMPONENT);

    mono::InputSystem* input_system = system_context->GetSystem<mono::InputSystem>();

    IEntityLogic* player_logic = new PlayerLogic(player_entity.id, player_info, input_system, event_handler, controller, system_context);
    logic_system->AddLogic(player_entity.id, player_logic);

    player_info->entity_id = player_entity.id;
    player_info->player_state = game::PlayerState::ALIVE;

    if(m_player_spawned_callback)
        m_player_spawned_callback(game::PlayerSpawnState::SPAWNED, player_entity.id, spawn_position);

    return player_entity.id;
}

uint32_t PlayerDaemonSystem::SpawnPlayerFamiliar(
    mono::IEntityManager* entity_system, mono::SystemContext* system_context, mono::EventHandler* event_handler)
{
    const std::string familiar_entity_file = m_familiar_entities.front();
    mono::Entity player_familiar_entity = entity_system->CreateEntity(familiar_entity_file.c_str());

    game::EntityLogicSystem* logic_system = system_context->GetSystem<EntityLogicSystem>();
    entity_system->AddComponent(player_familiar_entity.id, BEHAVIOUR_COMPONENT);

    IEntityLogic* player_logic = new PlayerFamiliarLogic(player_familiar_entity.id, event_handler, system_context);
    logic_system->AddLogic(player_familiar_entity.id, player_logic);

    return player_familiar_entity.id;
}

mono::EventResult PlayerDaemonSystem::OnControllerAdded(const event::ControllerAddedEvent& event)
{
    SpawnLocalPlayer(game::ANY_PLAYER_INFO, event.controller_id);
    return mono::EventResult::PASS_ON;
}

mono::EventResult PlayerDaemonSystem::OnControllerRemoved(const event::ControllerRemovedEvent& event)
{
    const auto it = m_controller_id_to_player_info.find(event.controller_id);
    if(it != m_controller_id_to_player_info.end())
    {
        DespawnPlayer(it->second);
        m_controller_id_to_player_info.erase(event.controller_id);
    }

    return mono::EventResult::PASS_ON;
}

mono::EventResult PlayerDaemonSystem::RemotePlayerConnected(const PlayerConnectedEvent& event)
{
    auto it = m_remote_players.find(event.address);
    if(it != m_remote_players.end())
        return mono::EventResult::HANDLED;

    System::Log("PlayerDaemonSystem|Remote player connected, %s", network::AddressToString(event.address).c_str());

    game::PlayerInfo* allocated_player_info = AllocatePlayerInfo();
    if(!allocated_player_info)
    {
        System::Log("Unable to allocate player info for local player.");
        return mono::EventResult::HANDLED;
    }

    PlayerDaemonSystem::RemotePlayerData& remote_player_data = m_remote_players[event.address];
    remote_player_data.player_info = allocated_player_info;

    const auto remote_player_destroyed = [this, allocated_player_info](uint32_t entity_id, int damage, uint32_t id_who_did_damage, DamageType type)
    {
        m_camera_system->Unfollow(allocated_player_info->entity_id);
        allocated_player_info->player_state = game::PlayerState::DEAD;
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

mono::EventResult PlayerDaemonSystem::RemotePlayerDisconnected(const PlayerDisconnectedEvent& event)
{
    auto it = m_remote_players.find(event.address);
    if(it != m_remote_players.end())
    {
        DespawnPlayer(it->second.player_info);
        m_remote_players.erase(it);
    }

    return mono::EventResult::HANDLED;
}

mono::EventResult PlayerDaemonSystem::RemotePlayerInput(const RemoteInputMessage& event)
{
    auto it = m_remote_players.find(event.sender);
    if(it != m_remote_players.end())
        it->second.controller_state = event.controller_state;

    return mono::EventResult::HANDLED;
}

mono::EventResult PlayerDaemonSystem::RemotePlayerViewport(const ViewportMessage& message)
{
    auto it = m_remote_players.find(message.sender);
    if(it != m_remote_players.end())
        it->second.player_info->viewport = message.viewport;

    return mono::EventResult::PASS_ON;
}

mono::EventResult PlayerDaemonSystem::OnSpawnPlayer(const SpawnPlayerEvent& event)
{
    SpawnLocalPlayer(event.player_index, event.player_index);
    return mono::EventResult::HANDLED;
}

mono::EventResult PlayerDaemonSystem::OnDespawnPlayer(const DespawnPlayerEvent& event)
{
    DespawnPlayer(&g_players[event.player_index]);
    return mono::EventResult::HANDLED;
}

mono::EventResult PlayerDaemonSystem::OnRespawnPlayer(const RespawnPlayerEvent& event)
{
    game::PlayerInfo* player_info = game::FindPlayerInfoFromEntityId(event.entity_id);
    if(player_info)
    {
        math::Vector spawn_point = m_player_spawn;

        for(const game::PlayerInfo* alive_player_info : game::GetAlivePlayers())
        {
            if(alive_player_info)
            {
                spawn_point = alive_player_info->position + math::Vector(2.0f, 1.0f);
                break;
            }
        }

        mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
        math::Matrix& transform = transform_system->GetTransform(event.entity_id);
        math::Position(transform, spawn_point);
        transform_system->SetTransformState(event.entity_id, mono::TransformState::CLIENT);

        game::DamageSystem* damage_system = m_system_context->GetSystem<game::DamageSystem>();
        damage_system->ReactivateDamageRecord(event.entity_id);

        player_info->player_state = game::PlayerState::ALIVE;

        m_camera_system->FollowEntity(event.entity_id);

        if(m_player_spawned_callback)
            m_player_spawned_callback(game::PlayerSpawnState::RESPAWNED, player_info->entity_id, spawn_point);
    }

    return mono::EventResult::HANDLED;
}
