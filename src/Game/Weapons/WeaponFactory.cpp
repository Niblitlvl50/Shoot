
#include "WeaponFactory.h"
#include "Weapons/BulletWeapon.h"
#include "Weapons/ThrowableWeapon.h"
#include "Weapons/CollisionCallbacks.h"
#include "DamageSystem.h"

#include "SystemContext.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "TransformSystem/TransformSystem.h"

#include <functional>

using namespace game;

WeaponFactory::WeaponFactory(mono::IEntityManager* entity_manager, mono::SystemContext* system_context)
    : m_entity_manager(entity_manager)
    , m_system_context(system_context)
{
    InitWeaponCallbacks(system_context);
}

WeaponFactory::~WeaponFactory()
{
    CleanupWeaponCallbacks();
}

IWeaponPtr WeaponFactory::CreateWeapon(WeaponType weapon_type, WeaponFaction faction, uint32_t owner_id)
{
    const bool is_bullet_weapon = IsBulletWeapon(weapon_type);
    if(is_bullet_weapon)
        return CreateBulletWeapon(weapon_type, faction, owner_id);
    else
        return CreateThrowableWeapon(weapon_type, faction, owner_id);
}

IWeaponPtr WeaponFactory::CreateBulletWeapon(WeaponType weapon_type, WeaponFaction faction, uint32_t owner_id)
{
    using namespace std::placeholders;

    DamageSystem* damage_system = m_system_context->GetSystem<DamageSystem>();
    mono::PhysicsSystem* physics_system = m_system_context->GetSystem<mono::PhysicsSystem>();
    mono::SpriteSystem* sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();
    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();

    WeaponConfiguration weapon_config;
    weapon_config.owner_id = owner_id;
    BulletConfiguration& bullet_config = weapon_config.bullet_config;

    const bool enemy_weapon = (faction == WeaponFaction::ENEMY);
    bullet_config.collision_category = enemy_weapon ? shared::CollisionCategory::ENEMY_BULLET : shared::CollisionCategory::PLAYER_BULLET;
    bullet_config.collision_mask = enemy_weapon ? shared::ENEMY_BULLET_MASK : shared::PLAYER_BULLET_MASK;

    switch(weapon_type)
    {
        case game::WeaponType::STANDARD:
        {
            bullet_config.life_span = 10.0f;
            bullet_config.fuzzy_life_span = 0;
            bullet_config.bullet_behaviour = BulletCollisionBehaviour::JUMPER;
            bullet_config.collision_callback
                = std::bind(StandardCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, physics_system, transform_system);
            bullet_config.entity_file = "res/entities/plasma_bullet.entity";
            bullet_config.sound_file = nullptr;

            weapon_config.reload_time = 1.0f;
            weapon_config.magazine_size = 99;
            //weapon_config.rounds_per_second = 7.0f;
            weapon_config.rounds_per_second = 10.0f;
            weapon_config.fire_rate_multiplier = 1.1f;
            weapon_config.max_fire_rate = 2.0f;
            weapon_config.bullet_force = 20.0f;
            weapon_config.bullet_spread_degrees = 2.0f;
            weapon_config.fire_sound = "res/sound/plasma_fire.wav";
            weapon_config.reload_sound = "res/sound/shotgun_reload2.wav";

            break;
        }

        case game::WeaponType::ROCKET_LAUNCHER:
        {
            bullet_config.life_span = 2.0f;
            bullet_config.fuzzy_life_span = 0.3f;
            bullet_config.collision_callback
                = std::bind(RocketCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, physics_system, sprite_system, transform_system);
            bullet_config.entity_file = "res/entities/rocket_bullet.entity";
            bullet_config.sound_file = nullptr;

            weapon_config.reload_time = 1.0f;
            weapon_config.magazine_size = 5;
            weapon_config.rounds_per_second = 1.5f;
            weapon_config.bullet_force = 10.0f;
            weapon_config.fire_sound = "res/sound/rocket_fire2.wav";
            weapon_config.reload_sound = "res/sound/shotgun_reload2.wav";

            break;
        }

        case game::WeaponType::CACOPLASMA:
        {
            bullet_config.life_span = 2.0f;
            bullet_config.fuzzy_life_span = 0.3f;
            bullet_config.collision_callback
                = std::bind(CacoPlasmaCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, physics_system, sprite_system, transform_system);
            bullet_config.entity_file = "res/entities/caco_bullet.entity";
            bullet_config.sound_file = nullptr;

            weapon_config.reload_time = 1.0f;
            weapon_config.magazine_size = 30;
            //weapon_config.rounds_per_second = 0.7f;
            weapon_config.rounds_per_second = 50.0f;
            weapon_config.bullet_force = 5.0f;
            weapon_config.bullet_spread_degrees = 10.0f;

            break;
        }

        case game::WeaponType::GENERIC:
        {
            bullet_config.life_span = 10.0f;
            bullet_config.fuzzy_life_span = 0;
            bullet_config.collision_callback
                = std::bind(StandardCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, physics_system, transform_system);
            bullet_config.entity_file = "res/entities/green_blob.entity";
            bullet_config.sound_file = nullptr;

            weapon_config.reload_time = 1.0f;
            weapon_config.magazine_size = 40;
            weapon_config.rounds_per_second = 1.0f;
            weapon_config.bullet_force = 10.0f;

            break;
        }

        case game::WeaponType::FLAK_CANON:
        {
            bullet_config.life_span = 10.0f;
            bullet_config.fuzzy_life_span = 0;
            bullet_config.collision_callback
                = std::bind(StandardCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, physics_system, transform_system);
            bullet_config.entity_file = "res/entities/flak_bullet.entity";
            bullet_config.sound_file = nullptr;

            weapon_config.projectiles_per_fire = 6;
            weapon_config.magazine_size = 30;
            weapon_config.reload_time = 1.0f;
            //weapon_config.rounds_per_second = 1.0f;
            weapon_config.rounds_per_second = 2.0f;
            weapon_config.bullet_force = 25.0f;
            weapon_config.bullet_spread_degrees = 4.0f;
            weapon_config.fire_sound = "res/sound/shotgun_fire.wav";
            weapon_config.reload_sound = "res/sound/shotgun_reload2.wav";
            //weapon_config.out_of_ammo_sound = "res/sound/ooa_sound.wav";

            break;
        }

        default:
            break;
    }

    return std::make_unique<game::Weapon>(weapon_config, m_entity_manager, m_system_context);
}

