
#include "DamageSystem.h"
#include "Effects/GibSystem.h"
#include "Entity/IEntityManager.h"
#include "EventHandler/EventHandler.h"
#include "Particle/ParticleSystem.h"
#include "Events/ScoreEvent.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Hash.h"
#include <limits>
#include <cassert>

using namespace game;

DamageSystem::DamageSystem(
    size_t num_records,
    IEntityManager* entity_manager,
    mono::ParticleSystem* particle_system,
    mono::TransformSystem* transform_system,
    mono::EventHandler* event_handler)
    : m_entity_manager(entity_manager)
    , m_particle_system(particle_system)
    , m_transform_system(transform_system)
    , m_event_handler(event_handler)
    , m_timestamp(0)
    , m_damage_records(num_records)
    , m_destroyed_callbacks(num_records)
    , m_active(num_records, false)
{
    m_damage_effect = std::make_unique<DamageEffect>(m_particle_system);
}

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

    return &new_record;
}

uint32_t DamageSystem::SetDestroyedCallback(uint32_t id, DestroyedCallback destroyed_callback)
{
    const size_t free_index = FindFreeCallbackIndex(id);
    assert(free_index != std::numeric_limits<size_t>::max());
    m_destroyed_callbacks[id][free_index] = destroyed_callback;
    return free_index;
}

void DamageSystem::RemoveCallback(uint32_t id, uint32_t callback_id)
{
    m_destroyed_callbacks[id][callback_id] = nullptr;
}

void DamageSystem::ReleaseRecord(uint32_t id)
{
    for(auto& callback : m_destroyed_callbacks[id])
        callback = nullptr;

    m_active[id] = false;
}

DamageRecord* DamageSystem::GetDamageRecord(uint32_t id)
{
    assert(m_active[id]);
    return &m_damage_records[id];
}

DamageResult DamageSystem::ApplyDamage(uint32_t id, int damage, uint32_t id_who_did_damage)
{
    DamageResult result = { 0 };

    if(!m_active[id])
        return result;

    DamageRecord& damage_record = m_damage_records[id];
    damage_record.health -= damage * damage_record.multipier;
    damage_record.last_damaged_timestamp = m_timestamp;

    result.health_left = damage_record.health;

    if(result.health_left <= 0)
    {
        for(const auto& callback : m_destroyed_callbacks[id])
        {
            if(callback)
                callback(id);
        }

        m_event_handler->DispatchEvent(game::ScoreEvent(id_who_did_damage, damage_record.score));
    }

    const math::Matrix& world_transform = m_transform_system->GetWorld(id);
    const math::Vector& world_position = math::GetPosition(world_transform);
    m_damage_effect->EmitGibsAt(world_position, 0.0f);

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

uint32_t DamageSystem::Capacity() const
{
    return m_damage_records.size();
}

void DamageSystem::Update(const mono::UpdateContext& update_context)
{
    m_timestamp = update_context.total_time;

    for(uint32_t entity_id = 0; entity_id < m_damage_records.size(); ++entity_id)
    {
        if(!m_active[entity_id])
            continue;

        DamageRecord& damage_record = m_damage_records[entity_id];
        if(damage_record.health <= 0)
            m_entity_manager->ReleaseEntity(entity_id);
    }
}

size_t DamageSystem::FindFreeCallbackIndex(uint32_t id) const
{
    const DestroyedCallbacks& callbacks = m_destroyed_callbacks[id];
    for(size_t index = 0; index < callbacks.size(); ++index)
    {
        if(callbacks[index] == nullptr)
            return index;
    }

    return std::numeric_limits<size_t>::max();
}
