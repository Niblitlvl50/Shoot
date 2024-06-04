
#include "MissionSystem.h"
#include "Util/Random.h"
#include "TransformSystem/TransformSystem.h"
#include "EntitySystem/IEntityManager.h"

#include "System/File.h"
#include "System/System.h"

#include "nlohmann/json.hpp"

using namespace game;

MissionSystem::MissionSystem(mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system)
    : m_entity_manager(entity_manager)
    , m_transform_system(transform_system)
    , m_point_index(0)
{
    file::FilePtr config_file = file::OpenAsciiFile("res/configs/mission_config.json");
    if(config_file)
    {
        const std::vector<byte>& file_data = file::FileRead(config_file);
        const nlohmann::json& json = nlohmann::json::parse(file_data);

        for(auto& mission_json : json["missions"])
        {
            Mission mission;
            mission.name = mission_json["name"];
            mission.entity_collection = mission_json["entity_collection"];
            m_defined_missions.push_back(mission);
        }
    }
}

const char* MissionSystem::Name() const
{
    return "missionsystem";
}

void MissionSystem::Update(const mono::UpdateContext& update_context)
{

}

void MissionSystem::InitializeMissionPositions(const std::vector<uint32_t>& mission_points)
{
    m_mission_points = mission_points;

    mono::UniformRandomBitGenerator random_bit_generator(System::GetMilliseconds());
    std::shuffle(m_mission_points.begin(), m_mission_points.end(), random_bit_generator);
}

void MissionSystem::ActivateMission()
{
    if(m_mission_points.empty())
        return;

    if(m_point_index >= m_mission_points.size())
        return;

    if(m_defined_missions.empty())
        return;

    const uint32_t mission_point_id = m_mission_points[m_point_index];
    const math::Vector& world_position = m_transform_system->GetWorldPosition(mission_point_id);

    // Just the first mission for now
    const Mission& selected_mission = m_defined_missions.front();
    std::vector<mono::Entity> spawned_entities = m_entity_manager->SpawnEntityCollection(selected_mission.entity_collection.c_str());

    // This should position the entities at the mission point.
    for(const mono::Entity& entity : spawned_entities)
    {
        math::Matrix& transform = m_transform_system->GetTransform(entity.id);
        math::Translate(transform, world_position);
    }

    m_activated_mission.spawned_entities;

    m_point_index++;
}

