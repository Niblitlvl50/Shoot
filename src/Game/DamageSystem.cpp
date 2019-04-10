
#include "DamageSystem.h"
#include "Entity/IEntityManager.h"
#include "Hash.h"
#include <numeric>

using namespace game;

DamageSystem::DamageSystem(size_t num_records, IEntityManager* entity_manager)
    : m_entity_manager(entity_manager)
    , m_elapsed_time(0)
    , m_damage_records(num_records)
    , m_destroyed_callbacks(num_records, nullptr)
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
    new_record.last_damaged_timestamp = std::numeric_limits<unsigned int>::max();

    return &new_record;
}

DamageRecord* DamageSystem::CreateRecord(uint32_t id, DestroyedCallback destroyed_callback)
{
    m_destroyed_callbacks[id] = destroyed_callback;
    return CreateRecord(id);
}

void DamageSystem::SetDestroyedCallback(uint32_t id, DestroyedCallback destroyed_callback)
{
    assert(m_destroyed_callbacks[id] == nullptr);
    m_destroyed_callbacks[id] = destroyed_callback;
}

void DamageSystem::ReleaseRecord(uint32_t id)
{
    m_destroyed_callbacks[id] = nullptr;
    m_active[id] = false;
}

DamageRecord* DamageSystem::GetDamageRecord(uint32_t id)
{
    assert(m_active[id]);
    return &m_damage_records[id];
}

DamageResult DamageSystem::ApplyDamage(uint32_t id, int damage)
{
    DamageResult result = { 0 };

    if(!m_active[id])
        return result;

    DamageRecord& damage_record = m_damage_records[id];
    damage_record.health -= damage * damage_record.multipier;
    damage_record.last_damaged_timestamp = m_elapsed_time;

    result.health_left = damage_record.health;

    if(result.health_left <= 0)
    {
        DestroyedCallback callback = m_destroyed_callbacks[id];
        if(callback)
            callback(id);
    }

    return result;
}

const std::vector<DamageRecord>& DamageSystem::GetDamageRecords() const
{
    return m_damage_records;
}

uint32_t DamageSystem::Id() const
{
    return mono::Hash(Name());
}

const char* DamageSystem::Name() const
{
    return "damagesystem";
}

void DamageSystem::Update(uint32_t delta_ms)
{
    m_elapsed_time += delta_ms;

    for(uint32_t entity_id = 0; entity_id < m_damage_records.size(); ++entity_id)
    {
        if(!m_active[entity_id])
            continue;

        DamageRecord& damage_record = m_damage_records[entity_id];
        if(damage_record.health <= 0)
            m_entity_manager->ReleaseEntity(entity_id);
    }
}
