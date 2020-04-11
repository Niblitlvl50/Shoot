
#include "PlayerDaemon.h"
#include "PlayerLogic.h"
#include "RenderLayers.h"
#include "AIKnowledge.h"
#include "FontIds.h"

#include "Factories.h"
#include "EntitySystem/Entity.h"
#include "Entity/IEntityManager.h"
#include "UpdateTasks/CameraViewportReporter.h"

#include "SystemContext.h"
#include "DamageSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Entity/EntityLogicSystem.h"

#include "EventHandler/EventHandler.h"
#include "Events/ControllerEvent.h"
#include "Events/QuitEvent.h"
#include "Events/EventFuncFwd.h"
#include "Events/PlayerConnectedEvent.h"

#include "Camera/ICamera.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "System/System.h"
#include "Zone/EntityBase.h"
#include "Hud/UIElements.h"
#include "Hud/Dialog.h"

#include "Network/NetworkMessage.h"
#include "Network/INetworkPipe.h"
#include "Network/NetworkSerialize.h"

#include "Component.h"
#include "System/System.h"

#include <functional>

using namespace game;

#define IS_TRIGGERED(variable) (!m_last_state.variable && state.variable)
#define HAS_CHANGED(variable) (m_last_state.variable != state.variable)

namespace
{
    class PlayerDeathScreen : public mono::EntityBase
    {
    public:
        PlayerDeathScreen(PlayerDaemon* player_daemon, mono::EventHandler& event_handler, const math::Vector& position)
            : m_player_daemon(player_daemon), m_event_handler(event_handler)
        {
            m_position = position;

            const std::vector<UIDialog::Option> options = {
                { "Respawn",    "res/sprites/ps_cross.sprite",      0.35f },
                { "Quit",       "res/sprites/ps_triangle.sprite",   0.35f }
            };

            constexpr mono::Color::RGBA background_color(0, 0, 0);
            constexpr mono::Color::RGBA text_color(1, 0, 0);

            AddChild(std::make_shared<UIDialog>("YOU DEAD! Respawn?", options, background_color, text_color));
        }

        void Update(const mono::UpdateContext& update_context)
        {
            const System::ControllerState& state = System::GetController(System::ControllerId::Primary);

            const bool a_pressed = IS_TRIGGERED(a) && HAS_CHANGED(a);
            const bool y_pressed = IS_TRIGGERED(y) && HAS_CHANGED(y);

            if(a_pressed)
            {
                m_player_daemon->SpawnPlayer1();
                //m_event_handler.DispatchEvent(RemoveEntityEvent(Id()));
            }
            else if(y_pressed)
            {
                m_event_handler.DispatchEvent(event::QuitEvent());
            }

            m_last_state = state;
        }

        void Draw(mono::IRenderer& renderer) const
        { }

        PlayerDaemon* m_player_daemon;
        mono::EventHandler& m_event_handler;
        System::ControllerState m_last_state;
    };
}

PlayerDaemon::PlayerDaemon(
    GameCamera* game_camera,
    INetworkPipe* remote_connection,
    mono::SystemContext* system_context,
    mono::EventHandler& event_handler)
    : m_game_camera(game_camera)
    , m_remote_connection(remote_connection)
    , m_system_context(system_context)
    , m_event_handler(event_handler)
{
    using namespace std::placeholders;

    const event::ControllerAddedFunc& added_func = std::bind(&PlayerDaemon::OnControllerAdded, this, _1);
    const event::ControllerRemovedFunc& removed_func = std::bind(&PlayerDaemon::OnControllerRemoved, this, _1);

    //m_added_token = m_event_handler.AddListener(added_func);
    //m_removed_token = m_event_handler.AddListener(removed_func);

    const PlayerConnectedFunc& connected_func = std::bind(&PlayerDaemon::PlayerConnected, this, _1);
    const PlayerDisconnectedFunc& disconnected_func = std::bind(&PlayerDaemon::PlayerDisconnected, this, _1);

    m_player_connected_token = m_event_handler.AddListener(connected_func);
    m_player_disconnected_token = m_event_handler.AddListener(disconnected_func);

    const std::function<mono::EventResult (const RemoteInputMessage&)>& remote_input_func = std::bind(&PlayerDaemon::RemoteInput, this, _1);
    m_remote_input_token = m_event_handler.AddListener(remote_input_func);

    if(System::IsControllerActive(System::ControllerId::Primary))
    {
        m_player_one_id = System::GetControllerId(System::ControllerId::Primary);
        SpawnPlayer1();
    }

    if(System::IsControllerActive(System::ControllerId::Secondary))
    {
        m_player_two_id = System::GetControllerId(System::ControllerId::Secondary);
        SpawnPlayer2();
    }
}

