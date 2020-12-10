
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

#include "Component.h"

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
    if(m_state == WeaponState::RELOADING && reload_delta < (m_config.reload_time * 1000.0f))
        return m_state;

    if(m_ammunition == 0)
    {
        m_state = WeaponState::OUT_OF_AMMO;
        return m_state;
    }

    m_last_fire_timestamp = timestamp;

    math::Vector diff = target - position;
    const float target_length = math::Length(diff);
    if(target_length > m_config.max_distance)
        diff = math::Normalized(diff) * m_config.max_distance;

    for(int n_bullet = 0; n_bullet < m_config.projectiles_per_fire; ++n_bullet)
    {
        //const float bullet_direction = direction + math::ToRadians(mono::Random(-m_config.bullet_spread_degrees, m_weapon_config.bullet_spread_degrees));
        mono::Entity thrown_entity = m_entity_manager->CreateEntity(m_config.thrown_entity);

        ThrowableLogic* throwable_logic = new ThrowableLogic(
            thrown_entity.id,
            m_config.spawned_entity,
            position,
            position + diff,
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

void ThrowableWeapon::Reload(uint32_t timestamp)
{
    m_ammunition = m_config.magazine_size;
    m_last_reload_timestamp = timestamp;
    m_state = WeaponState::RELOADING;
}

int ThrowableWeapon::AmmunitionLeft() const
{
    return m_ammunition;
}

int ThrowableWeapon::MagazineSize() const
{
    return m_config.magazine_size;
}

WeaponState ThrowableWeapon::GetState() const
{
    return m_state;
}
