
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "Entity/TargetTypes.h"
#include "Physics/IBody.h"
#include "StateMachine.h"

#include "Behaviour/HomingBehaviour.h"
#include "Behaviour/TrackingBehaviour.h"

#include <memory>

namespace game
{
    class EyeMonsterController : public IEntityLogic, public mono::ICollisionHandler
    {
    public:

        EyeMonsterController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);
        virtual ~EyeMonsterController();

        void Update(const mono::UpdateContext& update_context) override;
        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

        mono::CollisionResolve OnCollideWith(
            mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t category) override;
        void OnSeparateFrom(mono::IBody* body) override;

    private:

        enum class States
        {
            SLEEPING,
            AWAKE,
            RETARGET,
            TRACKING,
            HUNT
        };

        void ToSleep();
        void SleepState(const mono::UpdateContext& update_context);

        void ToAwake();
        void AwakeState(const mono::UpdateContext& update_context);

        void ToRetarget();
        void RetargetState(const mono::UpdateContext& update_context);

        void ToTracking();
        void TrackingState(const mono::UpdateContext& update_context);

        void ToHunt();
        void HuntState(const mono::UpdateContext& update_context);
        void ExitHunt();

        mono::TransformSystem* m_transform_system;
        mono::IEntityManager* m_entity_manager;
        mono::PhysicsSystem* m_physics_system;
        class NavigationSystem* m_navigation_system;
        class DamageSystem* m_damage_system;
        class TargetSystem* m_target_system;

        const uint32_t m_entity_id;
        float m_visibility_check_timer_s;
        float m_retarget_timer_s;
        bool m_force_update_path;

        using MyStateMachine = StateMachine<States, const mono::UpdateContext&>;
        MyStateMachine m_states;
        HomingBehaviour m_homing_movement;
        TrackingBehaviour m_tracking_movement;
        mono::ISprite* m_sprite;
        ITargetPtr m_aquired_target;
    };
}