PlayerDaemon::~PlayerDaemon()
{
    m_event_handler.RemoveListener(m_added_token);
    m_event_handler.RemoveListener(m_removed_token);

    m_event_handler.RemoveListener(m_player_connected_token);
    m_event_handler.RemoveListener(m_player_disconnected_token);
    m_event_handler.RemoveListener(m_remote_input_token);

    if(g_player_one.is_active)
        game::g_entity_manager->ReleaseEntity(g_player_one.entity_id);

    if(g_player_two.is_active)
        game::g_entity_manager->ReleaseEntity(g_player_two.entity_id);
}

std::vector<uint32_t> PlayerDaemon::GetPlayerIds() const
{
    std::vector<uint32_t> ids;

    if(g_player_one.is_active)
        ids.push_back(g_player_one.entity_id);

    for(const auto& pair : m_remote_players)
        ids.push_back(pair.second.player_info.entity_id);

    return ids;
}

void PlayerDaemon::SpawnPlayer(
    game::PlayerInfo* player_info, const System::ControllerState& controller, DestroyedCallback destroyed_callback)
{
    mono::Entity player_entity = game::g_entity_manager->CreateEntity("res/entities/player_entity.entity");

    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    math::Matrix& transform = transform_system->GetTransform(player_entity.id);
    math::Position(transform, math::ZeroVec);

    game::DamageSystem* damage_system = m_system_context->GetSystem<DamageSystem>();

    // No need to store the callback id, when destroyed this callback will be cleared up.
    const uint32_t callback_id = damage_system->SetDestroyedCallback(player_entity.id, destroyed_callback);
    (void)callback_id;

    game::EntityLogicSystem* logic_system = m_system_context->GetSystem<EntityLogicSystem>();
    game::g_entity_manager->AddComponent(player_entity.id, BEHAVIOUR_COMPONENT);

    IEntityLogic* player_logic = new PlayerLogic(player_entity.id, player_info, m_event_handler, controller, m_system_context);
    logic_system->AddLogic(player_entity.id, player_logic);

    m_game_camera->Follow(player_entity.id, math::ZeroVec);

    player_info->entity_id = player_entity.id;
    player_info->is_active = true;
}

void PlayerDaemon::SpawnPlayer1()
{
    const auto destroyed_func = [](uint32_t entity_id) {
        game::g_player_one.is_active = false;
        //auto player_death_ui = std::make_shared<PlayerDeathScreen>(this, m_event_handler, game::g_player_one.position);
        //m_event_handler.DispatchEvent(SpawnEntityEvent(player_death_ui, LayerId::UI));
    };

    SpawnPlayer(&game::g_player_one, System::GetController(System::ControllerId::Primary), destroyed_func);
}

void PlayerDaemon::SpawnPlayer2()
{
    const auto destroyed_func = [](uint32_t entity_id) {
        game::g_player_two.is_active = false;
    };

    SpawnPlayer(&game::g_player_two, System::GetController(System::ControllerId::Primary), destroyed_func);
}

mono::EventResult PlayerDaemon::OnControllerAdded(const event::ControllerAddedEvent& event)
{
    if(!game::g_player_one.is_active)
    {
        SpawnPlayer1();
        m_player_one_id = event.id;
    }
    else
    {
        SpawnPlayer2();
        m_player_two_id = event.id;
    }

    return mono::EventResult::PASS_ON;
}

mono::EventResult PlayerDaemon::OnControllerRemoved(const event::ControllerRemovedEvent& event)
{
    if(event.id == m_player_one_id)
    {
        if(game::g_player_one.is_active)
            g_entity_manager->ReleaseEntity(game::g_player_one.entity_id);

        m_game_camera->Unfollow();
        game::g_player_one.entity_id = mono::INVALID_ID;
        game::g_player_one.is_active = false;
    }
    else if(event.id == m_player_two_id)
    {
        if(game::g_player_two.is_active)
            g_entity_manager->ReleaseEntity(game::g_player_two.entity_id);

        game::g_player_two.entity_id = mono::INVALID_ID;
        game::g_player_two.is_active = false;
    }

    return mono::EventResult::PASS_ON;
}

