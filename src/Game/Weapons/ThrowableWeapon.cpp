
#include "ThrowableWeapon.h"
#include "ThrowableLogic.h"

#include "EntitySystem/IEntityManager.h"
#include "Entity/EntityLogicSystem.h"
#include "Math/MathFunctions.h"
#include "Particle/ParticleSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Random.h"

#include "Entity/Component.h"

#include <cassert>

using namespace game;

ThrowableWeapon::ThrowableWeapon(const ThrowableWeaponConfig& config, mono::IEntityManager* entity_manager, mono::SystemContext* system_context)
    : m_config(config)
    , m_entity_manager(entity_manager)
    , m_last_fire_timestamp(0)
    , m_last_reload_timestamp(0)
    , m_ammunition(config.magazine_size)
    , m_state(WeaponState::IDLE)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_particle_system = system_context->GetSystem<mono::ParticleSystem>();
    m_logic_system = system_context->GetSystem<EntityLogicSystem>();
}

WeaponState ThrowableWeapon::Fire(const math::Vector& position, float direction, uint32_t timestamp)
{
    assert(false); // Use the other fire for now...
    return m_state;
}

WeaponState ThrowableWeapon::Fire(const math::Vector& position, const math::Vector& target, uint32_t timestamp)
{
    const uint32_t reload_delta = timestamp - m_last_reload_timestamp;
    if(m_state == WeaponState::RELOADING && reload_delta < m_config.reload_time_ms)
        return m_state;

    if(m_ammunition == 0)
    {
        m_state = WeaponState::OUT_OF_AMMO;
        return m_state;
    }

    m_last_fire_timestamp = timestamp;

    math::Vector diff = position - target;
    const float target_length = math::Length(diff);
    if(target_length > m_config.max_distance)
        diff = math::Normalized(diff) * m_config.max_distance;

    for(int n_bullet = 0; n_bullet < m_config.projectiles_per_fire; ++n_bullet)
    {
        math::Vector accuracy_diff;
        if(m_config.target_accuracy != 0.0f)
        {
            const float radians = mono::Random(-math::PI(), math::PI());
            accuracy_diff = math::VectorFromAngle(radians) * mono::Random(0.0f, m_config.target_accuracy);
        }

        mono::Entity thrown_entity = m_entity_manager->CreateEntity(m_config.thrown_entity);

        ThrowableLogic* throwable_logic = new ThrowableLogic(
            thrown_entity.id,
            m_config.spawned_entity,
            position,
            position + diff + accuracy_diff,
            m_transform_system,
            m_sprite_system,
            m_particle_system,
            m_entity_manager);

        m_entity_manager->AddComponent(thrown_entity.id, BEHAVIOUR_COMPONENT);
        m_logic_system->AddLogic(thrown_entity.id, throwable_logic);
    }

    m_ammunition--;
    m_state = WeaponState::FIRE;
    return m_state;
}

void ThrowableWeapon::StopFire(uint32_t timestamp)
{
    
}

void ThrowableWeapon::Reload(uint32_t timestamp)
{
    m_ammunition = m_config.magazine_size;
    m_last_reload_timestamp = timestamp;
    m_state = WeaponState::RELOADING;
}

WeaponState ThrowableWeapon::UpdateWeaponState(uint32_t timestamp)
{
    switch(m_state)
    {
    case WeaponState::RELOADING:
    {
        const uint32_t reload_delta = timestamp - m_last_reload_timestamp;
        m_reload_percentage = float(reload_delta) / float(m_config.reload_time_ms) * 100.0f;

        const bool still_reloading = reload_delta < m_config.reload_time_ms;
        if(!still_reloading)
            m_state = WeaponState::IDLE;

        break;
    }
    default:
        break;
    };

    return m_state;
}

void ThrowableWeapon::AddAmmunition(int amount)
{
    m_ammunition = std::clamp(m_ammunition + amount, 0, m_config.magazine_size);
}

int ThrowableWeapon::AmmunitionLeft() const
{
    return m_ammunition;
}

int ThrowableWeapon::MagazineSize() const
{
    return m_config.magazine_size;
}

int ThrowableWeapon::ReloadPercentage() const
{
    return m_reload_percentage;
}
