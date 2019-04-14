
#include "WeaponFactory.h"
#include "Weapons/Weapon.h"
#include "Math/MathFwd.h"
#include "Math/MathFunctions.h"

#include "Explosion.h"
#include "Physics/IBody.h"
#include "Entity/IPhysicsEntity.h"
#include "Particle/ParticlePool.h"

#include "EventHandler/EventHandler.h"
#include "Events/DamageEvent.h"
#include "Events/SpawnEntityEvent.h"
#include "Events/RemoveEntityEvent.h"
#include "Events/ShockwaveEvent.h"

#include "RenderLayers.h"

#include "SystemContext.h"
#include "DamageSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Entity/IEntityManager.h"

#include <functional>
#include <limits>

namespace
{
    void StandardCollision(
        uint32_t entity_id,
        const mono::IBody* other,
        IEntityManager* entity_manager,
        game::DamageSystem* damage_system,
        mono::PhysicsSystem* physics_system)
    {
        const uint32_t other_entity_id = physics_system->GetIdFromBody(other);
        if(other_entity_id != std::numeric_limits<uint32_t>::max())
            damage_system->ApplyDamage(other_entity_id, 10);
        entity_manager->ReleaseEntity(entity_id);
    }

    /*
    void RocketCollision(const mono::IPhysicsEntity* bullet, const mono::IBody* other, mono::EventHandler& event_handler)
    {
        game::ExplosionConfiguration explosion_config;
        explosion_config.position = bullet->Position();
        explosion_config.scale = 2.0f;
        explosion_config.rotation = 0.0f;
        explosion_config.sprite_file = "res/sprites/explosion.sprite";

        const game::SpawnEntityEvent spawn_event(
            std::make_shared<game::Explosion>(explosion_config, event_handler), game::GAMEOBJECTS);
        event_handler.DispatchEvent(spawn_event);
        event_handler.DispatchEvent(game::DamageEvent(other, 50, bullet->Rotation()));
        event_handler.DispatchEvent(game::ShockwaveEvent(explosion_config.position, 150));
        event_handler.DispatchEvent(game::RemoveEntityEvent(bullet->Id()));
    }

    void CacoPlasmaCollision(
        const mono::IPhysicsEntity* bullet, const mono::IBody* other, mono::EventHandler& event_handler)
    {
        game::ExplosionConfiguration explosion_config;
        explosion_config.position = bullet->Position();
        explosion_config.scale = 1.0f;
        explosion_config.rotation = 0.0f;
        explosion_config.sprite_file = "res/sprites/caco_explosion.sprite";

        const game::SpawnEntityEvent spawn_event(
            std::make_shared<game::Explosion>(explosion_config, event_handler), game::GAMEOBJECTS);
        event_handler.DispatchEvent(spawn_event);
        event_handler.DispatchEvent(game::DamageEvent(other, 20, bullet->Rotation()));
        event_handler.DispatchEvent(game::RemoveEntityEvent(bullet->Id()));
    }

    void FlakCannonCollision(
        const mono::IPhysicsEntity* bullet, mono::IBody* other, mono::EventHandler& event_handler)
    {
        const float direction = bullet->Rotation();

        const math::Vector& impulse = math::VectorFromAngle(direction) * 100.0f;
        other->ApplyImpulse(impulse, other->GetPosition());

        event_handler.DispatchEvent(game::DamageEvent(other, 20, direction));
        event_handler.DispatchEvent(game::RemoveEntityEvent(bullet->Id()));
    }

    void PlasmaParticleGenerator(const math::Vector& position, mono::ParticlePool& pool, size_t index)
    {
        constexpr int life = 100;

        pool.m_position[index] = position;
        pool.m_start_color[index] = mono::Color::RGBA(0.3f, 0.3f, 1.0f, 1.0f);
        pool.m_end_color[index] = mono::Color::RGBA(0.0f, 0.0f, 0.0f, 0.1f);
        pool.m_start_size[index] = 10.0f;
        pool.m_end_size[index] = 5.0f;
        pool.m_start_life[index] = life;
        pool.m_life[index] = life;
    }

    void RocketParticleGenerator(const math::Vector& position, mono::ParticlePool& pool, size_t index)
    {
        constexpr int life = 200;

        pool.m_position[index] = position;
        pool.m_start_color[index] = mono::Color::RGBA(0.2f, 0.3f, 0.3f, 1.0f);
        pool.m_end_color[index] = mono::Color::RGBA(0.0f, 0.0f, 0.0f, 0.1f);
        pool.m_start_size[index] = 24.0f;
        pool.m_end_size[index] = 16.0f;
        pool.m_start_life[index] = life;
        pool.m_life[index] = life;        
    }

    void FlakParticleGenerator(const math::Vector& position, mono::ParticlePool& pool, size_t index)
    {
        constexpr int life = 100;

        pool.m_position[index] = position;
        pool.m_start_color[index] = mono::Color::RGBA(1.0f, 0.5f, 0.0f, 1.0f);
        pool.m_end_color[index] = mono::Color::RGBA(0.0f, 0.0f, 0.0f, 0.1f);
        pool.m_start_size[index] = 15.0f;
        pool.m_end_size[index] = 10.0f;
        pool.m_start_life[index] = life;
        pool.m_life[index] = life;        
    }
    */
    
}

