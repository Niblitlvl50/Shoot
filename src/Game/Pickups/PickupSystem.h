
#pragma once

#include "IGameSystem.h"
#include "MonoFwd.h"

#include "PickupTypes.h"
#include "Physics/PhysicsFwd.h"

#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

class IEntityManager;

namespace game
{
    struct Pickup
    {
        shared::PickupType type;
        int amount;
    };

    using PickupCallback = std::function<void (shared::PickupType type, int amount)>;

    class PickupSystem : public mono::IGameSystem
    {
    public:
        
        PickupSystem(uint32_t n, mono::PhysicsSystem* physics_system, IEntityManager* entity_manager);

        Pickup* AllocatePickup(uint32_t id);
        void ReleasePickup(uint32_t id);
        void SetPickupData(uint32_t id, const Pickup& pickup_data);

        void HandlePickup(uint32_t pickup_id, uint32_t target_id);
        void RegisterPickupTarget(uint32_t target_id, PickupCallback callback);
        void UnregisterPickupTarget(uint32_t target_id);

        // IGameSystem
        uint32_t Id() const override;
        const char* Name() const override;
        uint32_t Capacity() const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        mono::PhysicsSystem* m_physics_system;
        IEntityManager* m_entity_manager;

        std::vector<Pickup> m_pickups;
        std::vector<bool> m_active;
        std::vector<std::unique_ptr<mono::ICollisionHandler>> m_collision_handlers;
        std::unordered_map<uint32_t, PickupCallback> m_pickup_targets;

        struct PickupToTarget
        {
            uint32_t pickup_id;
            uint32_t target_id;
        };

        std::vector<PickupToTarget> m_pickups_to_process;
    };
}
