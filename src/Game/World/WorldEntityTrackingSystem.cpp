
#include "WorldEntityTrackingSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "System/Hash.h"
#include "Util/Algorithm.h"

#include <limits>

namespace
{
    constexpr uint32_t NO_CALLBACK_SET = std::numeric_limits<uint32_t>::max();

    game::EntityTrackingComponent MakeTrackingComponent(uint32_t entity_id, game::EntityType type)
    {
        game::EntityTrackingComponent component;
        component.entity_id = entity_id;
        component.type = type;
        component.enabled = true;
        component.enable_trigger = hash::NO_HASH;
        component.disable_trigger = hash::NO_HASH;
        component.enable_callback_id = NO_CALLBACK_SET;
        component.disable_callback_id = NO_CALLBACK_SET;
        return component;
    }
}

using namespace game;

WorldEntityTrackingSystem::WorldEntityTrackingSystem(mono::TriggerSystem* trigger_system)
    : m_trigger_system(trigger_system)
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
    m_entities_to_track.push_back(MakeTrackingComponent(entity_id, EntityType::Boss));
}

void WorldEntityTrackingSystem::ReleaseEntityTracker(uint32_t entity_id)
{
    ForgetEntity(entity_id);
}

void WorldEntityTrackingSystem::UpdateEntityTracker(
    uint32_t entity_id, EntityType type, bool start_enabled, uint32_t enable_trigger, uint32_t disable_trigger)
{
    const auto find_by_entity_id = [entity_id](const EntityTrackingComponent& component) {
        return component.entity_id == entity_id;
    };
    const auto it = std::find_if(m_entities_to_track.begin(), m_entities_to_track.end(), find_by_entity_id);
    if(it == m_entities_to_track.end())
        return;

    EntityTrackingComponent& component = *it;
    RemoveTriggerCallbacks(component);

    component.type = type;
    component.enabled = start_enabled;
    component.enable_trigger = enable_trigger;
    component.disable_trigger = disable_trigger;

    // The callbacks look the component up again, the vector can reallocate.
    if(enable_trigger != hash::NO_HASH)
    {
        const mono::TriggerCallback enable_callback = [this, entity_id](uint32_t trigger_id) {
            SetTrackerEnabled(entity_id, true);
        };
        component.enable_callback_id = m_trigger_system->RegisterTriggerCallback(enable_trigger, enable_callback, entity_id);
    }

    if(disable_trigger != hash::NO_HASH)
    {
        const mono::TriggerCallback disable_callback = [this, entity_id](uint32_t trigger_id) {
            SetTrackerEnabled(entity_id, false);
        };
        component.disable_callback_id = m_trigger_system->RegisterTriggerCallback(disable_trigger, disable_callback, entity_id);
    }
}

void WorldEntityTrackingSystem::SetTrackerEnabled(uint32_t entity_id, bool enabled)
{
    for(EntityTrackingComponent& component : m_entities_to_track)
    {
        if(component.entity_id == entity_id)
            component.enabled = enabled;
    }
}

void WorldEntityTrackingSystem::TrackEntity(uint32_t entity_id, EntityType type)
{
    m_entities_to_track.push_back(MakeTrackingComponent(entity_id, type));
}

void WorldEntityTrackingSystem::ForgetEntity(uint32_t entity_id)
{
    for(EntityTrackingComponent& component : m_entities_to_track)
    {
        if(component.entity_id == entity_id)
            RemoveTriggerCallbacks(component);
    }

    const auto remove_on_entity_id = [entity_id](const EntityTrackingComponent& component) {
        return component.entity_id == entity_id;
    };
    mono::remove_if(m_entities_to_track, remove_on_entity_id);
}

void WorldEntityTrackingSystem::RemoveTriggerCallbacks(EntityTrackingComponent& component)
{
    if(component.enable_callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(component.enable_trigger, component.enable_callback_id, component.entity_id);

    if(component.disable_callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(component.disable_trigger, component.disable_callback_id, component.entity_id);

    component.enable_callback_id = NO_CALLBACK_SET;
    component.disable_callback_id = NO_CALLBACK_SET;
}

void WorldEntityTrackingSystem::SetEntityTypeFilter(EntityType filter_type)
{
}

void WorldEntityTrackingSystem::ClearProperty(EntityType type)
{
}

void WorldEntityTrackingSystem::ClearEntityTypeFilter()
{
}

bool WorldEntityTrackingSystem::IsActiveType(EntityType type) const
{
    return true;
}

const std::vector<EntityTrackingComponent>& WorldEntityTrackingSystem::GetTrackedEntities() const
{
    return m_entities_to_track;
}

