
#pragma once

#include "MonoFwd.h"
#include "Physics/CMFwd.h"
#include "Rendering/Color.h"
#include "Particle/ParticleFwd.h"
#include "Particle/ParticleEmitter.h"
#include "CollisionConfiguration.h"
#include "Math/Vector.h"

#include <functional>

namespace game
{
    using BulletImpactCallback = std::function<void (const mono::IPhysicsEntity*, mono::IBody*)>;

    struct BulletConfiguration
    {
        float life_span = 1.0f;
        float fuzzy_life_span = 0.0f;
        float mass = 1.0f;

        float collision_radius = 0.5f;
        math::Vector scale = math::Vector(1.0f, 1.0f);
        CollisionCategory collision_category = CollisionCategory::NONE;
        unsigned int collision_mask = 0;
        BulletImpactCallback collision_callback;

        mono::Color::RGBA sprite_shade;
        const char* sprite_file = nullptr;
        const char* sound_file = nullptr;

        mono::ParticlePool* pool = nullptr;
        mono::ParticleEmitter::Configuration emitter_config;
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
