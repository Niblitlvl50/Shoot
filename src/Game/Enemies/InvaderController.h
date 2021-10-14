
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "StateMachine.h"
#include "Weapons/IWeaponFactory.h"

#include <memory>

namespace game
{
    enum class InvaderStates
    {
        IDLE,
        TRACKING,
        ATTACK_ANTICIPATION,
        ATTACKING
    };

    class InvaderController : public IEntityLogic
    {
    public:

        InvaderController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        virtual ~InvaderController();

        void Update(const mono::UpdateContext& update_context) override;

    private:

        void ToIdle();
        void Idle(const mono::UpdateContext& update_context);

        void ToTracking();
        void Tracking(const mono::UpdateContext& update_context);

        void ToAttackAnticipation();
        void AttackAnticipation(const mono::UpdateContext& update_context);

        void ToAttacking();
        void Attacking(const mono::UpdateContext& update_context);

        const uint32_t m_entity_id;

        mono::TransformSystem* m_transform_system;
        mono::PhysicsSystem* m_physics_system;
        mono::SpriteSystem* m_sprite_system;
        mono::IEntityManager* m_entity_manager;

        IWeaponPtr m_weapon;
        std::unique_ptr<class TrackingBehaviour> m_tracking_behaviour;

        uint32_t m_idle_timer;
        using InvaderStateMachine = StateMachine<InvaderStates, const mono::UpdateContext&>;
        InvaderStateMachine m_states;

        const struct PlayerInfo* m_attack_target;
        int m_bullets_fired;
    };
}
