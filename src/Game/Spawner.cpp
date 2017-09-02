
#include "Spawner.h"
#include "EventHandler/EventHandler.h"
#include "Events/SpawnPhysicsEntityEvent.h"
#include "Meteor.h"
#include "Random.h"
#include "RenderLayers.h"

using namespace game;

Spawner::Spawner(const std::vector<SpawnPoint>& spawn_points, mono::EventHandler& event_handler)
    : m_spawn_points(spawn_points),
      m_event_handler(event_handler)
{
    const auto spawn_func = [](void* data) {
        Spawner* spawner = static_cast<Spawner*>(data);
        spawner->SpawnObject();
    };

    m_timer.reset(System::CreateTimer(1000, false, spawn_func, this));
}

void Spawner::SpawnObject()
{
    const int spawn_point_index = mono::RandomInt(0, m_spawn_points.size() -1);
    const SpawnPoint& point = m_spawn_points[spawn_point_index];

    const game::SpawnPhysicsEntityEvent event(std::make_shared<Meteor>(point.position.x, point.position.y), BACKGROUND);
    m_event_handler.DispatchEvent(event);
}