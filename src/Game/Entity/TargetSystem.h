
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include "Math/Vector.h"

#include <vector>
#include <memory>

namespace game
{
    struct FindTargetResult
    {
        uint32_t entity_id;
        math::Vector world_position;
        bool sees_target;
    };

    enum class AITargetBehaviour : uint32_t
    {
        Player,
        Package
    };

    struct TargetComponent
    {
        uint32_t entity_id;
        int priority;
    };

    class ITarget
    {
    public:

        virtual ~ITarget() = default;

        virtual uint32_t TargetId() const = 0;
        virtual bool IsValid() const = 0;
    };

    class TargetSystem : public mono::IGameSystem
    {
    public:

        TargetSystem(const mono::TransformSystem* transform_system, class DamageSystem* damage_system);
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void AllocateTarget(uint32_t entity_id);
        void ReleaseTarget(uint32_t entity_id);
        void SetTargetData(uint32_t entity_id, int priority);

        FindTargetResult FindAITargetFromPosition(const math::Vector& world_position, float max_distance);

        std::unique_ptr<ITarget> AquireTarget(const math::Vector& world_position, float max_distance);

        const mono::TransformSystem* m_transform_system;
        class DamageSystem* m_damage_system;
        AITargetBehaviour m_ai_target_behaviour;

        bool m_targets_dirty;
        std::vector<TargetComponent> m_targets;
    };
}
