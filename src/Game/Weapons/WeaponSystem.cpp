
#include "WeaponSystem.h"
#include "System/Hash.h"
#include "System/System.h"

#include "Weapons/BulletWeapon/BulletWeapon.h"
#include "Weapons/ThrowableWeapon/ThrowableWeapon.h"
#include "Weapons/CollisionCallbacks.h"
#include "DamageSystem/DamageSystem.h"

#include "SystemContext.h"
#include "EntitySystem/IEntityManager.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "TransformSystem/TransformSystem.h"

#include <functional>

namespace
{
    class NullWeapon : public game::IWeapon
    {
    public:

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
        game::WeaponState GetWeaponState() const override
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
        game::WeaponSetup GetWeaponSetup() const override
        {
            return { 0, 0 };
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
    : m_transform_system(transform_system)
    , m_entity_manager(entity_manager)
    , m_system_context(system_context)
{
    m_weapon_configuration = LoadWeaponConfig("res/configs/weapon_config.json");

    using namespace std::placeholders;
    m_standard_collision =
        std::bind(StandardCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, physics_system, sprite_system, transform_system);

    m_bullet_callbacks = {
        { hash::Hash("plasma_bullet"),      std::bind(PlasmaCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, physics_system, sprite_system, transform_system) },
        { hash::Hash("rocket"),             std::bind(RocketCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, camera_system, physics_system, sprite_system, transform_system) },
        { hash::Hash("caco_bullet"),        std::bind(CacoPlasmaCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, physics_system, sprite_system, transform_system) },
        { hash::Hash("caco_bullet_homing"), std::bind(CacoPlasmaCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, physics_system, sprite_system, transform_system) },
        { hash::Hash("webber_bullet"),      std::bind(WebberCollision, _1, _2, _3, _4, _5, m_entity_manager, damage_system, physics_system, sprite_system, transform_system) },
    };
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
    SetWeaponLoadout(
        entity_id,
        game::FindWeaponSetupFromString(m_weapon_configuration, primary.c_str()),
        game::FindWeaponSetupFromString(m_weapon_configuration, secondary.c_str()),
        game::FindWeaponSetupFromString(m_weapon_configuration, tertiary.c_str()));
}

void WeaponSystem::SetWeaponLoadout(
    uint32_t entity_id, const WeaponSetup& primary, const WeaponSetup& secondary, const WeaponSetup& tertiary)
{
    m_weapon_loadout[entity_id] = { primary, secondary, tertiary };
}

IWeaponPtr WeaponSystem::CreatePrimaryWeapon(uint32_t entity_id, WeaponFaction faction)
{
    const auto it = m_weapon_loadout.find(entity_id);
    if(it == m_weapon_loadout.end())
        return std::make_unique<NullWeapon>();

    return CreateWeapon(it->second.primary, faction, entity_id);
}

IWeaponPtr WeaponSystem::CreateSecondaryWeapon(uint32_t entity_id, WeaponFaction faction)
{
    const auto it = m_weapon_loadout.find(entity_id);
    if(it == m_weapon_loadout.end())
        return std::make_unique<NullWeapon>();

    return CreateWeapon(it->second.secondary, faction, entity_id);
}

IWeaponPtr WeaponSystem::CreateTertiaryWeapon(uint32_t entity_id, WeaponFaction faction)
{
    const auto it = m_weapon_loadout.find(entity_id);
    if(it == m_weapon_loadout.end())
        return std::make_unique<NullWeapon>();

    return CreateWeapon(it->second.tertiary, faction, entity_id);
}

IWeaponPtr WeaponSystem::CreateWeapon(WeaponSetup setup, WeaponFaction faction, uint32_t owner_id)
{
    const auto weapon_config_it = m_weapon_configuration.weapon_configs.find(setup.weapon_hash);
    if(weapon_config_it == m_weapon_configuration.weapon_configs.end())
    {
        System::Log("weaponsystem|Unable to find weapon configuration.");
        return std::make_unique<NullWeapon>();
    }

    const auto bullet_config_it = m_weapon_configuration.bullet_configs.find(setup.bullet_hash);
    if(bullet_config_it == m_weapon_configuration.bullet_configs.end())
    {
        System::Log("weaponsystem|Unable to find bullet configuration.");
        return std::make_unique<NullWeapon>();
    }

    const WeaponConfiguration& weapon_config = m_weapon_configuration.weapon_configs[setup.weapon_hash];
    const BulletConfiguration& bullet_config = m_weapon_configuration.bullet_configs[setup.bullet_hash];

    const bool enemy_weapon = (faction == WeaponFaction::ENEMY);

    CollisionConfiguration collision_config;
    collision_config.collision_category = enemy_weapon ? CollisionCategory::ENEMY_BULLET : CollisionCategory::PLAYER_BULLET;
    collision_config.collision_mask = enemy_weapon ? ENEMY_BULLET_MASK : PLAYER_BULLET_MASK;
    collision_config.collision_callback = nullptr;

    const auto it = m_bullet_callbacks.find(setup.bullet_hash);
    collision_config.collision_callback = (it != m_bullet_callbacks.end()) ? it->second : m_standard_collision;

    const bool is_bullet_weapon = true; //IsBulletWeapon(setup);
    if(is_bullet_weapon)
    {
        return std::make_unique<game::Weapon>(
            owner_id, setup, weapon_config, bullet_config, collision_config, m_entity_manager, m_system_context);
    }
    else
    {
        return CreateThrowableWeapon(setup, faction, owner_id);
    }
}

IWeaponPtr WeaponSystem::CreateThrowableWeapon(WeaponSetup setup, WeaponFaction faction, uint32_t owner_id)
{
    ThrowableWeaponConfig weapon_config;

    const bool enemy_weapon = (faction == WeaponFaction::ENEMY);
    weapon_config.collision_category = enemy_weapon ? CollisionCategory::ENEMY_BULLET : CollisionCategory::PLAYER_BULLET;
    weapon_config.collision_mask = enemy_weapon ? ENEMY_BULLET_MASK : PLAYER_BULLET_MASK;

    return std::make_unique<game::ThrowableWeapon>(setup, weapon_config, m_entity_manager, m_system_context);
}

std::vector<WeaponBulletCombination> WeaponSystem::GetAllWeaponCombinations() const
{
    std::vector<WeaponBulletCombination> weapon_combinations;
    for(const auto& pair : m_weapon_configuration.weapon_combinations)
        weapon_combinations.push_back(pair.second);

    return weapon_combinations;
}

uint32_t WeaponSystem::SpawnWeaponPickupAt(const WeaponSetup& setup, const math::Vector& world_position)
{
    const mono::Entity spawned_entity = m_entity_manager->SpawnEntity(m_weapon_configuration.weapon_pickup_entity.c_str());
    m_transform_system->SetTransform(spawned_entity.id, math::CreateMatrixWithPosition(world_position), mono::TransformState::CLIENT);

    SetWeaponLoadout(spawned_entity.id, setup, game::WeaponSetup(), game::WeaponSetup());

    return spawned_entity.id;
}
