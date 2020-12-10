
#pragma once

#include "MonoFwd.h"
#include "Entity/IEntityLogic.h"
#include "Math/Vector.h"
#include "StateMachine.h"

#include <memory>

namespace game
{
    class ThrowableLogic : public game::IEntityLogic
    {
    public:

        ThrowableLogic(
            uint32_t entity_id,
            const char* spawned_entity,
            const math::Vector& position,
            const math::Vector& target,
            mono::TransformSystem* transform_system,
            mono::SpriteSystem* sprite_system,
            mono::ParticleSystem* particle_system,
            mono::IEntityManager* entity_manager);
        void Update(const mono::UpdateContext& update_context) override;

    private:

        void ToThrowing();
        void Throwing(const mono::UpdateContext& update_context);
        void ToSpawning();
        void Spawning(const mono::UpdateContext& update_context);

        const uint32_t m_entity_id;
        const char* m_spawned_entity;
        const math::Vector m_target;
        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        mono::IEntityManager* m_entity_manager;

        math::Vector m_start_position;
        math::Vector m_move_delta;
        uint32_t m_move_timer;
        uint32_t m_spawn_timer;

        std::unique_ptr<class SmokeEffect> m_smoke_effect;

        enum class ThrowableStates
        {
            THROWING,
            SPAWNING,
        };

        using ThrowableStatemachine = StateMachine<ThrowableStates, const mono::UpdateContext&>;
        ThrowableStatemachine m_state;
    };
}
