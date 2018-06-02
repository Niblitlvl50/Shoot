
#include "PlayerDaemon.h"
#include "Shuttle.h"
#include "RenderLayers.h"
#include "Events/SpawnPhysicsEntityEvent.h"
#include "Events/RemoveEntityEvent.h"
#include "AIKnowledge.h"

#include "EventHandler/EventHandler.h"
#include "Events/ControllerEvent.h"
#include "Events/EventFuncFwd.h"

#include "Rendering/ICamera.h"
#include "Rendering/Color.h"
#include "System/System.h"

#include <functional>

using namespace game;

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
    const math::Vector& spawn_point = m_player_points.empty() ? math::ZeroVec : m_player_points.front();

    m_player_one =
        std::make_shared<Shuttle>(spawn_point, m_event_handler, System::GetController(System::ControllerId::Primary));
    m_player_one->SetPlayerInfo(&game::g_player_one);
    //m_player_one->SetShading(mono::Color::RGBA(0.5, 1.0f, 0.5f));

    game::g_player_one.is_active = true;

    m_camera->SetPosition(spawn_point);
    m_camera->Follow(m_player_one, math::ZeroVec);

    const auto destroyed_func = [](unsigned int id) {
        game::g_player_one.is_active = false;
    };

    m_event_handler.DispatchEvent(SpawnPhysicsEntityEvent(m_player_one, FOREGROUND, destroyed_func));
}

void PlayerDaemon::SpawnPlayer2()
{
    const math::Vector& spawn_point = m_player_points.empty() ? math::ZeroVec : m_player_points.front();

    m_player_two =
        std::make_shared<Shuttle>(spawn_point, m_event_handler, System::GetController(System::ControllerId::Secondary));
    m_player_two->SetPlayerInfo(&game::g_player_two);
    m_player_two->SetShading(mono::Color::RGBA(1.0, 0.0f, 0.5f));

    game::g_player_two.is_active = true;
    
    const auto destroyed_func = [](unsigned int id) {
        game::g_player_two.is_active = false;
    };
    
    m_event_handler.DispatchEvent(SpawnPhysicsEntityEvent(m_player_two, FOREGROUND, destroyed_func));
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
