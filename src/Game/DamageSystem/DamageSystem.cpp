
#include "DamageSystem.h"
#include "Weapons/CollisionCallbacks.h"

#include "EntitySystem/IEntityManager.h"
#include "Math/MathFunctions.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "System/Hash.h"
#include "System/Debug.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Random.h"

#include <limits>


//
// Tickle, White
// Hit, White
// Crush, White
// Destroy, Brown
// Slaughter, Brown
// Obliterate, Brown
// Decimate, Brown
// Demolish, Brown
// Eradicate
// Annihilate, Purple
// Pulverize, Red
// Cremate, Black
// Liquidate, Red
//


//
// Pow
// Blaw
// Hit
// Pof
// Smack
// Blast
// Thud
// Whack
// Slap
// Splat
// Smash
// Crash
// 

using namespace game;

DamageSystem::DamageSystem(
    size_t num_records,
    mono::TransformSystem* tranform_system,
    mono::SpriteSystem* sprite_system,
    mono::IEntityManager* entity_manager)
    : m_transform_system(tranform_system)
    , m_sprite_system(sprite_system)
    , m_entity_manager(entity_manager)
    , m_timestamp(0)
    , m_damage_records(num_records)
    , m_damage_callbacks(num_records)
    , m_damage_filters(num_records)
    , m_active(num_records, false)
{ }

DamageRecord* DamageSystem::CreateRecord(uint32_t id)
{
    MONO_ASSERT(!m_active[id]);
    m_active[id] = true;

    DamageRecord& new_record = m_damage_records[id];
    new_record.health = 100;
    new_record.multipier = 1;
    new_record.last_damaged_timestamp = std::numeric_limits<uint32_t>::max();
    new_record.release_entity_on_death = true;
    new_record.is_boss = false;
    new_record.is_invincible = false;

    return &new_record;
}

void DamageSystem::SetDamageFilter(uint32_t id, DamageFilter damage_filter)
{
    m_damage_filters[id] = damage_filter;
}

void DamageSystem::ClearDamageFilter(uint32_t id)
{
    m_damage_filters[id] = nullptr;
}

uint32_t DamageSystem::SetDamageCallback(uint32_t id, uint32_t callback_types, DamageCallback damage_callback)
{
    DamageCallbacks& damage_callbacks = m_damage_callbacks[id];
    const uint32_t free_index = FindFreeCallbackIndex(damage_callbacks);
    MONO_ASSERT(free_index != std::numeric_limits<uint32_t>::max());
    damage_callbacks[free_index] = { callback_types, damage_callback };
    return free_index;
}

void DamageSystem::RemoveDamageCallback(uint32_t id, uint32_t callback_id)
{
    m_damage_callbacks[id][callback_id].callback = nullptr;
}

uint32_t DamageSystem::SetGlobalDamageCallback(uint32_t callback_types, DamageCallback damage_callback)
{
    const uint32_t free_index = FindFreeCallbackIndex(m_global_damage_callbacks);
    MONO_ASSERT(free_index != std::numeric_limits<uint32_t>::max());
    m_global_damage_callbacks[free_index] = { callback_types, damage_callback };
    return free_index;
}

void DamageSystem::RemoveGlobalDamageCallback(uint32_t callback_id)
{
    m_global_damage_callbacks[callback_id].callback = nullptr;
}

void DamageSystem::ReleaseRecord(uint32_t id)
{
    for(auto& callback : m_damage_callbacks[id])
        callback.callback = nullptr;

    ClearDamageFilter(id);
    m_active[id] = false;
}

bool DamageSystem::IsAllocated(uint32_t id) const
{
    if(id >= m_active.size())
        return false;

    return m_active[id];
}

void DamageSystem::ReactivateDamageRecord(uint32_t id)
{
    m_active[id] = true;

    DamageRecord* record = GetDamageRecord(id);
    record->health = record->full_health;
    record->last_damaged_timestamp = std::numeric_limits<uint32_t>::max();
}

DamageRecord* DamageSystem::GetDamageRecord(uint32_t id)
{
    MONO_ASSERT(m_active[id]);
    return &m_damage_records[id];
}

