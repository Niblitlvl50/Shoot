
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "StateMachine.h"
#include "Entity/IEntityLogic.h"

#include "EventHandler/EventToken.h"
#include "Physics/IBody.h"
#include "StateMachine.h"

namespace game
{
    struct PackageInfo;
    struct PackagePickupEvent;

    class PackageLogic : public IEntityLogic, public mono::ICollisionHandler
    {
    public:

        enum class States
        {
            IDLE,
            SHIELDED,
            THROWN
        };

        PackageLogic(
            uint32_t entity_id,
            game::PackageInfo* package_info,
            mono::EventHandler* event_handler,
            mono::SystemContext* system_context);
        ~PackageLogic();

        void Update(const mono::UpdateContext& update_context) override;
        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

        mono::CollisionResolve OnCollideWith(
            mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t categories) override;
        void OnSeparateFrom(mono::IBody* body) override;

        mono::EventResult OnPackageEvent(const PackagePickupEvent& event);

        bool IsShieldOnCooldown() const;
        const char* StatesToString(States state) const;

        void ToIdle();
        void Idle(const mono::UpdateContext& update_context);

        void ToShielded();
        void Shielded(const mono::UpdateContext& update_context);
        void ExitShielded();

        void ToThrown();
        void Thrown(const mono::UpdateContext& update_context);

        const uint32_t m_entity_id;
        game::PackageInfo* m_package_info;
        mono::EventHandler* m_event_handler;
        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        mono::PhysicsSystem* m_physics_system;
        mono::IEntityManager* m_entity_manager;
        class DamageSystem* m_damage_system;

        uint32_t m_spawned_shield_id;
        mono::EventToken<PackagePickupEvent> m_pickup_event_token;

        using PackageStateMachine = StateMachine<States, const mono::UpdateContext&>;
        PackageStateMachine m_states;

        float m_shield_timer_s;
        float m_shield_cooldown_s;
    };
}
