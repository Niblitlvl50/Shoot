
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include "Math/Vector.h"
#include "TargetTypes.h"

#include <vector>
#include <memory>

namespace game
{
    enum class AITargetBehaviour : uint32_t
    {
        Player,
        Package
    };

    struct TargetComponent
    {
        uint32_t entity_id;
        bool enabled;
        int priority;
    };

    class ITarget
    {
    public:

        virtual ~ITarget() = default;

        virtual uint32_t TargetId() const = 0;
        virtual bool IsValid() const = 0;
        virtual const math::Vector Position() const = 0;
    };

    class TargetSystem : public mono::IGameSystem
    {
    public:

        TargetSystem(const mono::TransformSystem* transform_system, mono::PhysicsSystem* physics_system, class DamageSystem* damage_system);
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void AllocateTarget(uint32_t entity_id);
        void ReleaseTarget(uint32_t entity_id);
        void SetTargetData(uint32_t entity_id, int priority);

        void SetTargetEnabled(uint32_t entity_id, bool enabled);

        ITargetPtr AquireTarget(const math::Vector& world_position, float max_distance);
        bool SeesTarget(uint32_t entity_id, const ITarget* target);

    private:
        const mono::TransformSystem* m_transform_system;
        mono::PhysicsSystem* m_physics_system;
        class DamageSystem* m_damage_system;
        AITargetBehaviour m_ai_target_behaviour;

        bool m_targets_dirty;
        std::vector<TargetComponent> m_targets;
    };
}
