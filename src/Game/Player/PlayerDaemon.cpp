
#include "PlayerDaemon.h"
#include "Shuttle.h"
#include "RenderLayers.h"
#include "Events/SpawnPhysicsEntityEvent.h"
#include "Events/SpawnEntityEvent.h"
#include "Events/RemoveEntityEvent.h"
#include "AIKnowledge.h"
#include "FontIds.h"

#include "Factories.h"
#include "GameObjects/IGameObjectFactory.h"

#include "EventHandler/EventHandler.h"
#include "Events/ControllerEvent.h"
#include "Events/QuitEvent.h"
#include "Events/EventFuncFwd.h"

#include "Rendering/ICamera.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "System/System.h"
#include "Entity/EntityBase.h"
#include "Hud/UIElements.h"
#include "Hud/Dialog.h"

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
                { "Respawn",    "res/sprites/ps_cross.sprite" },
                { "Quit",       "res/sprites/ps_triangle.sprite" }
            };

            constexpr mono::Color::RGBA background_color(0, 0, 0);
            constexpr mono::Color::RGBA text_color(1, 0, 0);

            AddChild(std::make_shared<UIDialog>("YOU DEAD! Respawn?", options, background_color, text_color));
        }

        void Update(unsigned int delta)
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
    mono::ICameraPtr camera, const std::vector<math::Vector>& player_points, mono::EventHandler& event_handler)
    : m_camera(camera)
    , m_player_points(player_points)
    , m_event_handler(event_handler)
{
    using namespace std::placeholders;

    const event::ControllerAddedFunc& added_func = std::bind(&PlayerDaemon::OnControllerAdded, this, _1);
    const event::ControllerRemovedFunc& removed_func = std::bind(&PlayerDaemon::OnControllerRemoved, this, _1);

    m_added_token = m_event_handler.AddListener(added_func);
    m_removed_token = m_event_handler.AddListener(removed_func);

    if(System::IsControllerActive(System::ControllerId::Primary))
        SpawnPlayer1();

    if(System::IsControllerActive(System::ControllerId::Secondary))
        SpawnPlayer2();
}

PlayerDaemon::~PlayerDaemon()
{
    m_event_handler.RemoveListener(m_added_token);
    m_event_handler.RemoveListener(m_removed_token);
}

void PlayerDaemon::SpawnPlayer1()
{
    const auto destroyed_func = [this](unsigned int id) {
        game::g_player_one.is_active = false;
        const math::Vector death_position = m_player_one->Position();
        auto player_death_ui = std::make_shared<PlayerDeathScreen>(this, m_event_handler, death_position);
        m_event_handler.DispatchEvent(SpawnEntityEvent(player_death_ui, LayerId::UI));
    };

    const math::Vector& spawn_point = m_player_points.empty() ? math::ZeroVec : m_player_points.front();
    auto player_one = game::gameobject_factory->CreateShuttle(
        spawn_point, System::GetController(System::ControllerId::Primary), destroyed_func);

    m_player_one = std::static_pointer_cast<Shuttle>(player_one);
    m_player_one->SetPlayerInfo(&game::g_player_one);
    //m_player_one->SetShading(mono::Color::RGBA(0.5, 1.0f, 0.5f));

    game::g_player_one.is_active = true;

    m_camera->SetPosition(spawn_point);
    m_camera->Follow(m_player_one, math::ZeroVec);

    m_event_handler.DispatchEvent(SpawnPhysicsEntityEvent(m_player_one, LayerId::FOREGROUND));
}

void PlayerDaemon::SpawnPlayer2()
{
    const auto destroyed_func = [](unsigned int id) {
        game::g_player_two.is_active = false;
    };

    const math::Vector& spawn_point = m_player_points.empty() ? math::ZeroVec : m_player_points.front();
    auto player_two = game::gameobject_factory->CreateShuttle(
        spawn_point, System::GetController(System::ControllerId::Secondary), destroyed_func);

    m_player_two = std::static_pointer_cast<Shuttle>(player_two);
    m_player_two->SetPlayerInfo(&game::g_player_two);
    m_player_two->SetShading(mono::Color::RGBA(1.0, 0.0f, 0.5f));

    game::g_player_two.is_active = true;

    m_event_handler.DispatchEvent(SpawnPhysicsEntityEvent(m_player_two, LayerId::FOREGROUND));
}

bool PlayerDaemon::OnControllerAdded(const event::ControllerAddedEvent& event)
{
    if(!m_player_one)
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
        m_camera->Unfollow();
        m_event_handler.DispatchEvent(RemoveEntityEvent(m_player_one->Id()));

        m_player_one = nullptr;
        game::g_player_one.is_active = false;
    }
    else if(event.id == m_player_two_id)
    {
        m_event_handler.DispatchEvent(RemoveEntityEvent(m_player_two->Id()));
        m_player_two = nullptr;
        game::g_player_two.is_active = false;
    }

    return false;
}
