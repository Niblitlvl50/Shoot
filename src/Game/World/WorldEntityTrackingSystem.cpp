
#include "WorldEntityTrackingSystem.h"
#include "Util/Algorithm.h"

using namespace game;

WorldEntityTrackingSystem::WorldEntityTrackingSystem()
    : m_type_filter((uint32_t)EntityType::All)
{ }

const char* WorldEntityTrackingSystem::Name() const
{
    return "WorldEntityTrackingSystem";
}

void WorldEntityTrackingSystem::Begin()
{
    ClearEntityTypeFilter();
}

void WorldEntityTrackingSystem::Update(const mono::UpdateContext& update_context)
{

}

void WorldEntityTrackingSystem::AllocateEntityTracker(uint32_t entity_id)
{
    const EntityTrackingComponent component = { entity_id, EntityType::Boss };
    m_entities_to_track.push_back(component);
}

void WorldEntityTrackingSystem::ReleaseEntityTracker(uint32_t entity_id)
{
    const auto remove_on_entity_id = [entity_id](const EntityTrackingComponent& component) {
        return component.entity_id == entity_id;
    };
    mono::remove_if(m_entities_to_track, remove_on_entity_id);
}

void WorldEntityTrackingSystem::UpdateEntityTracker(uint32_t entity_id, EntityType type)
{
    const auto find_by_entity_id = [entity_id](const EntityTrackingComponent& component) {
        return component.entity_id == entity_id;
    };
    const auto it = std::find_if(m_entities_to_track.begin(), m_entities_to_track.end(), find_by_entity_id);
    if(it != m_entities_to_track.end())
        it->type = type;
}

void WorldEntityTrackingSystem::TrackEntity(uint32_t entity_id, EntityType type)
{
    const EntityTrackingComponent component = { entity_id, type };
    m_entities_to_track.push_back(component);
}

void WorldEntityTrackingSystem::ForgetEntity(uint32_t entity_id)
{
    const auto remove_on_entity_id = [entity_id](const EntityTrackingComponent& component) {
        return component.entity_id == entity_id;
    };
    mono::remove_if(m_entities_to_track, remove_on_entity_id);
}

void WorldEntityTrackingSystem::SetEntityTypeFilter(EntityType filter_type)
{
    m_type_filter |= (uint32_t)filter_type;
}

void WorldEntityTrackingSystem::ClearProperty(EntityType type)
{
    m_type_filter &= ~(uint32_t)type;
}

void WorldEntityTrackingSystem::ClearEntityTypeFilter()
{
    m_type_filter = (uint32_t)EntityType::All;
}

bool WorldEntityTrackingSystem::IsActiveType(EntityType type) const
{
    return (m_type_filter & (uint32_t)type);
}

const std::vector<EntityTrackingComponent>& WorldEntityTrackingSystem::GetTrackedEntities() const
{
    return m_entities_to_track;
}

