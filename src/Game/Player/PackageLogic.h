
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "StateMachine.h"
#include "Entity/IEntityLogic.h"

#include "EventHandler/EventToken.h"
#include "Physics/IBody.h"

namespace game
{
    struct PackageInfo;
    struct PackagePickupEvent;

    class PackageLogic : public IEntityLogic, public mono::ICollisionHandler
    {
    public:

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

        const uint32_t m_entity_id;
        game::PackageInfo* m_package_info;
        mono::EventHandler* m_event_handler;
        mono::TransformSystem* m_transform_system;
        mono::PhysicsSystem* m_physics_system;
        class DamageSystem* m_damage_system;

        mono::EventToken<PackagePickupEvent> m_pickup_event_token;

        float m_shield_timer_s;
        bool m_damage_on_impact;
    };
}
