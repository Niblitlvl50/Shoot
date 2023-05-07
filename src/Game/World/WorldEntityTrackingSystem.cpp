
#include "WorldEntityTrackingSystem.h"
#include "Util/Algorithm.h"

using namespace game;

WorldEntityTrackingSystem::WorldEntityTrackingSystem()
{
}

const char* WorldEntityTrackingSystem::Name() const
{
    return "WorldEntityTrackingSystem";
}

void WorldEntityTrackingSystem::Update(const mono::UpdateContext& update_context)
{

}

void WorldEntityTrackingSystem::TrackEntity(uint32_t entity_id)
{
    m_entities_to_track.push_back(entity_id);
}

void WorldEntityTrackingSystem::ForgetEntity(uint32_t entity_id)
{
    mono::remove(m_entities_to_track, entity_id);
}

const std::vector<uint32_t>& WorldEntityTrackingSystem::GetTrackedEntities() const
{
    return m_entities_to_track;
}

