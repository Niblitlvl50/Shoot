
#include "WeaponSystem.h"
#include "System/Hash.h"
#include "System/System.h"
#include "Util/Algorithm.h"

#include "Weapons/BulletWeapon/BulletWeapon.h"
#include "Weapons/ThrowableWeapon/ThrowableWeapon.h"
#include "Weapons/CollisionCallbacks.h"
#include "DamageSystem/DamageSystem.h"

#include "SystemContext.h"
#include "EntitySystem/IEntityManager.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "TransformSystem/TransformSystem.h"

#include <algorithm>
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
            return { 0, 0, 0 };
        }
    };
}

using namespace game;

WeaponSystem::WeaponSystem(
    mono::TransformSystem* transform_system,
    mono::SpriteSystem* sprite_system,
    mono::PhysicsSystem* physics_system,
    mono::IEntityManager* entity_manager,
    game::DamageSystem* damage_system,
    game::CameraSystem* camera_system,
    game::EntityLogicSystem* logic_system,
    game::TargetSystem* target_system,
    mono::SystemContext* system_context)
    : m_transform_system(transform_system)
    , m_entity_manager(entity_manager)
    , m_system_context(system_context)
    , m_weapon_entity_factory(entity_manager, transform_system, physics_system, logic_system, target_system)
    , m_modifier_id(0)
{
    m_weapon_configuration = LoadWeaponConfig("res/configs/weapon_config.json");

    using namespace std::placeholders;
    m_standard_collision =
        std::bind(StandardCollision, _1, _2, _3, _4, _5, _6, m_entity_manager, damage_system, sprite_system, transform_system);

    m_bullet_callbacks = {
        { hash::Hash("rocket"),             std::bind(RocketCollision, _1, _2, _3, _4, _5, _6, m_entity_manager, damage_system, camera_system, sprite_system, transform_system) },
        { hash::Hash("webber_bullet"),      std::bind(WebberCollision, _1, _2, _3, _4, _5, _6, m_entity_manager, damage_system, physics_system, sprite_system, transform_system, &m_weapon_entity_factory) },
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
    for(auto& pair : m_weapon_modifiers)
    {
        std::vector<uint32_t> indices_to_remove;

        for(uint32_t index = 0; index < pair.second.durations.size(); ++index)
        {
            WeaponModifierDuration& duration = pair.second.durations[index];
            if(duration.duration_counter > 0.0f)
            {
                duration.duration_counter -= update_context.delta_s;

                if(duration.duration_counter <= 0.0f)
                    indices_to_remove.push_back(index);
            }
        }

        std::reverse(indices_to_remove.begin(), indices_to_remove.end());

        for(uint32_t index_to_remove : indices_to_remove)
        {
            pair.second.durations.erase(pair.second.durations.begin() + index_to_remove);
            pair.second.modifiers.erase(pair.second.modifiers.begin() + index_to_remove);
            pair.second.ids.erase(pair.second.ids.begin() + index_to_remove);
        }
    }
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
    const auto impact_callback_it = m_bullet_callbacks.find(setup.bullet_hash);

    CollisionConfiguration collision_config;
    collision_config.collision_category = enemy_weapon ? CollisionCategory::ENEMY_BULLET : CollisionCategory::PLAYER_BULLET;
    collision_config.collision_mask = enemy_weapon ? ENEMY_BULLET_MASK : PLAYER_BULLET_MASK;
    collision_config.collision_callback = (impact_callback_it != m_bullet_callbacks.end()) ? impact_callback_it->second : m_standard_collision;

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

const WeaponBulletCombination& WeaponSystem::GetWeaponBulletConfigForHash(uint32_t weapon_hash) const
{
    const auto it = m_weapon_configuration.weapon_combinations.find(weapon_hash);
    if(it != m_weapon_configuration.weapon_combinations.end())
        return it->second;

    static const WeaponBulletCombination dummy_combination;
    return dummy_combination;
}

uint32_t WeaponSystem::SpawnWeaponPickupAt(const WeaponSetup& setup, const math::Vector& world_position)
{
    const mono::Entity spawned_entity = m_entity_manager->SpawnEntity(m_weapon_configuration.weapon_pickup_entity.c_str());
    m_transform_system->SetTransform(spawned_entity.id, math::CreateMatrixWithPosition(world_position), mono::TransformState::CLIENT);

    SetWeaponLoadout(spawned_entity.id, setup, game::WeaponSetup(), game::WeaponSetup());

    return spawned_entity.id;
}

int WeaponSystem::AddModifierForId(uint32_t id, IWeaponModifier* weapon_modifier)
{
    return AddModifierForIdWithDuration(id, -1.0f, weapon_modifier);
}

int WeaponSystem::AddModifierForIdWithDuration(uint32_t id, float duration_s, IWeaponModifier* weapon_modifier)
{
    m_modifier_id++;

    WeaponModifierDuration duration;
    duration.duration = duration_s;
    duration.duration_counter = duration_s;

    WeaponModifierContext& context = m_weapon_modifiers[id];
    context.durations.push_back(duration);
    context.modifiers.push_back(std::unique_ptr<IWeaponModifier>(weapon_modifier));
    context.ids.push_back(m_modifier_id);

    return m_modifier_id;
}

float WeaponSystem::GetDurationFractionForModifierOnEntity(uint32_t entity_id, uint32_t modifier_id) const
{
    const auto it_context = m_weapon_modifiers.find(entity_id);
    if(it_context == m_weapon_modifiers.end())
        return 0.0f;
    
    const uint32_t index = mono::find_index_of(it_context->second.ids, modifier_id);
    if(index == -1)
        return 0.0f;

    const WeaponModifierDuration& duration = it_context->second.durations[index];
    return duration.duration / duration.duration_counter;
}

const WeaponModifierList& WeaponSystem::GetWeaponModifiersForId(uint32_t id) const
{
    const auto it = m_weapon_modifiers.find(id);
    if(it != m_weapon_modifiers.end())
        return it->second.modifiers;

    static const WeaponModifierList empty_list;
    return empty_list;
}
