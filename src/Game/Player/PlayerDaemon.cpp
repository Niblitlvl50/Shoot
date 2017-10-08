
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

using namespace game;

PlayerDaemon::PlayerDaemon(
    const std::vector<math::Vector>& player_points, mono::EventHandler& event_handler)
    : m_player_points(player_points),
      m_event_handler(event_handler)
{
    using namespace std::placeholders;

    const event::ControllerAddedFunc& added_func = std::bind(&PlayerDaemon::OnControllerAdded, this, _1);
    const event::ControllerRemovedFunc& removed_func = std::bind(&PlayerDaemon::OnControllerRemoved, this, _1);

    m_added_token = m_event_handler.AddListener(added_func);
    m_removed_token = m_event_handler.AddListener(removed_func);
}

PlayerDaemon::~PlayerDaemon()
{
    m_event_handler.RemoveListener(m_added_token);
    m_event_handler.RemoveListener(m_removed_token);
}

void PlayerDaemon::SetCamera(const mono::ICameraPtr& camera)
{
    m_camera = camera;
}

bool PlayerDaemon::OnControllerAdded(const event::ControllerAddedEvent& event)
{
    const math::Vector& spawn_point = m_player_points.empty() ? math::zeroVec : m_player_points.front();

    if(!m_player_one)
    {
        m_player_one = std::make_shared<Shuttle>(spawn_point, m_event_handler, System::GetController(event.id));
        m_player_one->SetPlayerInfo(&game::player_one);
        //m_player_one->SetShading(mono::Color::RGBA(0.5, 1.0f, 0.5f));

        game::player_one.is_active = true;

        m_player_one_id = event.id;
        m_camera->Follow(m_player_one, math::zeroVec);

        const auto destroyed_func = [this](unsigned int id) {
            game::player_one.is_active = false;
        };

        m_event_handler.DispatchEvent(SpawnPhysicsEntityEvent(m_player_one, FOREGROUND, destroyed_func));
    }
    else
    {
        m_player_two = std::make_shared<Shuttle>(spawn_point, m_event_handler, System::GetController(event.id));
        m_player_two->SetPlayerInfo(&game::player_two);
        m_player_two->SetShading(mono::Color::RGBA(1.0, 0.0f, 0.5f));

        game::player_two.is_active = true;
        
        m_player_two_id = event.id;

        const auto destroyed_func = [this](unsigned int id) {
            game::player_two.is_active = false;
        };
        
        m_event_handler.DispatchEvent(SpawnPhysicsEntityEvent(m_player_two, FOREGROUND, destroyed_func));
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
        game::player_one.is_active = false;        
    }
    else if(event.id == m_player_two_id)
    {
        m_event_handler.DispatchEvent(RemoveEntityEvent(m_player_two->Id()));
        m_player_two = nullptr;
        game::player_two.is_active = false;
    }

    return false;
}