IWeaponPtr WeaponFactory::CreateThrowableWeapon(WeaponType weapon_type, WeaponFaction faction, uint32_t owner_id)
{
    ThrowableWeaponConfig weapon_config;
    weapon_config.owner_id = owner_id;

    const bool enemy_weapon = (faction == WeaponFaction::ENEMY);
    weapon_config.collision_category = enemy_weapon ? shared::CollisionCategory::ENEMY_BULLET : shared::CollisionCategory::PLAYER_BULLET;
    weapon_config.collision_mask = enemy_weapon ? shared::ENEMY_BULLET_MASK : shared::PLAYER_BULLET_MASK;

    switch(weapon_type)
    {
    case WeaponType::TURRET:
    {
        weapon_config.magazine_size = 100;
        weapon_config.projectiles_per_fire = 1;
        weapon_config.cooldown_seconds = 2.0f;
        weapon_config.max_distance = 5.0f;
        weapon_config.target_accuracy = 1.0f;

        weapon_config.thrown_entity = "res/entities/throwable_thing.entity";
        weapon_config.spawned_entity = "res/entities/torch_small.entity";
        weapon_config.collision_callback = nullptr;

        break;
    }

    default:
        break;
    }

    return std::make_unique<game::ThrowableWeapon>(weapon_config, m_entity_manager, m_system_context);
}
