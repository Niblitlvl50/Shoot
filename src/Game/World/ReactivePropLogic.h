
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "StateMachine.h"
#include "Entity/IEntityLogic.h"
#include "Physics/IBody.h"

namespace game
{
    class ReactivePropLogic : public IEntityLogic, public mono::ICollisionHandler
    {
    public:
        ReactivePropLogic(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);
        void Update(const mono::UpdateContext& update_context) override;

        mono::CollisionResolve OnCollideWith(
            mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t categories) override;
        void OnSeparateFrom(mono::IBody* body) override;

        const uint32_t m_entity_id;
        mono::IEntityManager* m_entity_manager;
        mono::PhysicsSystem* m_physics_system;

        float m_alive_timer_s;
        bool m_ready_for_push;
        int m_pushed_times;
    };
}
