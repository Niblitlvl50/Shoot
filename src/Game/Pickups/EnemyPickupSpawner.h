
#pragma once

#include "MonoFwd.h"
#include "DamageSystem.h"

#include <vector>
#include <string>

namespace game
{
    class EnemyPickupSpawner
    {
    public:
    
        EnemyPickupSpawner(
            DamageSystem* damage_system, mono::TransformSystem* transform_system, mono::IEntityManager* entity_manager);
        ~EnemyPickupSpawner();

        void HandleSpawnEnemyPickup(uint32_t id, int damage, uint32_t who_did_damage, DamageType type);

        DamageSystem* m_damage_system;
        mono::TransformSystem* m_transform_system;
        mono::IEntityManager* m_entity_manager;

        uint32_t m_damage_callback_id;

        struct PickupDefinition
        {
            std::string entity_file;
            float drop_chance_percentage;
        };
        std::vector<PickupDefinition> m_pickup_definitions;
    };
}
