
#include "EntityLifetimeTriggerSystem.h"
#include "DamageSystem/DamageSystem.h"

#include "EntitySystem/IEntityManager.h"
#include "TriggerSystem/TriggerSystem.h"
#include "Util/Algorithm.h"

namespace
{
    constexpr uint32_t NO_CALLBACK_SET = std::numeric_limits<uint32_t>::max();
}

using namespace game;

EntityLifetimeTriggerSystem::EntityLifetimeTriggerSystem(
    mono::TriggerSystem* trigger_system, mono::IEntityManager* entity_manager, DamageSystem* damage_system)
    : m_trigger_system(trigger_system)
    , m_entity_system(entity_manager)
    , m_damage_system(damage_system)
//    , m_destroyed_triggers(100)
{

}

const char* EntityLifetimeTriggerSystem::Name() const
{
    return "entity_lifetime_trigger_system";
}

void EntityLifetimeTriggerSystem::AllocateDestroyedTrigger(uint32_t entity_id)
{
    DestroyedTriggerComponent component;
    component.entity_id = entity_id;
    component.callback_id = NO_CALLBACK_SET;
    component.trigger_hash = 0;
    component.trigger_type = DestroyedTriggerType::ON_DEATH;

    m_destroyed_triggers.push_back(std::move(component));
}

void EntityLifetimeTriggerSystem::ReleaseDestroyedTrigger(uint32_t entity_id)
{
    const auto find_by_id = [entity_id](const DestroyedTriggerComponent& component) {
        return component.entity_id == entity_id;
    };
    DestroyedTriggerComponent* allocated_trigger = mono::find_if(m_destroyed_triggers, find_by_id);
    if(!allocated_trigger)
        return;

    switch(allocated_trigger->trigger_type)
    {
    case DestroyedTriggerType::ON_DEATH:
        m_damage_system->RemoveDamageCallback(entity_id, allocated_trigger->callback_id);
        break;
    case DestroyedTriggerType::ON_DESTORYED:
        m_entity_system->RemoveReleaseCallback(entity_id, allocated_trigger->callback_id);
        break;
    };
    allocated_trigger->callback_id = NO_CALLBACK_SET;

    mono::remove_if(m_destroyed_triggers, find_by_id);
}

void EntityLifetimeTriggerSystem::AddDestroyedTrigger(uint32_t entity_id, uint32_t trigger_hash, DestroyedTriggerType type)
{
    const auto find_by_id = [entity_id](const DestroyedTriggerComponent& component) {
        return component.entity_id == entity_id;
    };
    DestroyedTriggerComponent* allocated_trigger = mono::find_if(m_destroyed_triggers, find_by_id);
    if(!allocated_trigger)
        return;

    allocated_trigger->trigger_hash = trigger_hash;

    if(allocated_trigger->callback_id != NO_CALLBACK_SET)
    {
        if(allocated_trigger->trigger_type == DestroyedTriggerType::ON_DEATH)
            m_damage_system->RemoveDamageCallback(entity_id, allocated_trigger->callback_id);
        else
            m_entity_system->RemoveReleaseCallback(entity_id, allocated_trigger->callback_id);
    }

    allocated_trigger->trigger_type = type;

    if(allocated_trigger->trigger_type == DestroyedTriggerType::ON_DEATH)
    {
        const DamageCallback callback = [this, trigger_hash](uint32_t damaged_entity_id, uint32_t id_who_did_damage, uint32_t weapon_identifier, int damage, DamageType type) {
            m_trigger_system->EmitTrigger(trigger_hash);
        };
        allocated_trigger->callback_id = m_damage_system->SetDamageCallback(entity_id, DamageType::DESTROYED, callback);
    }
    else
    {
        const mono::ReleaseCallback callback = [this, trigger_hash](uint32_t id, mono::ReleasePhase phase) {
            m_trigger_system->EmitTrigger(trigger_hash);
        };
        allocated_trigger->callback_id = m_entity_system->AddReleaseCallback(entity_id, mono::ReleasePhase::PRE_RELEASE, callback);
    }
}
