
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"

#include "PickupTypes.h"
#include "DamageSystem/DamageSystemTypes.h"
#include "Physics/PhysicsFwd.h"
#include "System/Audio.h"

#include "Util/ActiveVector.h"

#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <string>

namespace game
{
    class DamageSystem;

    struct Pickup
    {
        PickupType type;
        int amount;
    };

    struct LootBox
    {
        float value;

        // Internal
        uint32_t release_handle;
    };

    using PickupCallback = std::function<void (PickupType type, int amount)>;

    class PickupSystem : public mono::IGameSystem
    {
    public:
        
        PickupSystem(
            uint32_t n,
            game::DamageSystem* damage_system,
            mono::TransformSystem* transform_system,
            mono::ParticleSystem* particle_system,
            mono::PhysicsSystem* physics_system,
            mono::IEntityManager* entity_manager);

        void Begin() override;
        void Reset() override;

        Pickup* AllocatePickup(uint32_t id);
        void ReleasePickup(uint32_t id);
        void SetPickupData(uint32_t id, const Pickup& pickup_data);

        LootBox* AllocateLootBox(uint32_t id);
        void ReleaseLootBox(uint32_t id);
        void SetLootBoxData(uint32_t id, float temp);

        void HandlePickup(uint32_t pickup_id, uint32_t target_id);
        void RegisterPickupTarget(uint32_t target_id, PickupCallback callback);
        void UnregisterPickupTarget(uint32_t target_id);

        // IGameSystem
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        void HandleReleaseLootBox(uint32_t id);
        void HandleSpawnEnemyPickup(uint32_t id);
        void PlayPickupSound(PickupType type);

        game::DamageSystem* m_damage_system;
        mono::TransformSystem* m_transform_system;
        mono::ParticleSystem* m_particle_system;
        mono::PhysicsSystem* m_physics_system;
        mono::IEntityManager* m_entity_manager;

        mono::ActiveVector<Pickup> m_pickups;
        mono::ActiveVector<LootBox> m_lootboxes;
        std::vector<std::unique_ptr<mono::ICollisionHandler>> m_collision_handlers;
        std::unordered_map<uint32_t, PickupCallback> m_pickup_targets;

        struct PickupToTarget
        {
            uint32_t pickup_id;
            uint32_t target_id;
        };
        std::vector<PickupToTarget> m_pickups_to_process;

        struct PickupDefinition
        {
            std::string entity_file;
            float drop_chance_percentage;
        };
        std::vector<PickupDefinition> m_pickup_definitions;
        std::vector<PickupDefinition> m_lootbox_pickup_definitions;

        struct SpawnedPickup
        {
            uint32_t pickup_id;
            float lifetime;
        };
        std::vector<SpawnedPickup> m_spawned_pickups;

        audio::ISoundPtr m_pickup_sound;
        audio::ISoundPtr m_coins_sound;

        uint32_t m_damage_callback_id;
        class PickupEffect* m_pickup_effect;
        class PickupLootEffect* m_pickup_loot_effect;
    };
}