mono::EventResult PlayerDaemon::PlayerConnected(const PlayerConnectedEvent& event)
{
    auto it = m_remote_players.find(event.address);
    if(it != m_remote_players.end())
        return mono::EventResult::HANDLED;

    System::Log("PlayerDaemon|Player connected, %s\n", network::AddressToString(event.address).c_str());

    PlayerDaemon::RemotePlayerData& remote_player_data = m_remote_players[event.address];

    const auto remote_player_destroyed = [](uint32_t entity_id) {
        //game::entity_manager->ReleaseEntity(it->second.player_info.entity_id);
        //m_remote_players.erase(it);
    };

    SpawnPlayer(&remote_player_data.player_info, remote_player_data.controller_state, remote_player_destroyed);

    ClientPlayerSpawned client_spawned_message;
    client_spawned_message.client_entity_id = remote_player_data.player_info.entity_id;

    NetworkMessage reply_message;
    reply_message.payload = SerializeMessage(client_spawned_message);

    m_remote_connection->SendMessageTo(reply_message, event.address);

    return mono::EventResult::HANDLED;
}

mono::EventResult PlayerDaemon::PlayerDisconnected(const PlayerDisconnectedEvent& event)
{
    auto it = m_remote_players.find(event.address);
    if(it != m_remote_players.end())
    {
        game::g_entity_manager->ReleaseEntity(it->second.player_info.entity_id);
        m_remote_players.erase(it);
    }

    return mono::EventResult::HANDLED;
}

mono::EventResult PlayerDaemon::RemoteInput(const RemoteInputMessage& event)
{
    auto it = m_remote_players.find(event.sender);
    if(it != m_remote_players.end())
        it->second.controller_state = event.controller_state;

    return mono::EventResult::HANDLED;
}


ClientPlayerDaemon::ClientPlayerDaemon(GameCamera* game_camera, mono::EventHandler& event_handler)
    : m_game_camera(game_camera)
    , m_event_handler(event_handler)
{
    using namespace std::placeholders;

    const event::ControllerAddedFunc& added_func = std::bind(&ClientPlayerDaemon::OnControllerAdded, this, _1);
    const event::ControllerRemovedFunc& removed_func = std::bind(&ClientPlayerDaemon::OnControllerRemoved, this, _1);
    const std::function<mono::EventResult (const ClientPlayerSpawned&)>& client_spawned = std::bind(&ClientPlayerDaemon::ClientSpawned, this, _1);

    m_added_token = m_event_handler.AddListener(added_func);
    m_removed_token = m_event_handler.AddListener(removed_func);
    m_client_spawned_token = m_event_handler.AddListener(client_spawned);

    if(System::IsControllerActive(System::ControllerId::Primary))
    {
        m_player_one_controller_id = System::GetControllerId(System::ControllerId::Primary);
        SpawnPlayer1();
    }
}

ClientPlayerDaemon::~ClientPlayerDaemon()
{
    m_event_handler.RemoveListener(m_added_token);
    m_event_handler.RemoveListener(m_removed_token);
    m_event_handler.RemoveListener(m_client_spawned_token);

    m_game_camera->Unfollow();
}

void ClientPlayerDaemon::SpawnPlayer1()
{
    System::Log("PlayerDaemon|Spawn player 1\n");
    //game::g_player_one.entity_id = player_entity.id;
    //game::g_player_one.is_active = true;
}

mono::EventResult ClientPlayerDaemon::OnControllerAdded(const event::ControllerAddedEvent& event)
{
    if(!game::g_player_one.is_active)
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
        game::g_player_one.is_active = false;
        m_game_camera->Unfollow();
    }

    return mono::EventResult::PASS_ON;
}

mono::EventResult ClientPlayerDaemon::ClientSpawned(const ClientPlayerSpawned& message)
{
    game::g_player_one.entity_id = message.client_entity_id;
    game::g_player_one.is_active = true;

    m_game_camera->Follow(g_player_one.entity_id, math::ZeroVec);

    return mono::EventResult::PASS_ON;
}
