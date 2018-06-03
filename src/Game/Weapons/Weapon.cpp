
#include "Weapon.h"
#include "Bullet.h"

#include "Math/Vector.h"
#include "Math/MathFunctions.h"
#include "System/System.h"
#include "EventHandler/EventHandler.h"
#include "Audio/ISound.h"
#include "Audio/AudioFactory.h"
#include "Random.h"

#include "Events/SpawnPhysicsEntityEvent.h"
#include "RenderLayers.h"

#include <cmath>
#include <algorithm>

using namespace game;

Weapon::Weapon(const WeaponConfiguration& config, mono::EventHandler& eventHandler)
    : m_weaponConfig(config),
      m_eventHandler(eventHandler),
      m_lastFireTimestamp(0),
      m_currentFireRate(1.0f),
      m_ammunition(config.magazine_size)
{
    m_fireSound = mono::AudioFactory::CreateNullSound();
    m_ooa_sound = mono::AudioFactory::CreateNullSound();
    m_reload_sound = mono::AudioFactory::CreateNullSound();

    if(config.fire_sound)
        m_fireSound = mono::AudioFactory::CreateSound(config.fire_sound, false, false);

    if(config.out_of_ammo_sound)
        m_ooa_sound = mono::AudioFactory::CreateSound(config.out_of_ammo_sound, false, false);
    
    if(config.reload_sound)
        m_reload_sound = mono::AudioFactory::CreateSound(config.reload_sound, false, true);
}

WeaponFireResult Weapon::Fire(const math::Vector& position, float direction)
{
    if(m_reload_sound && m_reload_sound->IsPlaying())
        return WeaponFireResult::RELOADING;

    const float rpsHz = 1.0f / m_weaponConfig.rounds_per_second;
    const unsigned int weapon_delta = rpsHz * 1000.0f;

    const unsigned int now = System::GetMilliseconds();
    const unsigned int delta = now - m_lastFireTimestamp;
    const unsigned int modified_delta = delta * m_currentFireRate;

    if(delta > weapon_delta)
        m_currentFireRate = 1.0f;

    if(modified_delta > weapon_delta)
    {
        m_lastFireTimestamp = now;
        
        if(m_ammunition == 0)
        {
            m_currentFireRate = 1.0f;
            m_ooa_sound->Position(position.x, position.y);
            m_ooa_sound->Play();

            return WeaponFireResult::OUT_OF_AMMO;
        }

        for(int n_bullet = 0; n_bullet < m_weaponConfig.projectiles_per_fire; ++n_bullet)
        {
            direction = direction + math::ToRadians(mono::Random(-m_weaponConfig.bullet_spread, m_weaponConfig.bullet_spread));

            const float force_multiplier = mono::Random(0.8f, 1.2f);

            const math::Vector& unit = math::VectorFromAngle(direction);
            const math::Vector& impulse = unit * m_weaponConfig.bullet_force * force_multiplier;

            auto bullet = std::make_shared<Bullet>(m_weaponConfig.bullet_config);
            bullet->SetPosition(position);
            bullet->SetRotation(direction);
            bullet->GetPhysics().body->ApplyImpulse(impulse, position);

            m_eventHandler.DispatchEvent(game::SpawnPhysicsEntityEvent(bullet, BULLETS, nullptr));
        }

        m_fireSound->Position(position.x, position.y);
        m_fireSound->Play();

        m_currentFireRate *= m_weaponConfig.fire_rate_multiplier;
        m_currentFireRate = std::min(m_currentFireRate, m_weaponConfig.max_fire_rate);

        m_ammunition--;

        return WeaponFireResult::FIRE;
    }

    return WeaponFireResult::NONE;
}

int Weapon::AmmunitionLeft() const
{
    return m_ammunition;
}

int Weapon::MagazineSize() const
{
    return m_weaponConfig.magazine_size;
}

void Weapon::Reload()
{
    m_ammunition = m_weaponConfig.magazine_size;
    m_reload_sound->Play();
}
