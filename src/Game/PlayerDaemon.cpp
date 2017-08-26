
#include "PlayerDaemon.h"
#include "Shuttle.h"
#include "RenderLayers.h"

#include "EventHandler/EventHandler.h"
#include "Events/ControllerEvent.h"
#include "Events/EventFuncFwd.h"
#include "Zone/IPhysicsZone.h"
#include "Rendering/ICamera.h"
#include "System/System.h"

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
    m_player_one = std::make_shared<Shuttle>(math::zeroVec, m_event_handler, System::GetController(event.id));
    m_zone->AddPhysicsEntity(m_player_one, FOREGROUND);
    m_camera->Follow(m_player_one, math::zeroVec);

    return false;
}

bool PlayerDaemon::OnControllerRemoved(const event::ControllerRemovedEvent& event)
{
    m_zone->RemovePhysicsEntity(m_player_one);
    m_camera->Unfollow();
    m_player_one = nullptr;

    return false;
}
