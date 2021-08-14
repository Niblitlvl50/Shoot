
#include "DamageSystem.h"
#include "Weapons/CollisionCallbacks.h"

#include "EntitySystem/IEntityManager.h"
#include "EventHandler/EventHandler.h"
#include "Events/ScoreEvent.h"
#include "Math/MathFunctions.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "System/Hash.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Random.h"

#include <limits>
#include <cassert>

using namespace game;

DamageSystem::DamageSystem(
    size_t num_records,
    mono::TransformSystem* tranform_system,
    mono::SpriteSystem* sprite_system,
    mono::IEntityManager* entity_manager,
    mono::EventHandler* event_handler)
    : m_transform_system(tranform_system)
    , m_sprite_system(sprite_system)
    , m_entity_manager(entity_manager)
    , m_event_handler(event_handler)
    , m_timestamp(0)
    , m_damage_records(num_records)
    , m_damage_callbacks(num_records)
    , m_active(num_records, false)
{ }

DamageRecord* DamageSystem::CreateRecord(uint32_t id)
{
    assert(!m_active[id]);
    m_active[id] = true;

    DamageRecord& new_record = m_damage_records[id];
    new_record.health = 100;
    new_record.strong_against = 0;
    new_record.weak_against = 0;
    new_record.multipier = 1;
    new_record.last_damaged_timestamp = std::numeric_limits<uint32_t>::max();
    new_record.release_entity_on_death = true;

    return &new_record;
}

uint32_t DamageSystem::SetDamageCallback(uint32_t id, uint32_t callback_types, DamageCallback damage_callback)
{
    const size_t free_index = FindFreeCallbackIndex(id);
    assert(free_index != std::numeric_limits<size_t>::max());
    m_damage_callbacks[id][free_index] = { callback_types, damage_callback };
    return free_index;
}

void DamageSystem::RemoveDamageCallback(uint32_t id, uint32_t callback_id)
{
    m_damage_callbacks[id][callback_id].callback = nullptr;
}

void DamageSystem::ReleaseRecord(uint32_t id)
{
    for(auto& callback : m_damage_callbacks[id])
        callback.callback = nullptr;

    m_active[id] = false;
}

bool DamageSystem::IsAllocated(uint32_t id) const
{
    return m_active[id];
}

DamageRecord* DamageSystem::GetDamageRecord(uint32_t id)
{
    assert(m_active[id]);
    return &m_damage_records[id];
}

DamageResult DamageSystem::ApplyDamage(uint32_t id, int damage, uint32_t id_who_did_damage)
{
    DamageResult result = { false, 0 };

    DamageRecord& damage_record = m_damage_records[id];
    if(damage_record.health <= 0)
        return result;

    damage_record.health -= damage * damage_record.multipier;
    damage_record.last_damaged_timestamp = m_timestamp;

    damage_record.health = std::max(0, damage_record.health);

    result.did_damage = true;
    result.health_left = damage_record.health;

    const DamageType damage_type = (result.health_left <= 0) ? DamageType::DESTROYED : DamageType::DAMAGED;

    for(const auto& callback_data : m_damage_callbacks[id])
    {
        if(callback_data.callback && callback_data.callback_types & damage_type)
            callback_data.callback(id, damage, id_who_did_damage, damage_type);
    }

    if(damage_type == DamageType::DESTROYED)
    {
        m_event_handler->DispatchEvent(game::ScoreEvent(id_who_did_damage, damage_record.score));
        if(!damage_record.release_entity_on_death)
            m_destroyed_but_not_released.push_back(id);
    }

    return result;
}

const std::vector<DamageRecord>& DamageSystem::GetDamageRecords() const
{
    return m_damage_records;
}

void DamageSystem::PreventReleaseOnDeath(uint32_t id, bool enable)
{
    m_damage_records[id].release_entity_on_death = !enable;
}

uint32_t DamageSystem::Id() const
{
    return hash::Hash(Name());
}

const char* DamageSystem::Name() const
{
    return "damagesystem";
}

void DamageSystem::Update(const mono::UpdateContext& update_context)
{
    m_timestamp = update_context.timestamp;

    for(uint32_t entity_id = 0; entity_id < m_damage_records.size(); ++entity_id)
    {
        if(!m_active[entity_id])
            continue;

        const DamageRecord& damage_record = m_damage_records[entity_id];
        if(damage_record.health <= 0)
        {
            if(damage_record.release_entity_on_death)
                m_entity_manager->ReleaseEntity(entity_id);

            game::SpawnEntityWithAnimation("res/entities/explosion_small.entity", 0, entity_id, m_entity_manager, m_transform_system, m_sprite_system);
            m_active[entity_id] = false;
        }
    }
}

void DamageSystem::Destroy()
{ }

size_t DamageSystem::FindFreeCallbackIndex(uint32_t id) const
{
    const DamageCallbacks& callbacks = m_damage_callbacks[id];
    for(size_t index = 0; index < callbacks.size(); ++index)
    {
        if(callbacks[index].callback == nullptr)
            return index;
    }

    return std::numeric_limits<size_t>::max();
}
