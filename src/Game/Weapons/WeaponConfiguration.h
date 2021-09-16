
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

    inline uint8_t StringToBulletCollisionFlag(const char* string)
    {
        if(strcmp(string, "bounce") == 0)
            return BulletCollisionFlag::BOUNCE;
        else if(strcmp(string, "jumper") == 0)
            return BulletCollisionFlag::JUMPER;
        else if(strcmp(string, "pass_through") == 0)
            return BulletCollisionFlag::PASS_THROUGH;

        return 0;
    }

    struct BulletConfiguration
    {
        std::string name;
        float life_span;
        float fuzzy_life_span;
        bool bullet_want_direction;
        std::string entity_file;
        std::string sound_file;
        uint32_t bullet_behaviour;
    };

    struct WeaponConfiguration
    {
        std::string name;
        int magazine_size;
        int projectiles_per_fire;
        float rounds_per_second;
        float fire_rate_multiplier;
        float max_fire_rate;
        float bullet_force;
        float bullet_spread_degrees;
        bool bullet_force_random;
        uint32_t reload_time_ms;
        std::string fire_sound;
        std::string out_of_ammo_sound;
        std::string reload_sound;
    };

    struct CollisionConfiguration
    {
        shared::CollisionCategory collision_category;
        uint32_t collision_mask;
        BulletImpactCallback collision_callback;
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
