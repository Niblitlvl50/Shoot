
#include "PlayerDaemonSystem.h"
#include "Player/PlayerLogic.h"
#include "Player/PackageLogic.h"
#include "Player/PlayerInfo.h"
#include "Player/SaveSystem.h"
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
#include "System/File.h"
#include "Entity/Component.h"

#include "nlohmann/json.hpp"
#include <functional>

using namespace game;

PlayerDaemonSystem::PlayerDaemonSystem(
    INetworkPipe* remote_connection,
    mono::IEntityManager* entity_system,
    mono::SystemContext* system_context,
    mono::EventHandler* event_handler,
    game::CameraSystem* camera_system,
    game::DamageSystem* damage_system)
    : m_remote_connection(remote_connection)
    , m_entity_system(entity_system)
    , m_system_context(system_context)
    , m_event_handler(event_handler)
    , m_camera_system(camera_system)
    , m_damage_system(damage_system)
    , m_spawn_players(false)
{
    const std::vector<byte> file_data = file::FileReadAll("res/configs/player_config.json");
    const nlohmann::json& json = nlohmann::json::parse(file_data);

    m_player_entities = json["player_entities"];
    m_familiar_entities = json["familiar_entities"];
    m_package_entities = json["package_entities"];
    m_decoy_entities = json["decoy_entities"];
    m_weapon_entities = json["weapon_entities"];

    m_player_levels = json.value("player_levels", std::vector<int>());

    const std::vector<std::string> player_damage_sounds = json["player_damage_sounds"];
    const std::vector<std::string> player_death_sounds = json["player_death_sounds"];

    for(const std::string& sound_file : player_damage_sounds)
        m_damage_sounds.push_back(audio::CreateSound(sound_file.c_str(), audio::SoundPlayback::ONCE));

    for(const std::string& sound_file : player_death_sounds)
        m_death_sounds.push_back(audio::CreateSound(sound_file.c_str(), audio::SoundPlayback::ONCE));

    mono::UniformRandomBitGenerator random_bit_generator(System::GetMilliseconds());
    //std::shuffle(m_player_entities.begin(), m_player_entities.end(), random_bit_generator);
    std::shuffle(m_familiar_entities.begin(), m_familiar_entities.end(), random_bit_generator);
    //std::shuffle(m_package_entities.begin(), m_package_entities.end(), random_bit_generator);

    game::LoadPlayerData(System::GetUserPath(), 0, m_save_slot_0);

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
    const game::DamageCallback collect_weapon_experience = [](uint32_t damaged_entity_id, uint32_t who_did_damage, uint32_t weapon_identifier, int damage, DamageType type) {
        const bool is_player = IsPlayer(who_did_damage);
        if(!is_player)
            return;

        game::PlayerInfo* player_info = game::FindPlayerInfoFromEntityId(who_did_damage);
        if(!player_info)
            return;

        int& weapon_exp = player_info->persistent_data.weapon_experience[weapon_identifier];
        weapon_exp += damage;
    };
    m_collect_weapon_exp_id = m_damage_system->SetGlobalDamageCallback(DamageType::DESTROYED, collect_weapon_experience);

    SpawnLocalPlayer(game::ANY_PLAYER_INFO, System::ControllerId::Primary);

    if(System::IsControllerActive(System::ControllerId::Secondary))
        SpawnLocalPlayer(game::ANY_PLAYER_INFO, System::ControllerId::Secondary);
}

