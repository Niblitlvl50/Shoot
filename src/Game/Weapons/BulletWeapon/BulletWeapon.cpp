
#include "BulletWeapon.h"
#include "Weapons/WeaponEntityFactory.h"
#include "Weapons/IWeaponModifier.h"
#include "Weapons/Modifiers/DamageModifier.h"
#include "Weapons/WeaponSystem.h"

#include "BulletLogic.h"
#include "Entity/Component.h"
#include "Entity/TargetSystem.h"
#include "Effects/MuzzleFlash.h"
#include "Effects/BulletTrailEffect.h"

#include "SystemContext.h"
#include "EntitySystem/IEntityManager.h"
#include "Entity/EntityLogicSystem.h"
#include "Math/Vector.h"
#include "Math/MathFunctions.h"
#include "Physics/PhysicsSystem.h"
#include "Particle/ParticleSystem.h"
#include "System/Audio.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Random.h"

#include <cmath>
#include <algorithm>
#include <cassert>

using namespace game;

Weapon::Weapon(
    uint32_t owner_id,
    const WeaponSetup& weapon_setup,
    const WeaponConfiguration& weapon_config,
    const BulletConfiguration& bullet_config,
    const CollisionConfiguration& collision_config,
    mono::IEntityManager* entity_manager,
    mono::SystemContext* system_context)
    : m_owner_id(owner_id)
    , m_weapon_setup(weapon_setup)
    , m_weapon_config(weapon_config)
    , m_bullet_config(bullet_config)
    , m_collision_config(collision_config)
    , m_entity_manager(entity_manager)
    , m_last_fire_timestamp(0)
    , m_last_reload_timestamp(0)
    , m_current_fire_rate(1.0f)
    , m_ammunition(weapon_config.magazine_size)
    , m_reload_percentage(100)
    , m_state(WeaponState::IDLE)
{
    m_fire_sound = audio::CreateNullSound();
    m_ooa_sound = audio::CreateNullSound();
    m_reload_sound = audio::CreateNullSound();
    m_reload_finished_sound = audio::CreateNullSound();

    if(!m_weapon_config.fire_sound.empty())
        m_fire_sound = audio::CreateSound(m_weapon_config.fire_sound.c_str(), audio::SoundPlayback::ONCE);

    if(!m_weapon_config.out_of_ammo_sound.empty())
        m_ooa_sound = audio::CreateSound(m_weapon_config.out_of_ammo_sound.c_str(), audio::SoundPlayback::ONCE);
    
    if(!m_weapon_config.reload_sound.empty())
        m_reload_sound = audio::CreateSound(m_weapon_config.reload_sound.c_str(), audio::SoundPlayback::ONCE);

    if(!m_weapon_config.reload_finished_sound.empty())
        m_reload_finished_sound = audio::CreateSound(m_weapon_config.reload_sound.c_str(), audio::SoundPlayback::ONCE);

    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_particle_system = system_context->GetSystem<mono::ParticleSystem>();
    m_logic_system = system_context->GetSystem<EntityLogicSystem>();
    m_target_system = system_context->GetSystem<TargetSystem>();
    m_weapon_system = system_context->GetSystem<WeaponSystem>();

    m_muzzle_flash = std::make_unique<MuzzleFlash>(m_particle_system, m_entity_manager);
    m_bullet_trail = std::make_unique<BulletTrailEffect>(m_transform_system, m_particle_system, m_entity_manager);
}

Weapon::~Weapon()
{
    for(const auto& pair : m_bullet_id_to_callback)
        m_entity_manager->RemoveReleaseCallback(pair.first, pair.second);
}

