
#include "PlayerDaemon.h"
#include "Shuttle.h"
#include "ShuttleLogic.h"
#include "RenderLayers.h"
#include "Events/SpawnPhysicsEntityEvent.h"
#include "Events/SpawnEntityEvent.h"
#include "Events/RemoveEntityEvent.h"
#include "AIKnowledge.h"
#include "FontIds.h"

#include "Factories.h"
#include "Entity/IEntityManager.h"

#include "SystemContext.h"
#include "DamageSystem.h"
#include "TransformSystem.h"
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
#include "Entity/EntityBase.h"
#include "Hud/UIElements.h"
#include "Hud/Dialog.h"

#include "Network/NetworkMessage.h"

#include "Component.h"

#include <functional>
#include <limits>

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
                m_event_handler.DispatchEvent(RemoveEntityEvent(Id()));
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
    mono::ICameraPtr camera, const std::vector<math::Vector>& player_points, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_camera(camera)
    , m_player_points(player_points)
    , m_system_context(system_context)
    , m_event_handler(event_handler)
{
    using namespace std::placeholders;

    const event::ControllerAddedFunc& added_func = std::bind(&PlayerDaemon::OnControllerAdded, this, _1);
    const event::ControllerRemovedFunc& removed_func = std::bind(&PlayerDaemon::OnControllerRemoved, this, _1);

    m_added_token = m_event_handler.AddListener(added_func);
    m_removed_token = m_event_handler.AddListener(removed_func);

    const PlayerConnectedFunc& connected_func = std::bind(&PlayerDaemon::PlayerConnected, this, _1);
    const PlayerDisconnectedFunc& disconnected_func = std::bind(&PlayerDaemon::PlayerDisconnected, this, _1);

    m_player_connected_token = m_event_handler.AddListener(connected_func);
    m_player_disconnected_token = m_event_handler.AddListener(disconnected_func);

    const std::function<bool (const RemoteInputMessage&)>& remote_input_func = std::bind(&PlayerDaemon::RemoteInput, this, _1);
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
        game::entity_manager->ReleaseEntity(g_player_one.entity_id);

    if(g_player_two.is_active)
        game::entity_manager->ReleaseEntity(g_player_two.entity_id);
}

void PlayerDaemon::SpawnPlayer(
    game::PlayerInfo* player_info, const System::ControllerState& controller, DestroyedCallback destroyed_callback)
{
    mono::Entity player_entity = game::entity_manager->CreateEntity("res/entities/player_entity.entity");

    const math::Vector& spawn_point = m_player_points.empty() ? math::ZeroVec : m_player_points.front();
    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    math::Matrix& transform = transform_system->GetTransform(player_entity.id);
    math::Position(transform, spawn_point);

    game::DamageSystem* damage_system = m_system_context->GetSystem<DamageSystem>();
    damage_system->SetDestroyedCallback(player_entity.id, destroyed_callback);

    game::EntityLogicSystem* logic_system = m_system_context->GetSystem<EntityLogicSystem>();
    entity_manager->AddComponent(player_entity.id, BEHAVIOUR_COMPONENT);

    IEntityLogic* player_logic = new ShuttleLogic(player_entity.id, player_info, m_event_handler, controller, m_system_context);
    logic_system->AddLogic(player_entity.id, player_logic);

    m_camera->Follow(player_entity.id, math::ZeroVec);

    player_info->entity_id = player_entity.id;
    player_info->is_active = true;
}

void PlayerDaemon::SpawnPlayer1()
{
//    const auto destroyed_func = [this](uint32_t entity_id) {
//        game::g_player_one.is_active = false;
//        auto player_death_ui = std::make_shared<PlayerDeathScreen>(this, m_event_handler, game::g_player_one.position);
//        m_event_handler.DispatchEvent(SpawnEntityEvent(player_death_ui, LayerId::UI));
//    };
//
//    SpawnPlayer(&game::g_player_one, System::GetController(System::ControllerId::Primary), destroyed_func);
}

