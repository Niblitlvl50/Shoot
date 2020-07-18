
#pragma once

#include "IGameSystem.h"
#include "MonoFwd.h"

#include "PickupTypes.h"
#include "Physics/PhysicsFwd.h"

#include <vector>
#include <memory>

namespace game
{
    struct Pickup
    {
        shared::PickupType type;
        int amount;
    };

    class PickupSystem : public mono::IGameSystem
    {
    public:
        
        PickupSystem(uint32_t n, mono::PhysicsSystem* physics_system);

        Pickup* AllocatePickup(uint32_t id);
        void ReleasePickup(uint32_t id);
        void SetPickupData(uint32_t id, const Pickup& pickup_data);

        uint32_t Id() const override;
        const char* Name() const override;
        uint32_t Capacity() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void HandlePickup(uint32_t id, const Pickup* pickup);

    private:

        mono::PhysicsSystem* m_physics_system;

        std::vector<Pickup> m_pickups;
        std::vector<std::unique_ptr<mono::ICollisionHandler>> m_collision_handlers;
        std::vector<bool> m_active;
    };
}
