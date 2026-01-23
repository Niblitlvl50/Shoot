
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "Entity/TargetTypes.h"
#include "Physics/IBody.h"
#include "StateMachine.h"

#include "Behaviour/TrackingBehaviour.h"
#include "Behaviour/PathBehaviour.h"

#include <memory>

namespace game
{
    class GolemTinyController : public IEntityLogic, public mono::ICollisionHandler
    {
    public:

        GolemTinyController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);
        virtual ~GolemTinyController();

        void Update(const mono::UpdateContext& update_context) override;
        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

        mono::CollisionResolve OnCollideWith(
            mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t category) override;
        void OnSeparateFrom(mono::IBody* body) override;

    private:

        enum class States
        {
            IDLE,
            WANDER,
            TRACKING,
            STOMP_ATTACK,
            ROLL_ATTACK,
        };

        void ToIdle();
        void IdleState(const mono::UpdateContext& update_context);

        void ToWander();

        void ToTracking();
        void ExitTracking();

        void TrackingState(const mono::UpdateContext& update_context);

        void ToStompAttack();
        void StompAttackState(const mono::UpdateContext& update_context);
        
        void ToRollAttack();
        void RollAttackState(const mono::UpdateContext& update_context);

        void ExitAttack();

        mono::TransformSystem* m_transform_system;
        mono::IEntityManager* m_entity_manager;
        mono::PhysicsSystem* m_physics_system;
        class NavigationSystem* m_navigation_system;
        class DamageSystem* m_damage_system;
        class TargetSystem* m_target_system;

        const uint32_t m_entity_id;
        float m_visibility_check_timer_s;
        float m_retarget_timer_s;

        bool m_update_spawn_position = true;
        math::Vector m_spawn_position;

        bool m_perform_roll_attack = false;

        using MyStateMachine = StateMachine<States, const mono::UpdateContext&>;
        MyStateMachine m_states;

        TrackingBehaviour m_tracking_movement;
        PathBehaviour m_path_behaviour;

        mono::ISprite* m_sprite;
        ITargetPtr m_aquired_target;
    };
}