void PlayerDaemonSystem::Reset()
{
    m_damage_system->RemoveGlobalDamageCallback(m_collect_weapon_exp_id);
    
    for(int index = 0; index < game::n_players; ++index)
    {
        game::PlayerInfo& player_info = g_players[index];
        if(player_info.player_state != game::PlayerState::NOT_SPAWNED)
            DespawnPlayer(&player_info);
    
        m_camera_system->Unfollow(player_info.entity_id);
    }

    game::SavePlayerData(System::GetUserPath(), 0, m_save_slot_0);

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
    const mono::Entity package_entity = m_entity_system->SpawnEntity(package_entity_file.c_str());

    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    transform_system->SetTransform(package_entity.id, math::CreateMatrixWithPosition(spawn_position), mono::TransformState::CLIENT);

    m_entity_system->AddComponent(package_entity.id, BEHAVIOUR_COMPONENT);

    game::EntityLogicSystem* logic_system = m_system_context->GetSystem<EntityLogicSystem>();
    logic_system->AddLogic(package_entity.id, new PackageLogic(package_entity.id, &g_package_info, m_event_handler, m_system_context));

    game::g_package_info.entity_id = package_entity.id;
    game::g_package_info.state = PackageState::SPAWNED;

    const mono::ReleaseCallback release_callback = [](uint32_t entity_id, mono::ReleasePhase phase) {
        game::g_package_info.entity_id = mono::INVALID_ID;
        game::g_package_info.state = PackageState::NOT_SPAWNED;
    };
    m_entity_system->AddReleaseCallback(package_entity.id, mono::ReleasePhase::POST_RELEASE, release_callback);

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

void PlayerDaemonSystem::SpawnLocalPlayer(int player_index, System::ControllerId controller_id)
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

    // Apply saved data
    const uint32_t actual_player_index = game::FindPlayerIndex(allocated_player_info);
    allocated_player_info->persistent_data = m_save_slot_0.player_data[actual_player_index];

    const game::DamageCallback destroyed_func = [this, allocated_player_info](uint32_t damaged_entity_id, uint32_t id_who_did_damage, uint32_t weapon_identifier, int damage, DamageType type) {

        if(type == DamageType::DESTROYED)
        {
            m_camera_system->Unfollow(damaged_entity_id);
            allocated_player_info->player_state = game::PlayerState::DEAD;
            allocated_player_info->killer_entity_id = id_who_did_damage;

            if(!m_death_sounds.empty())
            {
                const int random_index = mono::RandomInt(0, m_death_sounds.size() -1);
                m_death_sounds[random_index]->Play();
            }

            const bool any_player_alive = game::IsAnyPlayerAlive();
            if(!any_player_alive)
                m_event_handler->DispatchEvent(game::GameOverEvent());
        }
        else if(type == DamageType::DAMAGED)
        {
            m_camera_system->AddCameraShake(0.25f);

            if(!m_damage_sounds.empty())
            {
                const int random_index = mono::RandomInt(0, m_damage_sounds.size() -1);
                m_damage_sounds[random_index]->Play();
            }
        }
    };

    const uint32_t spawned_id = SpawnPlayer(
        allocated_player_info,
        m_player_spawn,
        m_entity_system,
        m_system_context,
        m_event_handler,
        destroyed_func);
    
    m_camera_system->FollowEntity(spawned_id);

    allocated_player_info->familiar_entity_id = SpawnPlayerFamiliar(spawned_id, actual_player_index, m_entity_system, m_system_context);
}

void PlayerDaemonSystem::DespawnPlayer(PlayerInfo* player_info)
{
    m_camera_system->Unfollow(player_info->entity_id);
    m_entity_system->ReleaseEntity(player_info->entity_id);
    m_entity_system->ReleaseEntity(player_info->familiar_entity_id);

    const uint32_t player_index = game::FindPlayerIndex(player_info);
    m_save_slot_0.player_data[player_index] = player_info->persistent_data;

    ReleasePlayerInfo(player_info);
}

