
#pragma once

#include "MonoFwd.h"
#include "Entity/IEntityLogic.h"
#include "Math/Vector.h"
#include "StateMachine.h"

#include <memory>
#include <string>

namespace game
{
    class ThrowableLogic : public game::IEntityLogic
    {
    public:

        ThrowableLogic(
            uint32_t entity_id,
            uint32_t owner_entity_id,
            const math::Vector& position,
            const math::Vector& target,
            const math::Vector& velocity,
            const struct BulletConfiguration& bullet_config,
            mono::TransformSystem* transform_system,
            mono::IEntityManager* entity_manager);
        void Update(const mono::UpdateContext& update_context) override;

    private:

        void ToThrowing();
        void Throwing(const mono::UpdateContext& update_context);
        void ToSpawning();
        void Spawning(const mono::UpdateContext& update_context);

        const uint32_t m_entity_id;
        const uint32_t m_owner_entity_id;
        const std::string m_spawned_entity;
        const math::Vector m_target;
        mono::TransformSystem* m_transform_system;
        mono::IEntityManager* m_entity_manager;

        math::Vector m_start_position;
        math::Vector m_move_delta;
        float m_meters_per_second;
        float m_life_span;
        float m_move_timer_s;
        float m_spawn_timer_s;

        enum class ThrowableStates
        {
            THROWING,
            SPAWNING,
        };

        using ThrowableStatemachine = StateMachine<ThrowableStates, const mono::UpdateContext&>;
        ThrowableStatemachine m_state;
    };
}
