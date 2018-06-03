
#include "WeaponFactory.h"
#include "Weapons/Weapon.h"
#include "Math/MathFwd.h"
#include "Math/MathFunctions.h"

#include "Explosion.h"
#include "Physics/IBody.h"
#include "Entity/IPhysicsEntity.h"

#include "EventHandler/EventHandler.h"
#include "Events/DamageEvent.h"
#include "Events/SpawnEntityEvent.h"
#include "Events/RemoveEntityEvent.h"
#include "Events/ShockwaveEvent.h"

#include "RenderLayers.h"

#include <functional>

namespace
{
    void StandardCollision(
        const mono::IPhysicsEntity* bullet, const mono::IBodyPtr& other, mono::EventHandler& event_handler)
    {
        event_handler.DispatchEvent(game::DamageEvent(other, 10, bullet->Rotation()));
        event_handler.DispatchEvent(game::RemoveEntityEvent(bullet->Id()));
    }

    void RocketCollision(const mono::IPhysicsEntity* bullet, const mono::IBodyPtr& other, mono::EventHandler& event_handler)
    {
        game::ExplosionConfiguration explosion_config;
        explosion_config.position = bullet->Position();
        explosion_config.scale = 2.0f;
        explosion_config.rotation = 0.0f;
        explosion_config.sprite_file = "res/sprites/explosion.sprite";

        const game::SpawnEntityEvent spawn_event(
            std::make_shared<game::Explosion>(explosion_config, event_handler), game::FOREGROUND, nullptr);
        event_handler.DispatchEvent(spawn_event);
        event_handler.DispatchEvent(game::DamageEvent(other, 50, bullet->Rotation()));
        event_handler.DispatchEvent(game::ShockwaveEvent(explosion_config.position, 100));
        event_handler.DispatchEvent(game::RemoveEntityEvent(bullet->Id()));
    }

    void CacoPlasmaCollision(
        const mono::IPhysicsEntity* bullet, const mono::IBodyPtr& other, mono::EventHandler& event_handler)
    {
        game::ExplosionConfiguration explosion_config;
        explosion_config.position = bullet->Position();
        explosion_config.scale = 2.0f;
        explosion_config.rotation = 0.0f;
        explosion_config.sprite_file = "res/sprites/caco_explosion.sprite";

        const game::SpawnEntityEvent spawn_event(
            std::make_shared<game::Explosion>(explosion_config, event_handler), game::FOREGROUND, nullptr);
        event_handler.DispatchEvent(spawn_event);
        event_handler.DispatchEvent(game::DamageEvent(other, 20, bullet->Rotation()));
        event_handler.DispatchEvent(game::RemoveEntityEvent(bullet->Id()));
    }

    void FlakCannonCollision(
        const mono::IPhysicsEntity* bullet, const mono::IBodyPtr& other, mono::EventHandler& event_handler)
    {
        const float direction = bullet->Rotation();

        const math::Vector& impulse = math::VectorFromAngle(direction) * 100.0f;
        other->ApplyImpulse(impulse, other->GetPosition());

        event_handler.DispatchEvent(game::DamageEvent(other, 20, direction));
        event_handler.DispatchEvent(game::RemoveEntityEvent(bullet->Id()));
    }
}

using namespace game;

WeaponFactory::WeaponFactory(mono::EventHandler& event_handler)
    : m_event_handler(event_handler)
{ }

std::unique_ptr<game::IWeaponSystem> WeaponFactory::CreateWeapon(WeaponType weapon, WeaponFaction faction)
{
    return CreateWeapon(weapon, faction, nullptr);
}

