
#include "WeaponSystem.h"
#include "System/Hash.h"

#include "Weapons/BulletWeapon.h"
#include "Weapons/ThrowableWeapon.h"
#include "Weapons/CollisionCallbacks.h"
#include "DamageSystem.h"

#include "SystemContext.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "System/Hash.h"

#include <functional>

namespace
{
    class NullWeapon : public game::IWeapon
    {
    public:

        game::WeaponState Fire(const math::Vector& position, float direction, uint32_t timestamp) override
        {
            return game::WeaponState::IDLE;
        }
        game::WeaponState Fire(const math::Vector& position, const math::Vector& target, uint32_t timestamp) override
        {
            return game::WeaponState::IDLE;
        }
        void StopFire(uint32_t timestamp) override { }
        void Reload(uint32_t timestamp) override { }
        game::WeaponState UpdateWeaponState(uint32_t timestamp) override
        {
            return game::WeaponState::IDLE;
        }
        void AddAmmunition(int amount) override { }
        
        int AmmunitionLeft() const override
        {
            return 0;
        }
        int MagazineSize() const override
        {
            return 0;
        }
        int ReloadPercentage() const override
        {
            return 100;
        }
    };
}

using namespace game;

WeaponSystem::WeaponSystem(
    mono::TransformSystem* transform_system,
    mono::SpriteSystem* sprite_system,
    mono::PhysicsSystem* physics_system,
    game::DamageSystem* damage_system,
    game::CameraSystem* camera_system,
    mono::IEntityManager* entity_manager,
    mono::SystemContext* system_context)
    : m_entity_manager(entity_manager)
    , m_system_context(system_context)
{
    m_weapon_configuration = LoadWeaponConfig("res/weapon_config.json");

    using namespace std::placeholders;
    m_bullet_callbacks = {
        { GENERIC.bullet_hash,          std::bind(StandardCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, physics_system, sprite_system, transform_system) },
        { PLASMA_GUN.bullet_hash,       std::bind(PlasmaCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, physics_system, sprite_system, transform_system) },
        { ROCKET_LAUNCHER.bullet_hash,  std::bind(RocketCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, camera_system, physics_system, sprite_system, transform_system) },
        { CACO_PLASMA.bullet_hash,      std::bind(CacoPlasmaCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, physics_system, sprite_system, transform_system) },
        { CACO_PLASMA_HOMING.bullet_hash,std::bind(CacoPlasmaCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, physics_system, sprite_system, transform_system) },
        { FLAK_CANON.bullet_hash,       std::bind(StandardCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, physics_system, sprite_system, transform_system) },
        { LASER_BLASTER.bullet_hash,    std::bind(StandardCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, physics_system, sprite_system, transform_system) },
    };
}

uint32_t WeaponSystem::Id() const
{
    return hash::Hash(Name());
}

const char* WeaponSystem::Name() const
{
    return "weaponsystem";
}

void WeaponSystem::Begin()
{
    InitWeaponCallbacks(m_system_context);
}

void WeaponSystem::Reset()
{
    CleanupWeaponCallbacks();
}

void WeaponSystem::Update(const mono::UpdateContext& update_context)
{

}

void WeaponSystem::SetWeaponLoadout(
    uint32_t entity_id, const std::string& primary, const std::string& secondary, const std::string& tertiary)
{
    m_weapon_loadout[entity_id] = { primary, secondary, tertiary };
}

IWeaponPtr WeaponSystem::CreatePrimaryWeapon(uint32_t entity_id, WeaponFaction faction)
{
    const auto it = m_weapon_loadout.find(entity_id);
    if(it == m_weapon_loadout.end())
        return std::make_unique<NullWeapon>();

    return CreateWeapon(it->second.primary_name.c_str(), faction, entity_id);
}

IWeaponPtr WeaponSystem::CreateWeapon(WeaponSetup setup, WeaponFaction faction, uint32_t owner_id)
{
    if(setup == game::NO_WEAPON)
        return std::make_unique<NullWeapon>();

    const WeaponConfiguration& weapon_config = m_weapon_configuration.weapon_configs[setup.weapon_hash];
    const BulletConfiguration& bullet_config = m_weapon_configuration.bullet_configs[setup.bullet_hash];

    CollisionConfiguration collision_config;

    const bool enemy_weapon = (faction == WeaponFaction::ENEMY);
    collision_config.collision_category = enemy_weapon ? CollisionCategory::ENEMY_BULLET : CollisionCategory::PLAYER_BULLET;
    collision_config.collision_mask = enemy_weapon ? ENEMY_BULLET_MASK : PLAYER_BULLET_MASK;
    collision_config.collision_callback = nullptr;

    const auto it = m_bullet_callbacks.find(setup.bullet_hash);
    if(it != m_bullet_callbacks.end())
        collision_config.collision_callback = it->second;

    const bool is_bullet_weapon = true; //IsBulletWeapon(setup);
    if(is_bullet_weapon)
    {
        return std::make_unique<game::Weapon>(owner_id, weapon_config, bullet_config, collision_config, m_entity_manager, m_system_context);
    }
    else
    {
        return CreateThrowableWeapon(setup, faction, owner_id);
    }
}

IWeaponPtr WeaponSystem::CreateWeapon(const char* weapon_name, WeaponFaction faction, uint32_t owner_id)
{
    const auto it = m_weapon_configuration.weapon_combinations.find(hash::Hash(weapon_name));
    if(it != m_weapon_configuration.weapon_combinations.end())
    {
        WeaponSetup weapon_setup;
        weapon_setup.weapon_hash = hash::Hash(it->second.weapon.c_str());
        weapon_setup.bullet_hash = hash::Hash(it->second.bullet.c_str());

        return CreateWeapon(weapon_setup, faction, owner_id);
    }

    return std::make_unique<NullWeapon>();
}

IWeaponPtr WeaponSystem::CreateThrowableWeapon(WeaponSetup setup, WeaponFaction faction, uint32_t owner_id)
{
    ThrowableWeaponConfig weapon_config;

    const bool enemy_weapon = (faction == WeaponFaction::ENEMY);
    weapon_config.collision_category = enemy_weapon ? CollisionCategory::ENEMY_BULLET : CollisionCategory::PLAYER_BULLET;
    weapon_config.collision_mask = enemy_weapon ? ENEMY_BULLET_MASK : PLAYER_BULLET_MASK;

/*
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
    */

    return std::make_unique<game::ThrowableWeapon>(weapon_config, m_entity_manager, m_system_context);
}
