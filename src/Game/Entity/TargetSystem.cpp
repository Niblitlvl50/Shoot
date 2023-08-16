
#include "TargetSystem.h"
#include "DamageSystem/DamageSystem.h"
#include "EntitySystem/Entity.h"
#include "Player/PlayerInfo.h"
#include "Util/Algorithm.h"

#include "TransformSystem/TransformSystem.h"

using namespace game;

namespace
{
    class TargetImpl : public game::ITarget
    {
    public:

        TargetImpl(uint32_t target_entity_id, DamageSystem* damage_system)
            : m_target_id(target_entity_id)
            , m_callback_handle(mono::INVALID_ID)
            , m_damage_system(damage_system)
        {
            if(IsValid())
            {
                const DamageCallback& on_destroyed = [this](uint32_t id, int damage, uint32_t who_did_damage, DamageType type) {
                    m_target_id = mono::INVALID_ID;
                };
                m_callback_handle = m_damage_system->SetDamageCallback(m_target_id, DamageType::DESTROYED, on_destroyed);
            }
        }

        ~TargetImpl()
        {
            if(m_callback_handle != mono::INVALID_ID)
                m_damage_system->RemoveDamageCallback(m_target_id, m_callback_handle);
        }

        uint32_t TargetId() const override
        {
            return m_target_id;
        }

        bool IsValid() const override
        {
            return m_target_id != mono::INVALID_ID;
        }

        uint32_t m_target_id;
        uint32_t m_callback_handle;
        DamageSystem* m_damage_system;
    };
}

TargetSystem::TargetSystem(const mono::TransformSystem* transform_system, DamageSystem* damage_system)
    : m_transform_system(transform_system)
    , m_damage_system(damage_system)
    , m_ai_target_behaviour(AITargetBehaviour::Player)
    , m_targets_dirty(false)
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
    target_component.priority = 0;
    m_targets.push_back(target_component);
}

void TargetSystem::ReleaseTarget(uint32_t entity_id)
{
    const auto remove_on_id = [entity_id](const TargetComponent& target) {
        return target.entity_id == entity_id;
    };
    mono::remove_if(m_targets, remove_on_id);
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

FindTargetResult TargetSystem::FindAITargetFromPosition(const math::Vector& world_position, float max_distance)
{
    FindTargetResult result;
    result.entity_id = mono::INVALID_ID;
    result.sees_target = false;

    switch(m_ai_target_behaviour)
    {
    case AITargetBehaviour::Player:
    {
        const game::PlayerInfo* player_info = game::GetClosestActivePlayer(world_position);
        if(player_info)
        {
            const float target_distance = math::DistanceBetween(player_info->position, world_position);
            if(target_distance < max_distance)
            {
                result.entity_id = player_info->entity_id;
                result.world_position = player_info->position;
            }
        }

        break;
    }
    case AITargetBehaviour::Package:
    {
        const bool package_spawned = (g_package_info.state != PackageState::NOT_SPAWNED);
        if(package_spawned)
        {
            result.entity_id = g_package_info.entity_id;
            result.world_position = g_package_info.position;
        }

        break;
    }
    }

    return result;
}

std::unique_ptr<game::ITarget> TargetSystem::AquireTarget(const math::Vector& world_position, float max_distance)
{
    uint32_t found_target_entity_id = mono::INVALID_ID;

    for(const TargetComponent& target : m_targets)
    {
        const math::Vector& target_world_position = m_transform_system->GetWorldPosition(target.entity_id);
        const float target_distance = math::DistanceBetween(target_world_position, world_position);
        if(target_distance < max_distance)
        {
            found_target_entity_id = target.entity_id;
            break;
        }
    }

    return std::make_unique<TargetImpl>(found_target_entity_id, m_damage_system);
}