std::unique_ptr<IWeaponSystem> WeaponFactory::CreateWeapon(WeaponType weapon, WeaponFaction faction, mono::ParticlePool* pool)
{
    using namespace std::placeholders;

    WeaponConfiguration weapon_config;
    BulletConfiguration& bullet_config = weapon_config.bullet_config;

    const bool enemy_weapon = (faction == WeaponFaction::ENEMY);
    bullet_config.collision_category = enemy_weapon ? CollisionCategory::ENEMY_BULLET : CollisionCategory::PLAYER_BULLET;
    bullet_config.collision_mask = enemy_weapon ? ENEMY_BULLET_MASK : PLAYER_BULLET_MASK;
    
    bullet_config.pool = pool;

    switch(weapon)
    {
        case game::WeaponType::STANDARD:
        {
            bullet_config.life_span = 10.0f;
            bullet_config.fuzzy_life_span = 0;
            bullet_config.collision_radius = 0.4f;
            bullet_config.scale = 0.4;
            bullet_config.collision_callback = std::bind(StandardCollision, _1, _2, std::ref(m_event_handler));
            bullet_config.sprite_file = "res/sprites/generic.sprite";
            bullet_config.shade = mono::Color::RGBA(0.6f, 0.6f, 0.8f, 1.0f);
            bullet_config.sound_file = nullptr;

            weapon_config.magazine_size = 99;
            weapon_config.rounds_per_second = 7.0f;
            weapon_config.fire_rate_multiplier = 1.1f;
            weapon_config.max_fire_rate = 2.0f;
            weapon_config.bullet_force = 30.0f;
            weapon_config.bullet_spread = 2.0f;
            weapon_config.fire_sound = "res/sound/plasma_fire.wav";
            weapon_config.reload_sound = "res/sound/shotgun_reload2.wav";

            break;
        }

        case game::WeaponType::ROCKET_LAUNCHER:
        {
            bullet_config.life_span = 2.0f;
            bullet_config.fuzzy_life_span = 0.3f;
            bullet_config.collision_radius = 0.5f;
            bullet_config.collision_callback = std::bind(RocketCollision, _1, _2, std::ref(m_event_handler));
            bullet_config.scale = 0.5;
            bullet_config.sprite_file = "res/sprites/rocket.sprite";
            bullet_config.sound_file = nullptr;

            weapon_config.magazine_size = 5;
            weapon_config.rounds_per_second = 1.5f;
            weapon_config.bullet_force = 20.0f;
            weapon_config.fire_sound = "res/sound/rocket_fire2.wav";
            weapon_config.reload_sound = "res/sound/shotgun_reload2.wav";

            break;
        }

        case game::WeaponType::CACOPLASMA:
        {
            bullet_config.life_span = 1.0f;
            bullet_config.fuzzy_life_span = 0.3f;
            bullet_config.collision_radius = 0.5f;
            bullet_config.collision_callback = std::bind(CacoPlasmaCollision, _1, _2, std::ref(m_event_handler));
            bullet_config.sprite_file = "res/sprites/caco_bullet.sprite";
            bullet_config.scale = 0.5;
            bullet_config.sound_file = nullptr;

            weapon_config.magazine_size = 40;
            weapon_config.rounds_per_second = 5.0f;
            weapon_config.bullet_force = 20.0f;

            break;
        }

        case game::WeaponType::GENERIC:
        {
            bullet_config.life_span = 10.0f;
            bullet_config.fuzzy_life_span = 0;
            bullet_config.scale = 0.3;
            bullet_config.collision_radius = 0.15f;
            bullet_config.collision_callback = std::bind(StandardCollision, _1, _2, std::ref(m_event_handler));
            bullet_config.shade = mono::Color::RGBA(1.0f, 0.0f, 0.0f, 1.0f);
            bullet_config.sprite_file = "res/sprites/generic.sprite";
            bullet_config.sound_file = nullptr;

            weapon_config.magazine_size = 40;
            weapon_config.rounds_per_second = 4.0f;
            weapon_config.bullet_force = 15.0f;

            break;
        }

        case game::WeaponType::FLAK_CANON:
        {
            bullet_config.life_span = 10.0f;
            bullet_config.fuzzy_life_span = 0;
            bullet_config.scale = 0.2;
            bullet_config.collision_radius = 0.15f;
            bullet_config.collision_callback = std::bind(FlakCannonCollision, _1, _2, std::ref(m_event_handler));
            bullet_config.shade = mono::Color::RGBA(0.3f, 0.3f, 0.3f, 1.0f);
            bullet_config.sprite_file = "res/sprites/generic.sprite";
            bullet_config.sound_file = nullptr;

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

    return std::make_unique<game::Weapon>(weapon_config, m_event_handler);
}
