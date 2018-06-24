
#include "Spawner.h"
#include "Events/SpawnPhysicsEntityEvent.h"
#include "Random.h"
#include "RenderLayers.h"
#include "Factories.h"
#include "Enemies/IEnemyFactory.h"
#include "Enemies/Enemy.h"

#include "EventHandler/EventHandler.h"
#include "Math/MathFunctions.h"

#include "ObjectAttribute.h"

using namespace game;

Spawner::Spawner(const std::vector<SpawnPoint>& spawn_points, mono::EventHandler& event_handler)
    : m_spawn_points(spawn_points),
      m_event_handler(event_handler)
{
    const auto spawn_func = [](void* data) {
        Spawner* spawner = static_cast<Spawner*>(data);
        spawner->CheckForSpawn();
    };

    m_timer.reset(System::CreateTimer(1000, false, spawn_func, this));
}

void Spawner::CheckForSpawn()
{
    m_seconds_elapsed++;

    const auto check_spawn_func = [this](const SpawnPoint& spawn_point) {
        return (spawn_point.time_stamp < m_seconds_elapsed);
    };

    auto it = std::remove_if(m_spawn_points.begin(), m_spawn_points.end(), check_spawn_func);
    if(it != m_spawn_points.end())
    {
        for(auto spawn_it = it; spawn_it != m_spawn_points.end(); ++spawn_it)
            SpawnObject(*spawn_it);

        m_spawn_points.erase(it, m_spawn_points.end());
    }
}

void Spawner::SpawnObject(const SpawnPoint& spawn_point)
{
    const std::vector<Attribute> attributes;

    for(const std::string& spawn_tag : spawn_point.spawn_tags)
    {
        const float spawn_angle = mono::Random(0.0f, math::PI() * 2.0f);
        const float spawn_radius = mono::Random(0.0f, spawn_point.radius);

        const math::Vector& spawn_offset = math::VectorFromAngle(spawn_angle) * spawn_radius;

        EnemyPtr enemy = enemy_factory->CreateFromName(spawn_tag.c_str(), spawn_point.position + spawn_offset, attributes);
        m_event_handler.DispatchEvent(game::SpawnPhysicsEntityEvent(enemy, LayerId::BACKGROUND));
    }    
}