WeaponState Weapon::Fire(const math::Vector& position, const math::Vector& target, uint32_t timestamp)
{
    if(m_state == WeaponState::RELOADING)
        return m_state;

    if(m_ammunition == 0)
    {
        m_current_fire_rate = 1.0f;

        if(!m_ooa_sound->IsPlaying())
            m_ooa_sound->Play();

        m_state = WeaponState::OUT_OF_AMMO;
        return m_state;
    }

    WeaponConfiguration local_weapon_config = m_weapon_config;

    const WeaponModifierList& modifier_list = m_weapon_system->GetWeaponModifiersForId(m_owner_id);
    for(const auto& modifier : modifier_list)
        local_weapon_config = modifier->ModifyWeapon(local_weapon_config);

    const float rps_hz = 1.0f / local_weapon_config.rounds_per_second;
    const uint32_t weapon_delta = rps_hz * 1000.0f;

    const uint32_t delta = timestamp - m_last_fire_timestamp;
    const uint32_t modified_delta = delta * m_current_fire_rate;

    if(modified_delta < weapon_delta)
    {
        m_state = WeaponState::IDLE;
        return m_state;
    }

    if(delta > weapon_delta)
        m_current_fire_rate = 1.0f;

    m_last_fire_timestamp = timestamp;

    const WeaponEntityFactory entity_factory(
        m_entity_manager, m_transform_system, m_physics_system, m_logic_system, m_target_system);

    const mono::ReleaseCallback release_callback = [this](uint32_t entity_id, mono::ReleasePhase phase) {
        m_bullet_trail->RemoveEmitterFromBullet(entity_id);
        m_bullet_id_to_callback.erase(entity_id);
    };

    const math::Vector fire_direction = math::Normalized(target - position);
    const math::Vector perpendicular_fire_direction = math::Perpendicular(fire_direction);

    BulletConfiguration local_bullet_config = m_bullet_config;
    for(const auto& modifier : modifier_list)
        local_bullet_config = modifier->ModifyBullet(local_bullet_config);

    for(int n_bullet = 0; n_bullet < local_weapon_config.projectiles_per_fire; ++n_bullet)
    {
        const float fire_direction_deviation =
            mono::Random(-local_weapon_config.bullet_spread_degrees, local_weapon_config.bullet_spread_degrees);
        const math::Vector modified_fire_direction = math::RotateAroundZero(fire_direction, math::ToRadians(fire_direction_deviation));

        const float velocity_multiplier = local_weapon_config.bullet_velocity_random ? mono::Random(0.8f, 1.2f) : 1.0f;
        const math::Vector& velocity =
            math::Normalized(modified_fire_direction) * local_weapon_config.bullet_velocity * velocity_multiplier;

        const float bullet_direction = math::AngleFromVector(modified_fire_direction);

        const math::Vector perp_offset =
            perpendicular_fire_direction * mono::Random(-local_weapon_config.bullet_offset, local_weapon_config.bullet_offset);
        const math::Vector fire_position = position + perp_offset;

        const float bullet_rotation =
            local_bullet_config.bullet_want_direction ? math::AngleFromVector(modified_fire_direction) : 0.0f;
        const math::Matrix& transform = math::CreateMatrixWithPositionRotation(fire_position, bullet_rotation);

        mono::Entity bullet_entity = entity_factory.CreateBulletEntity(
            m_owner_id, m_weapon_setup.weapon_identifier_hash, local_bullet_config, m_collision_config, target, velocity, bullet_direction, transform);

        m_bullet_trail->AttachEmitterToBullet(bullet_entity.id);

        const uint32_t callback_id = m_entity_manager->AddReleaseCallback(bullet_entity.id, mono::ReleasePhase::POST_RELEASE, release_callback);
        m_bullet_id_to_callback[bullet_entity.id] = callback_id;
    }

    m_fire_sound->Play();

    m_current_fire_rate *= local_weapon_config.fire_rate_multiplier;
    m_current_fire_rate = std::min(m_current_fire_rate, local_weapon_config.max_fire_rate);

    if(!local_weapon_config.infinite_ammo)
        m_ammunition--;

    m_state = WeaponState::FIRE;
    return m_state;
}

void Weapon::StopFire(uint32_t timestamp)
{

}

void Weapon::Reload(uint32_t timestamp)
{
    m_last_reload_timestamp = timestamp;
    m_ammunition = m_weapon_config.magazine_size;
    m_state = WeaponState::RELOADING;
    m_reload_sound->Play();
}

WeaponState Weapon::UpdateWeaponState(uint32_t timestamp)
{
    switch(m_state)
    {
    case WeaponState::RELOADING:
    {
        const float reload_delta = float(timestamp - m_last_reload_timestamp) / 1000.0f;
        m_reload_percentage = reload_delta / m_weapon_config.reload_time * 100.0f;

        const bool still_reloading = reload_delta < m_weapon_config.reload_time;
        if(!still_reloading)
        {
            m_reload_finished_sound->Play();
            m_state = WeaponState::IDLE;
        }

        break;
    }
    case WeaponState::OUT_OF_AMMO:
    {
        if(m_weapon_config.auto_reload)
            Reload(timestamp);
        break;
    }
    default:
        break;
    };

    return m_state;
}

WeaponState Weapon::GetWeaponState() const
{
    return m_state;
}

void Weapon::AddAmmunition(int amount)
{
    m_ammunition = std::clamp(m_ammunition + amount, 0, m_weapon_config.magazine_size);
}

int Weapon::AmmunitionLeft() const
{
    return m_ammunition;
}

int Weapon::MagazineSize() const
{
    return m_weapon_config.magazine_size;
}

int Weapon::ReloadPercentage() const
{
    return m_reload_percentage;
}

WeaponSetup Weapon::GetWeaponSetup() const
{
    return m_weapon_setup;
}
