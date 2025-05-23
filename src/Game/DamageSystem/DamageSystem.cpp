
#include "DamageSystem.h"
#include "Shockwave.h"
#include "Weapons/CollisionCallbacks.h"
#include "Debug/GameDebugVariables.h"
#include "Debug/IDebugDrawer.h"

#include "EntitySystem/IEntityManager.h"
#include "Math/MathFunctions.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "System/File.h"
#include "System/Hash.h"
#include "System/Debug.h"
#include "TransformSystem/TransformSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "Util/Random.h"

#include "nlohmann/json.hpp"

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
    mono::PhysicsSystem* physics_system,
    mono::IEntityManager* entity_manager,
    mono::TriggerSystem* trigger_system)
    : m_transform_system(tranform_system)
    , m_sprite_system(sprite_system)
    , m_physics_system(physics_system)
    , m_entity_manager(entity_manager)
    , m_trigger_system(trigger_system)
    , m_timestamp(0)
    , m_damage_records(num_records)
    , m_damage_callbacks(num_records)
    , m_damage_filters(num_records)
    , m_active(num_records, false)
{
    file::FilePtr config_file = file::OpenAsciiFile("res/configs/damage_config.json");
    if(config_file)
    {
        const std::vector<byte>& file_data = file::FileRead(config_file);
        const nlohmann::json& json = nlohmann::json::parse(file_data);

        m_death_entities = json["death_entities"];
    }
}

DamageRecord* DamageSystem::CreateRecord(uint32_t id)
{
    MONO_ASSERT(!m_active[id]);
    m_active[id] = true;

    DamageRecord& new_record = m_damage_records[id];
    new_record.health = 100;
    new_record.multipier = 1.0f;
    new_record.last_damaged_timestamp = std::numeric_limits<uint32_t>::max();
    new_record.release_entity_on_death = true;
    new_record.is_boss = false;
    new_record.is_invincible = false;

    return &new_record;
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

ShockwaveComponent* DamageSystem::CreateShockwaveComponent(uint32_t entity_id)
{
    auto pair = m_shockwave_components.insert(std::make_pair(entity_id, ShockwaveComponent()));
    return &pair.first->second;
}

void DamageSystem::ReleaseShockwaveComponent(uint32_t entity_id)
{
    ShockwaveComponent& component = m_shockwave_components[entity_id];

    if(component.trigger_handle != mono::INVALID_ID)
        m_trigger_system->RemoveTriggerCallback(component.trigger_hash, component.trigger_handle, mono::INVALID_ID);

    m_shockwave_components.erase(entity_id);
}

void DamageSystem::UpdateShockwaveComponent(uint32_t entity_id, uint32_t trigger_hash, float radius, float magnitude, int damage)
{
    ShockwaveComponent& component = m_shockwave_components[entity_id];

    if(component.trigger_handle != mono::INVALID_ID)
        m_trigger_system->RemoveTriggerCallback(component.trigger_hash, component.trigger_handle, mono::INVALID_ID);

    component.trigger_hash = trigger_hash;
    component.radius = radius;
    component.magnitude = magnitude;
    component.damage = damage;
    component.trigger_handle = mono::INVALID_ID;

    const mono::TriggerCallback trigger_callback = [this, entity_id](uint32_t trigger_id) {
        ApplyShockwave(entity_id);
    };
    component.trigger_handle = m_trigger_system->RegisterTriggerCallback(component.trigger_hash, trigger_callback, mono::INVALID_ID);
}

DamageResult DamageSystem::ApplyDamage(uint32_t id_damaged_entity, uint32_t id_who_did_damage, uint32_t weapon_identifier, const DamageDetails& damage_details)
{
    DamageResult result = { false, 0 };

    const bool has_damage_record = IsAllocated(id_damaged_entity);
    if(!has_damage_record)
        return result;

    DamageRecord& damage_record = m_damage_records[id_damaged_entity];
    if(damage_record.health <= 0 || damage_record.is_invincible)
        return result;

    const DamageFilter& damage_filter = m_damage_filters[id_damaged_entity];
    if(damage_filter != nullptr)
    {
        const FilterResult filter_result = damage_filter(id_damaged_entity, id_who_did_damage, weapon_identifier, damage_details.damage);
        if(filter_result == FilterResult::FILTER_OUT)
            return result;
    }

    damage_record.health -= damage_details.damage * damage_record.multipier;
    damage_record.last_damaged_timestamp = m_timestamp;

    damage_record.health = std::max(0, damage_record.health);

    result.did_damage = true;
    result.health_left = damage_record.health;

    const DamageType damage_type = (result.health_left <= 0) ? DamageType::DESTROYED : DamageType::DAMAGED;
    m_damage_events.push_back(
        { id_damaged_entity, id_who_did_damage, weapon_identifier, damage_details.damage, damage_details.critical_hit, damage_type }
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

const std::vector<DamageEvent>& DamageSystem::GetDamageEventsThisFrame() const
{
    return m_damage_events;
}

void DamageSystem::ApplyShockwave(uint32_t entity_id)
{
    const ShockwaveComponent& component = m_shockwave_components[entity_id];
    const math::Vector& world_position = m_transform_system->GetWorldPosition(entity_id);
    game::ShockwaveAndDamageAt(
        m_physics_system,
        this,
        world_position,
        component.radius,
        component.magnitude,
        component.damage,
        entity_id,
        CollisionCategory::CC_ALL);

    if(g_debug_draw_shockwaves)
    {
        g_debug_drawer->DrawCircleFading(world_position, component.radius, mono::Color::RED, 1.0f);
    }
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

void DamageSystem::SetDamageMultiplier(uint32_t id, float multiplier)
{
    const bool has_damage_record = IsAllocated(id);
    if(has_damage_record)
        m_damage_records[id].multipier = multiplier;
}

float DamageSystem::GetDamageMultiplier(uint32_t id) const
{
    const bool has_damage_record = IsAllocated(id);
    if(has_damage_record)
        return m_damage_records[id].multipier;

    return 1.0f;
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
                    damage_event.id_damaged_entity, damage_event.id_who_did_damage, damage_event.weapon_identifier, damage_event.damage, damage_event.damage_result);
            }
        }
    };

    for(const DamageEvent& damage_event : m_damage_events)
    {
        call_callbacks(damage_event, m_damage_callbacks[damage_event.id_damaged_entity]);
        call_callbacks(damage_event, m_global_damage_callbacks);
    }

    for(uint32_t entity_id = 0; entity_id < m_damage_records.size(); ++entity_id)
    {
        if(!m_active[entity_id])
            continue;

        const DamageRecord& damage_record = m_damage_records[entity_id];
        if(damage_record.health <= 0)
        {
            if(damage_record.release_entity_on_death)
                m_entity_manager->ReleaseEntity(entity_id);

            const std::string& explosion_entity = m_death_entities.front();
            game::SpawnEntityWithAnimation(
                explosion_entity.c_str(), 0, entity_id, m_entity_manager, m_transform_system, m_sprite_system);
            m_active[entity_id] = false;
        }
    }
}

void DamageSystem::PostUpdate()
{
    m_damage_events.clear();
}

uint32_t DamageSystem::FindFreeCallbackIndex(const DamageCallbacks& callbacks) const
{
    for(uint32_t index = 0; index < std::size(callbacks); ++index)
    {
        if(callbacks[index].callback == nullptr)
            return index;
    }

    return std::numeric_limits<uint32_t>::max();
}
