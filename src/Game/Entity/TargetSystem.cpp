
#include "TargetSystem.h"
#include "DamageSystem/DamageSystem.h"
#include "EntitySystem/Entity.h"
#include "Util/Algorithm.h"
#include "CollisionConfiguration.h"

#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;

namespace game
{
    class TargetImpl : public game::ITarget
    {
    public:

        TargetImpl(uint32_t target_entity_id, const mono::TransformSystem* transform_system)
            : m_target_id(target_entity_id)
            , m_transform_system(transform_system)
        { }

        uint32_t TargetId() const override
        {
            return m_target_id;
        }

        bool IsValid() const override
        {
            return (m_target_id != mono::INVALID_ID);
        }

        const math::Vector Position() const override
        {
            return m_transform_system->GetWorldPosition(m_target_id);
        }

        bool IsWithinRange(const math::Vector& position, float distance) const override
        {
            return math::DistanceBetweenSquared(Position(), position) <= (distance * distance);
        }

        void InvalidateTarget()
        {
            m_target_id = mono::INVALID_ID;
        }

        uint32_t m_target_id;
        const mono::TransformSystem* m_transform_system;
    };
}

TargetSystem::TargetSystem(const mono::TransformSystem* transform_system, mono::PhysicsSystem* physics_system, DamageSystem* damage_system)
    : m_transform_system(transform_system)
    , m_physics_system(physics_system)
    , m_damage_system(damage_system)
    , m_targets_dirty(false)
    , m_global_target_mode(EnemyTargetMode::Normal)
{ }

const char* TargetSystem::Name() const
{
    return "TargetSystem";
}

void TargetSystem::Update(const mono::UpdateContext& update_context)
{
    if(m_targets_dirty)
    {
        const auto sort_by_id = [](const TargetComponent& left, const TargetComponent& right) {
            return left.priority < right.priority;
        };
        std::sort(m_targets.begin(), m_targets.end(), sort_by_id);

        m_targets_dirty = false;
    }
}

void TargetSystem::AllocateTarget(uint32_t entity_id)
{
    TargetComponent target_component;
    target_component.entity_id = entity_id;
    target_component.enabled = true;
    target_component.priority = 0;

    const DamageCallback& on_destroyed = [this](uint32_t id, int damage, uint32_t who_did_damage, DamageType type) {
        const auto it = m_active_targets.find(id);
        if(it != m_active_targets.end())
        {
            it->second->InvalidateTarget();
            m_active_targets.erase(id);
        }
    };
    target_component.callback_handle = m_damage_system->SetDamageCallback(entity_id, DamageType::DESTROYED, on_destroyed);

    m_targets.push_back(target_component);
}

void TargetSystem::ReleaseTarget(uint32_t entity_id)
{
    const auto remove_on_id = [this, entity_id](const TargetComponent& target) {
        const bool is_target = target.entity_id == entity_id;
        if(is_target)
            m_damage_system->RemoveDamageCallback(entity_id, target.callback_handle);

        return is_target;
    };
    mono::remove_if(m_targets, remove_on_id);

    m_active_targets.erase(entity_id);
}

void TargetSystem::SetTargetData(uint32_t entity_id, int priority)
{
    const auto find_on_id = [entity_id](const TargetComponent& target) {
        return target.entity_id == entity_id;
    };
    TargetComponent* component = mono::find_if(m_targets, find_on_id);
    if(component)
    {
        component->priority = priority;
    }

    m_targets_dirty = true;
}

void TargetSystem::SetTargetEnabled(uint32_t entity_id, bool enabled)
{
    const auto find_on_id = [entity_id](const TargetComponent& target) {
        return target.entity_id == entity_id;
    };
    TargetComponent* component = mono::find_if(m_targets, find_on_id);
    if(component)
        component->enabled = enabled;
}

void TargetSystem::SetGlobalTargetMode(EnemyTargetMode target_mode)
{
    m_global_target_mode = target_mode;
}

ITargetPtr TargetSystem::AquireTarget(TargetFaction faction, const math::Vector& world_position, float max_distance)
{
    uint32_t found_target_entity_id = mono::INVALID_ID;

    for(const TargetComponent& target : m_targets)
    {
        if(!target.enabled)
            continue;

        if(m_global_target_mode == EnemyTargetMode::Normal)
        {
            const math::Vector& target_world_position = m_transform_system->GetWorldPosition(target.entity_id);
            const float target_distance = math::DistanceBetween(target_world_position, world_position);
            if(target_distance < max_distance)
            {
                found_target_entity_id = target.entity_id;
                break;
            }
        }
        else
        {
            found_target_entity_id = target.entity_id;
            break;
        }
    }

    const auto it = m_active_targets.find(found_target_entity_id);
    if(it != m_active_targets.end())
        return it->second;

    return MakeAndCacheTarget(found_target_entity_id);
}

bool TargetSystem::SeesTarget(uint32_t entity_id, const ITarget* target)
{
    const math::Vector& origin_world_position = m_transform_system->GetWorldPosition(entity_id);
    const math::Vector& target_world_position = m_transform_system->GetWorldPosition(target->TargetId());

    const uint32_t query_category = CollisionCategory::PLAYER | CollisionCategory::STATIC;
    const mono::PhysicsSpace* space = m_physics_system->GetSpace();
    const mono::QueryResult result = space->QueryFirst(origin_world_position, target_world_position, query_category);
    return (result.body != nullptr && result.collision_category & CollisionCategory::PLAYER);
}

std::vector<ITargetPtr> TargetSystem::GetActiveTargets() const
{
    std::vector<ITargetPtr> targets;
    for(const auto& pair : m_active_targets)
    {
        if(pair.second.use_count() > 1)
            targets.push_back(pair.second);
    }
    return targets;    
}

ITargetPtr TargetSystem::MakeAndCacheTarget(uint32_t target_entity_id)
{
    std::shared_ptr<TargetImpl> new_target = std::make_shared<TargetImpl>(target_entity_id, m_transform_system);
 
    if(target_entity_id != mono::INVALID_ID)
        m_active_targets[target_entity_id] = new_target;

    return new_target;
}
