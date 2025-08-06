
#pragma once

#include "Physics/PhysicsFwd.h"
#include "Particle/ParticleFwd.h"
#include "DamageSystem/DamageSystemTypes.h"
#include "CollisionConfiguration.h"
#include "Math/Vector.h"

#include <string>
#include <functional>
#include <cstring>

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
        int32_t material;
    };

    // bullet_entity_id is the bullet that collides with something
    // owner_entity_id is the owner of this bullet
    // weapon_identifier_hash is the weapon setup hash
    // damage is how much damage to apply
    // impact_entity optionally an entity to spawn on impact
    // imact_flags is what to do on impact
    // details contains more data on the collision
    using BulletImpactCallback =
        std::function<void (
            uint32_t bullet_entity_id,
            uint32_t owner_entity_id,
            uint32_t weapon_identifier_hash,
            const char* impact_entity,
            BulletImpactFlag impact_flags,
            const DamageDetails& damage_details,
            const CollisionDetails& details)>;

    enum BulletCollisionFlag : uint8_t
    {
        BOUNCE          = ENUM_BIT(0),
        JUMPER          = ENUM_BIT(1),
        PASS_THROUGH    = ENUM_BIT(2),
        EXPLODES        = ENUM_BIT(3),
        VAMPERIC        = ENUM_BIT(4),
    };

    enum BulletMovementFlag : uint8_t
    {
        HOMING          = ENUM_BIT(0),
        CIRCULATING     = ENUM_BIT(1),
        SINEWAVE        = ENUM_BIT(2),
        ARC_TRAJECTORY  = ENUM_BIT(3),
    };

    inline uint8_t StringToBulletCollisionFlag(const char* string)
    {
        if(std::strcmp(string, "bounce") == 0)
            return BulletCollisionFlag::BOUNCE;
        else if(std::strcmp(string, "jumper") == 0)
            return BulletCollisionFlag::JUMPER;
        else if(std::strcmp(string, "pass_through") == 0)
            return BulletCollisionFlag::PASS_THROUGH;
        else if(std::strcmp(string, "explodes") == 0)
            return BulletCollisionFlag::EXPLODES;
        else if(std::strcmp(string, "vameric") == 0)
            return BulletCollisionFlag::VAMPERIC;
        
        return 0;
    }

    inline uint8_t StringToBulletMovementFlag(const char* string)
    {
        if(std::strcmp(string, "homing") == 0)
            return BulletMovementFlag::HOMING;
        else if(std::strcmp(string, "circulating") == 0)
            return BulletMovementFlag::CIRCULATING;
        else if(std::strcmp(string, "sinewave") == 0)
            return BulletMovementFlag::SINEWAVE;
        else if(std::strcmp(string, "arc_trajectory") == 0)
            return BulletMovementFlag::ARC_TRAJECTORY;

        return 0;
    }

    struct BulletConfiguration
    {
        std::string name;
        int min_damage;
        int max_damage;
        int critical_hit_chance;
        float life_span;
        float fuzzy_life_span;
        bool bullet_want_direction;
        std::string entity_file;
        std::string impact_entity_file;
        std::string sound_file;

        uint32_t bullet_collision_behaviour;
        uint32_t bullet_movement_behaviour;
    };

    struct WeaponConfiguration
    {
        std::string name;
        int magazine_size;
        bool infinite_ammo;
        int projectiles_per_fire;
        float rounds_per_second;
        float fire_rate_multiplier;
        float max_fire_rate;
        float bullet_velocity;
        float bullet_spread_degrees;
        float bullet_offset;
        bool bullet_velocity_random;
        bool auto_reload;
        float reload_time;
        std::string fire_sound;
        std::string out_of_ammo_sound;
        std::string reload_sound;
        std::string reload_finished_sound;
    };

    struct WeaponBulletCombination
    {
        std::string name;
        std::string weapon;
        std::string bullet;
        std::string sprite_file;
    };

    struct CollisionConfiguration
    {
        CollisionCategory collision_category;
        uint32_t collision_mask;
        BulletImpactCallback collision_callback;
    };
}