void PlayerDaemon::SpawnPlayer2()
{
    const auto destroyed_func = [](uint32_t entity_id) {
        game::g_player_two.is_active = false;
    };

    SpawnPlayer(&game::g_player_two, System::GetController(System::ControllerId::Primary), destroyed_func);
}

bool PlayerDaemon::OnControllerAdded(const event::ControllerAddedEvent& event)
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

    return false;
}

bool PlayerDaemon::OnControllerRemoved(const event::ControllerRemovedEvent& event)
{
    if(event.id == m_player_one_id)
    {
        if(game::g_player_one.is_active)
            entity_manager->ReleaseEntity(game::g_player_one.entity_id);

        m_camera->Unfollow();
        game::g_player_one.entity_id = std::numeric_limits<uint32_t>::max();
        game::g_player_one.is_active = false;
    }
    else if(event.id == m_player_two_id)
    {
        if(game::g_player_two.is_active)
            entity_manager->ReleaseEntity(game::g_player_two.entity_id);

        game::g_player_two.entity_id = std::numeric_limits<uint32_t>::max();
        game::g_player_two.is_active = false;
    }

    return false;
}

bool PlayerDaemon::PlayerConnected(const PlayerConnectedEvent& event)
{
    auto it = m_remote_players.find(event.id);
    if(it != m_remote_players.end())
        return true;

    std::printf("Player connected, %u\n", event.id);

    PlayerDaemon::RemotePlayerData& remote_player_data = m_remote_players[event.id];

    const auto remote_player_destroyed = [](uint32_t entity_id) {

    };

    SpawnPlayer(&remote_player_data.player_info, remote_player_data.controller_state, remote_player_destroyed);

    return true;
}

bool PlayerDaemon::PlayerDisconnected(const PlayerDisconnectedEvent& event)
{
    auto it = m_remote_players.find(event.id);
    if(it != m_remote_players.end())
    {
        game::entity_manager->ReleaseEntity(it->second.player_info.entity_id);
        m_remote_players.erase(it);
    }

    return true;
}

bool PlayerDaemon::RemoteInput(const RemoteInputMessage& event)
{
    std::printf("Got remote input! %u\n", event.id);
    
    auto it = m_remote_players.find(event.id);
    if(it != m_remote_players.end())
    {
        std::printf("Applying input! %f\n", event.controller_state.left_x);
        it->second.controller_state = event.controller_state;
    }

    return true;
}


ClientPlayerDaemon::ClientPlayerDaemon(mono::EventHandler& event_handler)
    : m_event_handler(event_handler)
{
    using namespace std::placeholders;

    const event::ControllerAddedFunc& added_func = std::bind(&ClientPlayerDaemon::OnControllerAdded, this, _1);
    const event::ControllerRemovedFunc& removed_func = std::bind(&ClientPlayerDaemon::OnControllerRemoved, this, _1);

    m_added_token = m_event_handler.AddListener(added_func);
    m_removed_token = m_event_handler.AddListener(removed_func);

    if(System::IsControllerActive(System::ControllerId::Primary))
    {
        m_player_one_id = System::GetControllerId(System::ControllerId::Primary);
        SpawnPlayer1();
    }
}

ClientPlayerDaemon::~ClientPlayerDaemon()
{
    m_event_handler.RemoveListener(m_added_token);
    m_event_handler.RemoveListener(m_removed_token);
}

void ClientPlayerDaemon::SpawnPlayer1()
{
    std::printf("Spawn player 1\n");
    //game::g_player_one.entity_id = player_entity.id;
    game::g_player_one.is_active = true;
}

bool ClientPlayerDaemon::OnControllerAdded(const event::ControllerAddedEvent& event)
{
    if(!game::g_player_one.is_active)
    {
        SpawnPlayer1();
        m_player_one_id = event.id;
    }

    return false;
}

bool ClientPlayerDaemon::OnControllerRemoved(const event::ControllerRemovedEvent& event)
{
    if(event.id == m_player_one_id)
    {
        game::g_player_one.entity_id = std::numeric_limits<uint32_t>::max();
        game::g_player_one.is_active = false;
    }

    return false;
}