DamageResult DamageSystem::ApplyDamage(uint32_t id, int damage, uint32_t id_who_did_damage)
{
    DamageResult result = { false, 0 };

    const bool has_damage_record = IsAllocated(id);
    if(!has_damage_record)
        return result;

    DamageRecord& damage_record = m_damage_records[id];
    if(damage_record.health <= 0 || damage_record.is_invincible)
        return result;

    const DamageFilter& damage_filter = m_damage_filters[id];
    if(damage_filter != nullptr)
    {
        const FilterResult filter_result = damage_filter(id, damage, id_who_did_damage);
        if(filter_result == FilterResult::FILTER_OUT)
            return result;
    }

    damage_record.health -= damage * damage_record.multipier;
    damage_record.last_damaged_timestamp = m_timestamp;

    damage_record.health = std::max(0, damage_record.health);

    result.did_damage = true;
    result.health_left = damage_record.health;

    const DamageType damage_type = (result.health_left <= 0) ? DamageType::DESTROYED : DamageType::DAMAGED;
    m_damage_events.push_back(
        { id, id_who_did_damage, damage, damage_type }
    );

    return result;
}

void DamageSystem::GainHealth(uint32_t id, int health_gain)
{
    DamageRecord* record = GetDamageRecord(id);
    record->health = std::clamp(record->health + health_gain, 0, record->full_health);
    record->last_damaged_timestamp = m_timestamp;
}

const std::vector<DamageRecord>& DamageSystem::GetDamageRecords() const
{
    return m_damage_records;
}

void DamageSystem::PreventReleaseOnDeath(uint32_t id, bool enable)
{
    m_damage_records[id].release_entity_on_death = !enable;
}

bool DamageSystem::IsInvincible(uint32_t id) const
{
    const bool has_damage_record = IsAllocated(id);
    if(!has_damage_record)
        return false;

    return m_damage_records[id].is_invincible;
}

void DamageSystem::SetInvincible(uint32_t id, bool invincible)
{
    const bool has_damage_record = IsAllocated(id);
    if(has_damage_record)
        m_damage_records[id].is_invincible = invincible;
}

bool DamageSystem::IsBoss(uint32_t id) const
{
    const bool has_damage_record = IsAllocated(id);
    if(!has_damage_record)
        return false;

    return m_damage_records[id].is_boss;
}


const char* DamageSystem::Name() const
{
    return "damagesystem";
}

void DamageSystem::Update(const mono::UpdateContext& update_context)
{
    m_timestamp = update_context.timestamp;

    const auto call_callbacks = [](const DamageEvent& damage_event, DamageCallbacks& callbacks) {
        for(const auto& callback_data : callbacks)
        {
            if(!callback_data.callback)
                continue;
    
            const bool valid_callback_type = (callback_data.callback_types & damage_event.damage_result);
            if(valid_callback_type)
            {
                callback_data.callback(
                    damage_event.id,
                    damage_event.damage,
                    damage_event.id_who_did_damage,
                    damage_event.damage_result);
            }
        }
    };

    for(const DamageEvent& damage_event : m_damage_events)
    {
        call_callbacks(damage_event, m_damage_callbacks[damage_event.id]);
        call_callbacks(damage_event, m_global_damage_callbacks);
    }
    m_damage_events.clear();

    for(uint32_t entity_id = 0; entity_id < m_damage_records.size(); ++entity_id)
    {
        if(!m_active[entity_id])
            continue;

        const DamageRecord& damage_record = m_damage_records[entity_id];
        if(damage_record.health <= 0)
        {
            if(damage_record.release_entity_on_death)
                m_entity_manager->ReleaseEntity(entity_id);

            //constexpr const char* explosion_entity = "res/entities/explosion_small.entity";
            constexpr const char* explosion_entity = "res/entities/explosion_splatter_small.entity";
            //constexpr const char* explosion_entity = "res/entities/explosion_medium_3.entity";
            game::SpawnEntityWithAnimation(
                explosion_entity, 0, entity_id, m_entity_manager, m_transform_system, m_sprite_system);
            m_active[entity_id] = false;
        }
    }
}

void DamageSystem::Destroy()
{ }

uint32_t DamageSystem::FindFreeCallbackIndex(const DamageCallbacks& callbacks) const
{
    for(uint32_t index = 0; index < std::size(callbacks); ++index)
    {
        if(callbacks[index].callback == nullptr)
            return index;
    }

    return std::numeric_limits<uint32_t>::max();
}