using namespace game;

WeaponFactory::WeaponFactory(IEntityManager* entity_manager, mono::SystemContext* system_context)
    : m_entity_manager(entity_manager)
    , m_system_context(system_context)
{ }

std::unique_ptr<game::IWeaponSystem> WeaponFactory::CreateWeapon(WeaponType weapon, WeaponFaction faction)
{
    return CreateWeapon(weapon, faction, nullptr);
}

std::unique_ptr<IWeaponSystem> WeaponFactory::CreateWeapon(WeaponType weapon, WeaponFaction faction, mono::ParticlePool* pool)
{
    using namespace std::placeholders;

    DamageSystem* damage_system = m_system_context->GetSystem<DamageSystem>();
    mono::PhysicsSystem* physics_system = m_system_context->GetSystem<mono::PhysicsSystem>();

    WeaponConfiguration weapon_config;
    BulletConfiguration& bullet_config = weapon_config.bullet_config;

    const bool enemy_weapon = (faction == WeaponFaction::ENEMY);
    bullet_config.collision_category = enemy_weapon ? CollisionCategory::ENEMY_BULLET : CollisionCategory::PLAYER_BULLET;
    bullet_config.collision_mask = enemy_weapon ? ENEMY_BULLET_MASK : PLAYER_BULLET_MASK;
    
    //bullet_config.pool = pool;

    switch(weapon)
    {
        case game::WeaponType::STANDARD:
        {
            bullet_config.life_span = 10.0f;
            bullet_config.fuzzy_life_span = 0;
            bullet_config.collision_callback
                = std::bind(StandardCollision, _1, _2, m_entity_manager, damage_system, physics_system);
            bullet_config.entity_file = "res/entities/green_blob.entity";
            bullet_config.sound_file = nullptr;

            //bullet_config.emitter_config.duration = -1;
            //bullet_config.emitter_config.emit_rate = 100.0f;
            //bullet_config.emitter_config.burst = false;
            //bullet_config.emitter_config.generator = PlasmaParticleGenerator;

            weapon_config.magazine_size = 99;
            weapon_config.rounds_per_second = 7.0f;
            weapon_config.fire_rate_multiplier = 1.1f;
            weapon_config.max_fire_rate = 2.0f;
            weapon_config.bullet_force = 20.0f;
            weapon_config.bullet_spread = 2.0f;
            weapon_config.fire_sound = "res/sound/plasma_fire.wav";
            weapon_config.reload_sound = "res/sound/shotgun_reload2.wav";

            break;
        }

        case game::WeaponType::ROCKET_LAUNCHER:
        {
            bullet_config.life_span = 2.0f;
            bullet_config.fuzzy_life_span = 0.3f;
            bullet_config.collision_callback
                = std::bind(StandardCollision, _1, _2, m_entity_manager, damage_system, physics_system);
            bullet_config.entity_file = "res/entities/green_blob.entity";
            bullet_config.sound_file = nullptr;

            //bullet_config.emitter_config.duration = -1;
            //bullet_config.emitter_config.emit_rate = 100.0f;
            //bullet_config.emitter_config.burst = false;
            //bullet_config.emitter_config.generator = RocketParticleGenerator;

            weapon_config.magazine_size = 5;
            weapon_config.rounds_per_second = 1.5f;
            weapon_config.bullet_force = 10.0f;
            weapon_config.fire_sound = "res/sound/rocket_fire2.wav";
            weapon_config.reload_sound = "res/sound/shotgun_reload2.wav";

            break;
        }

        case game::WeaponType::CACOPLASMA:
        {
            bullet_config.life_span = 1.0f;
            bullet_config.fuzzy_life_span = 0.3f;
            bullet_config.collision_callback
                = std::bind(StandardCollision, _1, _2, m_entity_manager, damage_system, physics_system);
            bullet_config.entity_file = "res/entities/green_blob.entity";
            bullet_config.sound_file = nullptr;

            //bullet_config.emitter_config.duration = -1;
            //bullet_config.emitter_config.emit_rate = 100.0f;
            //bullet_config.emitter_config.burst = false;
            //bullet_config.emitter_config.generator = PlasmaParticleGenerator;

            weapon_config.magazine_size = 40;
            weapon_config.rounds_per_second = 0.7f;
            weapon_config.bullet_force = 10.0f;

            break;
        }

        case game::WeaponType::GENERIC:
        {
            bullet_config.life_span = 10.0f;
            bullet_config.fuzzy_life_span = 0;
            bullet_config.collision_callback
                = std::bind(StandardCollision, _1, _2, m_entity_manager, damage_system, physics_system);
            bullet_config.entity_file = "res/entities/green_blob.entity";
            bullet_config.sound_file = nullptr;

            //bullet_config.emitter_config.duration = -1;
            //bullet_config.emitter_config.emit_rate = 100.0f;
            //bullet_config.emitter_config.burst = false;
            //bullet_config.emitter_config.generator = PlasmaParticleGenerator;

            weapon_config.magazine_size = 40;
            weapon_config.rounds_per_second = 2.0f;
            weapon_config.bullet_force = 10.0f;

            break;
        }

        case game::WeaponType::FLAK_CANON:
        {
            bullet_config.life_span = 10.0f;
            bullet_config.fuzzy_life_span = 0;
            bullet_config.collision_callback
                = std::bind(StandardCollision, _1, _2, m_entity_manager, damage_system, physics_system);
            bullet_config.entity_file = "res/entities/green_blob.entity";
            bullet_config.sound_file = nullptr;

            //bullet_config.emitter_config.duration = -1;
            //bullet_config.emitter_config.emit_rate = 100.0f;
            //bullet_config.emitter_config.burst = false;
            //bullet_config.emitter_config.generator = FlakParticleGenerator;

            weapon_config.projectiles_per_fire = 6;
            weapon_config.magazine_size = 6;
            weapon_config.rounds_per_second = 1.0f;
            weapon_config.bullet_force = 25.0f;
            weapon_config.bullet_spread = 4.0f;
            weapon_config.fire_sound = "res/sound/shotgun_fire.wav";
            weapon_config.reload_sound = "res/sound/shotgun_reload2.wav";
            //weapon_config.out_of_ammo_sound = "res/sound/ooa_sound.wav";

            break;
        }
    }

    return std::make_unique<game::Weapon>(weapon_config, m_entity_manager, m_system_context);
}
