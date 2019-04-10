
#pragma once

#include "MonoFwd.h"
#include "Physics/PhysicsFwd.h"
#include "Rendering/Color.h"
#include "Particle/ParticleFwd.h"
#include "Particle/ParticleEmitter.h"
#include "CollisionConfiguration.h"
#include "Math/Vector.h"

#include <functional>

namespace game
{
    using BulletImpactCallback = std::function<void (uint32_t entity_id, mono::IBody* collide_with)>;

    struct BulletConfiguration
    {
        float life_span = 1.0f;
        float fuzzy_life_span = 0.0f;

        const char* entity_file = nullptr;
        const char* sound_file = nullptr;

        CollisionCategory collision_category = CollisionCategory::STATIC;
        uint32_t collision_mask = 0;
        BulletImpactCallback collision_callback;

        //mono::ParticlePool* pool = nullptr;
        //mono::ParticleEmitter::Configuration emitter_config;
    };

    struct WeaponConfiguration
    {
        int magazine_size = 10;
        int projectiles_per_fire = 1;
        float rounds_per_second = 1.0f;
        float fire_rate_multiplier = 1.0f;
        float max_fire_rate = 1.0f;
        float bullet_force = 1.0f;
        float bullet_spread = 0.0f; // In degrees
        
        const char* fire_sound = nullptr;
        const char* out_of_ammo_sound = nullptr;
        const char* reload_sound = nullptr;

        BulletConfiguration bullet_config;
    };
}
