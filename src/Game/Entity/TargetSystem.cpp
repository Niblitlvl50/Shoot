
#include "TargetSystem.h"
#include "EntitySystem/Entity.h"
#include "Player/PlayerInfo.h"

#include "Util/Algorithm.h"

using namespace game;

namespace
{
    class TargetImpl : public game::ITarget
    {
    public:

        TargetImpl()
        {

        }

        ~TargetImpl()
        {

        }

        uint32_t TargetId() const override
        {
            return 0;
        }

        bool IsValid() const override
        {
            return false;
        }

    };
}

TargetSystem::TargetSystem(const mono::TransformSystem* transform_system)
    : m_transform_system(transform_system)
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
    return std::make_unique<TargetImpl>();
}
