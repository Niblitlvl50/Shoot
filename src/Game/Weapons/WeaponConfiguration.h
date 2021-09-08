
#pragma once

#include "Physics/PhysicsFwd.h"
#include "Particle/ParticleFwd.h"
#include "CollisionConfiguration.h"
#include "Math/Vector.h"

#include <string>
#include <functional>

#define ENUM_BIT(n) (1 << (n))

namespace game
{
    enum BulletImpactFlag
    {
        APPLY_DAMAGE = ENUM_BIT(0),
        DESTROY_THIS = ENUM_BIT(1),
    };

    struct CollisionDetails
    {
        mono::IBody* body;
        math::Vector point;
        math::Vector normal;
    };

    // entity_id is the bullet that collides with something
    // owner_entity_id is the owner of this bullet
    // imact_flags is what to do on impact
    // details contains more data on the collision
    using BulletImpactCallback =
        std::function<void (uint32_t entity_id, uint32_t owner_entity_id, BulletImpactFlag impact_flags, const CollisionDetails& details)>;

    enum BulletCollisionFlag : uint8_t
    {
        BOUNCE          = ENUM_BIT(0),
        JUMPER          = ENUM_BIT(1),
        PASS_THROUGH    = ENUM_BIT(2),
    };

    struct BulletConfiguration
    {
        float life_span = 1.0f;
        float fuzzy_life_span = 0.0f;

        std::string entity_file;
        std::string sound_file;

        uint32_t bullet_behaviour = 0;

        shared::CollisionCategory collision_category = shared::CollisionCategory::STATIC;
        uint32_t collision_mask = 0;
        BulletImpactCallback collision_callback;
    };

    struct WeaponConfiguration
    {
        int magazine_size = 10;
        int projectiles_per_fire = 1;
        float rounds_per_second = 1.0f;
        float fire_rate_multiplier = 1.0f;
        float max_fire_rate = 1.0f;
        float bullet_force = 1.0f;
        float bullet_spread_degrees = 0.0f;
        bool bullet_force_random = false;
        bool bullet_want_direction = false;

        uint32_t reload_time_ms = 1000;
        
        std::string fire_sound;
        std::string out_of_ammo_sound;
        std::string reload_sound;
    };

    struct ThrowableWeaponConfig
    {
        int magazine_size = 10;
        int projectiles_per_fire = 1;
        float cooldown_seconds = 2.0f;
        uint32_t reload_time_ms = 1000;
        float max_distance = 5.0f;
        float target_accuracy = 0.0f; // Radius

        // Throwable data
        const char* thrown_entity = nullptr;
        const char* spawned_entity = nullptr;

        shared::CollisionCategory collision_category = shared::CollisionCategory::STATIC;
        uint32_t collision_mask = 0;
        BulletImpactCallback collision_callback;
    };
}
