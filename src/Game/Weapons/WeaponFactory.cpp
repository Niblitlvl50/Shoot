
#include "WeaponFactory.h"
#include "Weapons/BulletWeapon.h"
#include "Weapons/ThrowableWeapon.h"
#include "DamageSystem.h"

#include "SystemContext.h"
#include "EntitySystem/IEntityManager.h"
#include "Math/MathFwd.h"
#include "Math/MathFunctions.h"
#include "Physics/IBody.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "TransformSystem/TransformSystem.h"

#include <functional>
#include <limits>

namespace
{
    void SpawnEntityWithAnimation(
        const char* entity_file,
        int animation_id,
        uint32_t position_at_transform_id,
        mono::IEntityManager* entity_manager,
        mono::TransformSystem* transform_system,
        mono::SpriteSystem* sprite_system)
    {
        const mono::Entity spawned_entity = entity_manager->CreateEntity(entity_file);
        math::Matrix& entity_transform = transform_system->GetTransform(spawned_entity.id);
        entity_transform = transform_system->GetWorld(position_at_transform_id);

        mono::Sprite* spawned_entity_sprite = sprite_system->GetSprite(spawned_entity.id);

        const auto remove_entity_callback = [spawned_entity, entity_manager]() {
            entity_manager->ReleaseEntity(spawned_entity.id);
        };
        spawned_entity_sprite->SetAnimation(animation_id, remove_entity_callback);
    }

    void StandardCollision(
        uint32_t entity_id,
        uint32_t owner_entity_id,
        game::BulletCollisionFlag flags,
        const mono::IBody* other,
        mono::IEntityManager* entity_manager,
        game::DamageSystem* damage_system,
        mono::PhysicsSystem* physics_system)
    {
        const uint32_t other_entity_id = physics_system->GetIdFromBody(other);
        if(other_entity_id != std::numeric_limits<uint32_t>::max() && flags & game::BulletCollisionFlag::APPLY_DAMAGE)
            damage_system->ApplyDamage(other_entity_id, 20, owner_entity_id);

        if(flags & game::BulletCollisionFlag::DESTROY_THIS)
            entity_manager->ReleaseEntity(entity_id);
    }

    void RocketCollision(
        uint32_t entity_id,
        uint32_t owner_entity_id,
        game::BulletCollisionFlag flags, 
        const mono::IBody* other,
        mono::IEntityManager* entity_manager,
        game::DamageSystem* damage_system,
        mono::PhysicsSystem* physics_system,
        mono::SpriteSystem* sprite_system,
        mono::TransformSystem* transform_system)
    {
        StandardCollision(entity_id, owner_entity_id, flags, other, entity_manager, damage_system, physics_system);
        //SpawnEntityWithAnimation("res/entities/explosion.entity", 0, entity_id, entity_manager, transform_system, sprite_system);
        //event_handler.DispatchEvent(game::ShockwaveEvent(explosion_config.position, 150));
    }

    void CacoPlasmaCollision(
        uint32_t entity_id,
        uint32_t owner_entity_id,
        game::BulletCollisionFlag flags,
        const mono::IBody* other,
        mono::IEntityManager* entity_manager,
        game::DamageSystem* damage_system,
        mono::PhysicsSystem* physics_system,
        mono::SpriteSystem* sprite_system,
        mono::TransformSystem* transform_system)
    {
        StandardCollision(entity_id, owner_entity_id, flags, other, entity_manager, damage_system, physics_system);
        SpawnEntityWithAnimation("res/entities/caco_explosion.entity", 0, entity_id, entity_manager, transform_system, sprite_system);
    }
}

using namespace game;

WeaponFactory::WeaponFactory(mono::IEntityManager* entity_manager, mono::SystemContext* system_context)
    : m_entity_manager(entity_manager)
    , m_system_context(system_context)
{ }

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
                = std::bind(StandardCollision, _1, _2, _3, _4, m_entity_manager, damage_system, physics_system);
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
                = std::bind(RocketCollision, _1, _2, _3, _4, m_entity_manager, damage_system, physics_system, sprite_system, transform_system);
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
                = std::bind(CacoPlasmaCollision, _1, _2, _3, _4, m_entity_manager, damage_system, physics_system, sprite_system, transform_system);
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
                = std::bind(StandardCollision, _1, _2, _3, _4, m_entity_manager, damage_system, physics_system);
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
                = std::bind(StandardCollision, _1, _2, _3, _4, m_entity_manager, damage_system, physics_system);
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
    const bool enemy_weapon = (faction == WeaponFaction::ENEMY);

    ThrowableWeaponConfig weapon_config;
    weapon_config.owner_id = owner_id;
    weapon_config.magazine_size = 10;
    weapon_config.projectiles_per_fire = 1;
    weapon_config.cooldown_seconds = 2.0f;
    weapon_config.max_distance = 5.0f;

    weapon_config.thrown_entity = "res/entities/throwable_thing.entity";
    weapon_config.spawned_entity = "res/entities/torch_small.entity";
    weapon_config.life_span = 1.0f;
    weapon_config.fuzzy_life_span = 0.0f;
    weapon_config.collision_category = enemy_weapon ? shared::CollisionCategory::ENEMY_BULLET : shared::CollisionCategory::PLAYER_BULLET;
    weapon_config.collision_mask = enemy_weapon ? shared::ENEMY_BULLET_MASK : shared::PLAYER_BULLET_MASK;
    weapon_config.collision_callback = nullptr;

    return std::make_unique<game::ThrowableWeapon>(weapon_config, m_entity_manager, m_system_context);
}
