
#include "Spawner.h"
#include "EventHandler/EventHandler.h"
#include "Events/SpawnPhysicsEntityEvent.h"
#include "Meteor.h"
#include "Random.h"
#include "RenderLayers.h"

using namespace game;

Spawner::Spawner(const std::vector<SpawnPoint>& spawn_points, mono::EventHandler& eventHandler)
    : m_spawn_points(spawn_points)
{
    const auto spawn_func = [](void* data) {
        mono::EventHandler* event_handler = static_cast<mono::EventHandler*>(data);

        const float x = mono::Random(-50.0f, 50.0f);
        const float y = mono::Random(-50.0f, 50.0f);

        const game::SpawnPhysicsEntityEvent event(std::make_shared<Meteor>(x, y), BACKGROUND);
        event_handler->DispatchEvent(event);
    };

    m_timer.reset(System::CreateTimer(1000, false, spawn_func, &eventHandler));
}
