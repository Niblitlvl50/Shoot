
#include "BulletWeapon.h"
#include "BulletLogic.h"

#include "Math/Vector.h"
#include "Math/MathFunctions.h"

#include "System/Audio.h"
#include "Util/Random.h"

#include "RenderLayers.h"
#include "Effects/MuzzleFlash.h"
#include "Effects/BulletTrailEffect.h"

#include "SystemContext.h"
#include "EntitySystem/IEntityManager.h"
#include "Entity/EntityLogicSystem.h"

#include "Physics/PhysicsSystem.h"
#include "Physics/IBody.h"
#include "Physics/IShape.h"
#include "Particle/ParticleSystem.h"
#include "TransformSystem/TransformSystem.h"

#include "Entity/Component.h"

#include "System/System.h"

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

    m_muzzle_flash = std::make_unique<MuzzleFlash>(m_particle_system, m_entity_manager);
    m_bullet_trail = std::make_unique<BulletTrailEffect>(m_transform_system, m_particle_system, m_entity_manager);
}

Weapon::~Weapon()
{
    for(const auto& pair : m_bullet_id_to_callback)
        m_entity_manager->RemoveReleaseCallback(pair.first, pair.second);
}

WeaponState Weapon::Fire(const math::Vector& position, float direction, uint32_t timestamp)
{
    const math::Vector target = math::VectorFromAngle(direction) * 10.0f;
    return Fire(position, position + target, timestamp);
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

    const float rps_hz = 1.0f / m_weapon_config.rounds_per_second;
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

    const mono::ReleaseCallback release_callback = [this](uint32_t entity_id) {
        m_bullet_trail->RemoveEmitterFromBullet(entity_id);
        m_bullet_id_to_callback.erase(entity_id);
    };

    const math::Vector fire_direction = math::Normalized(target - position);
    const math::Vector perpendicular_fire_direction = math::Perpendicular(fire_direction);

    for(int n_bullet = 0; n_bullet < m_weapon_config.projectiles_per_fire; ++n_bullet)
    {
        const float fire_direction_deviation =
            mono::Random(-m_weapon_config.bullet_spread_degrees, m_weapon_config.bullet_spread_degrees);
        const math::Vector modified_fire_direction = math::RotateAroundZero(fire_direction, math::ToRadians(fire_direction_deviation));

        const float force_multiplier = m_weapon_config.bullet_force_random ? mono::Random(0.8f, 1.2f) : 1.0f;
        const math::Vector& velocity =
            math::Normalized(modified_fire_direction) * m_weapon_config.bullet_force * force_multiplier;

        const float bullet_direction = math::AngleFromVector(modified_fire_direction);
        mono::Entity bullet_entity = m_entity_manager->CreateEntity(m_bullet_config.entity_file.c_str());
        BulletLogic* bullet_logic =
            new BulletLogic(bullet_entity.id, m_owner_id, target, bullet_direction, m_bullet_config, m_collision_config, m_physics_system);

        m_entity_manager->AddComponent(bullet_entity.id, BEHAVIOUR_COMPONENT);
        m_logic_system->AddLogic(bullet_entity.id, bullet_logic);

        const math::Vector perp_offset =
            perpendicular_fire_direction * mono::Random(-m_weapon_config.bullet_offset, m_weapon_config.bullet_offset);
        const math::Vector fire_position = position + perp_offset;

        const math::Matrix& transform = (m_bullet_config.bullet_want_direction) ?
            math::CreateMatrixWithPositionRotation(fire_position, math::AngleFromVector(modified_fire_direction)) :
            math::CreateMatrixWithPosition(fire_position);

        m_transform_system->SetTransform(bullet_entity.id, transform);
        m_transform_system->SetTransformState(bullet_entity.id, mono::TransformState::CLIENT);

        mono::IBody* body = m_physics_system->GetBody(bullet_entity.id);

        //const float velocity_length = math::Length(velocity);
        //const float body_mass = body->GetMass();
        //System::Log("[%u]%s m: %.2f velocity: %.2f", bullet_entity.id, m_weapon_config.name.c_str(), body_mass, velocity_length);

        body->AddCollisionHandler(bullet_logic);
        body->SetNoDamping();
        body->SetVelocity(velocity);

        std::vector<mono::IShape*> shapes = m_physics_system->GetShapesAttachedToBody(bullet_entity.id);
        for(mono::IShape* shape : shapes)
            shape->SetCollisionFilter(m_collision_config.collision_category, m_collision_config.collision_mask);

        m_bullet_trail->AttachEmitterToBullet(bullet_entity.id);

        const uint32_t callback_id = m_entity_manager->AddReleaseCallback(bullet_entity.id, release_callback);
        m_bullet_id_to_callback[bullet_entity.id] = callback_id;
    }

    m_fire_sound->Play();

    m_current_fire_rate *= m_weapon_config.fire_rate_multiplier;
    m_current_fire_rate = std::min(m_current_fire_rate, m_weapon_config.max_fire_rate);

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
