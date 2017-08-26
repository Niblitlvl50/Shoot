
#include "PlayerDaemon.h"
#include "Shuttle.h"
#include "RenderLayers.h"

#include "EventHandler/EventHandler.h"
#include "Events/ControllerEvent.h"
#include "Events/EventFuncFwd.h"
#include "Zone/IPhysicsZone.h"
#include "Rendering/ICamera.h"
#include "Rendering/Color.h"
#include "System/System.h"

#include "AIKnowledge.h"

using namespace game;

PlayerDaemon::PlayerDaemon(mono::EventHandler& event_handler, mono::IPhysicsZone* zone)
    : m_event_handler(event_handler),
      m_zone(zone)
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
    if(!m_player_one)
    {
        m_player_one = std::make_shared<Shuttle>(math::zeroVec, m_event_handler, System::GetController(event.id));
        m_player_one->SetPlayerInfo(&game::player_one);
        m_player_one->SetShading(mono::Color::RGBA(0.5, 1.0f, 0.5f));

        m_player_one_id = event.id;
        m_camera->Follow(m_player_one, math::zeroVec);
        m_zone->AddPhysicsEntity(m_player_one, FOREGROUND);
    }
    else
    {
        m_player_two = std::make_shared<Shuttle>(math::zeroVec, m_event_handler, System::GetController(event.id));
        m_player_two->SetPlayerInfo(&game::player_two);
        m_player_two->SetShading(mono::Color::RGBA(1.0, 0.0f, 0.5f));

        m_player_two_id = event.id;
        m_zone->AddPhysicsEntity(m_player_two, FOREGROUND);
    }

    return false;
}

bool PlayerDaemon::OnControllerRemoved(const event::ControllerRemovedEvent& event)
{
    if(event.id == m_player_one_id)
    {
        m_zone->RemovePhysicsEntity(m_player_one);
        m_camera->Unfollow();
        m_player_one = nullptr;
    }
    else if(event.id == m_player_two_id)
    {
        m_zone->RemovePhysicsEntity(m_player_two);
        m_player_two = nullptr;
    }

    return false;
}
