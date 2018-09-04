
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

#include "System/File.h"
#include "nlohmann_json/json.hpp"

using namespace game;

std::vector<Wave> game::LoadWaveFile(const char* wave_file)
{
    std::vector<Wave> waves;
    File::FilePtr file = File::OpenAsciiFile(wave_file);
    if(!file)
        return waves;

    std::vector<byte> file_data;
    File::FileRead(file, file_data);
    const nlohmann::json& json = nlohmann::json::parse(file_data);

    const nlohmann::json& json_waves = json["waves"];
    waves.reserve(json_waves.size());

    for(const auto& json_wave : json_waves)
    {
        Wave wave_data;
        wave_data.name = json_wave["name"];
        json_wave["tags"];

        const nlohmann::json& json_tags = json_wave["tags"];
        wave_data.tags.reserve(json_tags.size());

        for(const auto& json_tag : json_tags)
            wave_data.tags.push_back(json_tag);

        waves.push_back(wave_data);
    }

    return waves;
}

Spawner::Spawner(
    const std::vector<SpawnPoint>& spawn_points, const std::vector<Wave>& waves, mono::EventHandler& event_handler)
    : m_spawn_points(spawn_points)
    , m_waves(waves)
    , m_event_handler(event_handler)
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