uint32_t PlayerDaemonSystem::SpawnPlayer(
    game::PlayerInfo* player_info,
    const math::Vector& spawn_position,
    mono::IEntityManager* entity_system,
    mono::SystemContext* system_context,
    mono::EventHandler* event_handler,
    const game::DamageCallback& damage_callback)
{
    const uint32_t player_index = game::FindPlayerIndex(player_info);
    const std::string player_entity_file = m_player_entities[player_index];
    mono::Entity player_entity = entity_system->SpawnEntity(player_entity_file.c_str());

    System::Log(
        "PlayerDaemonSystem|Spawned player with index '%u' and id '%u' at %.2f %.2f.",
        player_index,
        player_entity.id,
        spawn_position.x,
        spawn_position.y);

    mono::TransformSystem* transform_system = system_context->GetSystem<mono::TransformSystem>();
    transform_system->SetTransform(player_entity.id, math::CreateMatrixWithPosition(spawn_position));
    transform_system->SetTransformState(player_entity.id, mono::TransformState::CLIENT);

    DamageRecord* damage_record = m_damage_system->GetDamageRecord(player_entity.id);
    damage_record->release_entity_on_death = false;

    // No need to store the callback id, when destroyed this callback will be cleared up.
    const uint32_t callback_id = m_damage_system->SetDamageCallback(player_entity.id, DamageType::DT_ALL, damage_callback);
    (void)callback_id;

    m_damage_system->SetInvincible(player_entity.id, player_info->persistent_data.god_mode);
    m_damage_system->SetDamageMultiplier(player_entity.id, player_info->persistent_data.damage_multiplier);

    game::EntityLogicSystem* logic_system = system_context->GetSystem<EntityLogicSystem>();
    entity_system->AddComponent(player_entity.id, BEHAVIOUR_COMPONENT);

    mono::InputSystem* input_system = system_context->GetSystem<mono::InputSystem>();

    PlayerConfig player_config;
    player_config.decoy_entity = m_decoy_entities.front();
    player_config.weapon_entity = m_weapon_entities.front();
    player_config.player_levels = m_player_levels;

    IEntityLogic* player_logic = new PlayerLogic(player_entity.id, player_info, player_config, input_system, event_handler, system_context);
    logic_system->AddLogic(player_entity.id, player_logic);

    player_info->entity_id = player_entity.id;
    player_info->player_state = game::PlayerState::ALIVE;

    if(m_player_spawned_callback)
        m_player_spawned_callback(game::PlayerSpawnState::SPAWNED, player_entity.id, spawn_position);

    return player_entity.id;
}

uint32_t PlayerDaemonSystem::SpawnPlayerFamiliar(
    uint32_t owner_entity_id, uint32_t player_index, mono::IEntityManager* entity_system, mono::SystemContext* system_context)
{
    const std::string familiar_entity_file = m_familiar_entities[player_index];
    mono::Entity player_familiar_entity = entity_system->SpawnEntity(familiar_entity_file.c_str());

    game::EntityLogicSystem* logic_system = system_context->GetSystem<EntityLogicSystem>();
    entity_system->AddComponent(player_familiar_entity.id, BEHAVIOUR_COMPONENT);

    IEntityLogic* player_logic = new PlayerFamiliarLogic(player_familiar_entity.id, owner_entity_id, system_context);
    logic_system->AddLogic(player_familiar_entity.id, player_logic);

    return player_familiar_entity.id;
}

mono::EventResult PlayerDaemonSystem::OnControllerAdded(const event::ControllerAddedEvent& event)
{
    const auto it = m_controller_id_to_player_info.find(System::ControllerId(event.controller_id));
    if(it == m_controller_id_to_player_info.end())
        SpawnLocalPlayer(game::ANY_PLAYER_INFO, System::ControllerId(event.controller_id));

    return mono::EventResult::PASS_ON;
}

mono::EventResult PlayerDaemonSystem::OnControllerRemoved(const event::ControllerRemovedEvent& event)
{
    const auto it = m_controller_id_to_player_info.find(System::ControllerId(event.controller_id));
    if(it != m_controller_id_to_player_info.end())
    {
        DespawnPlayer(it->second);
        m_controller_id_to_player_info.erase(System::ControllerId(event.controller_id));
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

    const game::DamageCallback remote_player_destroyed = [this, allocated_player_info](uint32_t damaged_entity_id, uint32_t id_who_did_damage, uint32_t weapon_identifier, int damage, DamageType type)
    {
        m_camera_system->Unfollow(allocated_player_info->entity_id);
        allocated_player_info->player_state = game::PlayerState::DEAD;
    };

    const uint32_t spawned_id = SpawnPlayer(
        remote_player_data.player_info,
        m_player_spawn,
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
    SpawnLocalPlayer(event.player_index, System::ControllerId(event.player_index));
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

        m_damage_system->ReactivateDamageRecord(event.entity_id);

        player_info->player_state = game::PlayerState::ALIVE;

        m_camera_system->FollowEntity(event.entity_id);

        if(m_player_spawned_callback)
            m_player_spawned_callback(game::PlayerSpawnState::RESPAWNED, player_info->entity_id, spawn_point);
    }

    return mono::EventResult::HANDLED;
}
