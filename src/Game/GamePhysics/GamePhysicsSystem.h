
#pragma once

#include "IGameSystem.h"
#include "MonoFwd.h"
#include "Physics/PhysicsFwd.h"

#include <vector>
#include <memory>

namespace game
{
    struct ImpulseComponent
    {
        float impulse_strength;

        // Internal
        uint32_t entity_id;
        std::unique_ptr<mono::ICollisionHandler> collision_handler;
    };

    class GamePhysicsSystem : public mono::IGameSystem
    {
    public:

        GamePhysicsSystem(mono::TransformSystem* transform_system, mono::PhysicsSystem* physics_system);

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        ImpulseComponent* AllocateImpulse(uint32_t entity_id);
        void ReleaseImpulse(uint32_t entity_id);
        void UpdateImpulse(uint32_t entity_id, float impulse_strength);

        void ApplyImpulseFromIdTo(uint32_t id, mono::IBody* body);

        ImpulseComponent* FindImpulseComponent(uint32_t entity_id);

        mono::TransformSystem* m_transform_system;
        mono::PhysicsSystem* m_physics_system;

        std::vector<ImpulseComponent> m_impulse_components;
    };
}
