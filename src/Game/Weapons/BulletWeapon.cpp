
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

#include "Component.h"

//#include "System/System.h"

#include <cmath>
#include <algorithm>
#include <cassert>

using namespace game;

Weapon::Weapon(const WeaponConfiguration& config, mono::IEntityManager* entity_manager, mono::SystemContext* system_context)
    : m_weapon_config(config)
    , m_entity_manager(entity_manager)
    , m_last_fire_timestamp(0)
    , m_last_reload_timestamp(0)
    , m_current_fire_rate(1.0f)
    , m_ammunition(config.magazine_size)
    , m_state(WeaponState::IDLE)
{
    m_fire_sound = audio::CreateNullSound();
    m_ooa_sound = audio::CreateNullSound();
    m_reload_sound = audio::CreateNullSound();

    if(config.fire_sound)
        m_fire_sound = audio::CreateSound(config.fire_sound, audio::SoundPlayback::ONCE);

    if(config.out_of_ammo_sound)
        m_ooa_sound = audio::CreateSound(config.out_of_ammo_sound, audio::SoundPlayback::ONCE);
    
    if(config.reload_sound)
        m_reload_sound = audio::CreateSound(config.reload_sound, audio::SoundPlayback::ONCE);

    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_particle_system = system_context->GetSystem<mono::ParticleSystem>();
    m_logic_system = system_context->GetSystem<EntityLogicSystem>();

    m_muzzle_flash = std::make_unique<MuzzleFlash>(m_particle_system, m_entity_manager);
    m_bullet_trail = std::make_unique<BulletTrailEffect>(m_transform_system, m_particle_system, entity_manager);
}

Weapon::~Weapon()
{
    for(const auto& pair : m_bullet_id_to_callback)
        m_entity_manager->RemoveReleaseCallback(pair.first, pair.second);
}

WeaponState Weapon::Fire(const math::Vector& position, float direction, uint32_t timestamp)
{
    if(m_state == WeaponState::RELOADING)
        return m_state;

    if(m_ammunition == 0)
    {
        m_current_fire_rate = 1.0f;
        m_ooa_sound->Play();

        m_state = WeaponState::OUT_OF_AMMO;
        return m_state;
    }

    const float rps_hz = 1.0f / m_weapon_config.rounds_per_second;
    const uint32_t weapon_delta = rps_hz * 1000.0f;

    const uint32_t delta = timestamp - m_last_fire_timestamp;
    const uint32_t modified_delta = delta * m_current_fire_rate;

    if(modified_delta < weapon_delta)
        return m_state;

    if(delta > weapon_delta)
        m_current_fire_rate = 1.0f;

    m_last_fire_timestamp = timestamp;

    for(int n_bullet = 0; n_bullet < m_weapon_config.projectiles_per_fire; ++n_bullet)
    {
        const float bullet_direction = direction + math::ToRadians(mono::Random(-m_weapon_config.bullet_spread_degrees, m_weapon_config.bullet_spread_degrees));

        float force_multiplier = 1.0f;
        if(m_weapon_config.bullet_force_random)
            force_multiplier = mono::Random(0.8f, 1.2f);
        
        const math::Vector& unit = math::Normalized(math::VectorFromAngle(bullet_direction));
        const math::Vector& impulse = unit * m_weapon_config.bullet_force * force_multiplier;

        mono::Entity bullet_entity = m_entity_manager->CreateEntity(m_weapon_config.bullet_config.entity_file);
        BulletLogic* bullet_logic = new BulletLogic(bullet_entity.id, m_weapon_config.owner_id, m_weapon_config.bullet_config, m_physics_system);

        m_entity_manager->AddComponent(bullet_entity.id, BEHAVIOUR_COMPONENT);
        m_logic_system->AddLogic(bullet_entity.id, bullet_logic);

        math::Matrix& transform = m_transform_system->GetTransform(bullet_entity.id);
        transform = math::CreateMatrixWithPositionRotation(position, bullet_direction);
        m_transform_system->SetTransformState(bullet_entity.id, mono::TransformState::CLIENT);

        mono::IBody* body = m_physics_system->GetBody(bullet_entity.id);
        //body->SetPosition(position);
        //body->SetAngle(bullet_direction);
        body->SetVelocity(impulse);
        body->SetNoDamping();
        body->AddCollisionHandler(bullet_logic);

        std::vector<mono::IShape*> shapes = m_physics_system->GetShapesAttachedToBody(bullet_entity.id);
        for(mono::IShape* shape : shapes)
            shape->SetCollisionFilter(m_weapon_config.bullet_config.collision_category, m_weapon_config.bullet_config.collision_mask);

        m_bullet_trail->AttachEmitterToBullet(bullet_entity.id);

        const ReleaseCallback release_callback = [this](uint32_t entity_id) {
            m_bullet_trail->RemoveEmitterFromBullet(entity_id);
            m_bullet_id_to_callback.erase(entity_id);
        };
        const uint32_t callback_id = m_entity_manager->AddReleaseCallback(bullet_entity.id, release_callback);
        m_bullet_id_to_callback[bullet_entity.id] = callback_id;
    }

    m_muzzle_flash->EmittAt(position, direction);

    m_fire_sound->Play();

    m_current_fire_rate *= m_weapon_config.fire_rate_multiplier;
    m_current_fire_rate = std::min(m_current_fire_rate, m_weapon_config.max_fire_rate);

    m_ammunition--;

    m_state = WeaponState::FIRE;
    return m_state;
}

WeaponState Weapon::Fire(const math::Vector& position, const math::Vector& target, uint32_t timestamp)
{
    assert(false); // Use the other fire for now...
    return m_state;
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
        const uint32_t reload_delta = timestamp - m_last_reload_timestamp;
        m_reload_percentage = float(reload_delta) / float(m_weapon_config.reload_time_ms) * 100.0f;

        const bool still_reloading = reload_delta < m_weapon_config.reload_time_ms;
        if(!still_reloading)
            m_state = WeaponState::IDLE;

        break;
    }
    default:
        break;
    };

    return m_state;
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
