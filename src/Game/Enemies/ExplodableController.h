
#pragma once

#include "MonoFwd.h"
#include "System/Audio.h"

#include "Entity/IEntityLogic.h"
#include "StateMachine.h"

#include <memory>

namespace game
{
    class DamageSystem;

    class ExplodableController : public IEntityLogic
    {
    public:

        ExplodableController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);

        void Update(const mono::UpdateContext& update_context) override;

        void OnIdle();
        void Idle(const mono::UpdateContext& update_context);

        void OnDead();
        void Dead(const mono::UpdateContext& update_context);

        uint32_t m_entity_id;
        mono::TransformSystem* m_transform_system;
        mono::PhysicsSystem* m_physics_system;
        mono::SpriteSystem* m_sprite_system;
        mono::IEntityManager* m_entity_system;

        game::DamageSystem* m_damage_system;

        std::unique_ptr<class ExplosionEffect> m_explosion_effect;
        audio::ISoundPtr m_explosion_sound;

        enum class ExplodableStates
        {
            IDLE,
            ATTACK,
            DEAD,
        };

        using ExplodableStateMachine = StateMachine<ExplodableStates, const mono::UpdateContext&>;
        ExplodableStateMachine m_states;

        uint32_t m_wait_timer;
    };
}
