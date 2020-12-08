
#include "ThrowableWeapon.h"

using namespace game;

ThrowableWeapon::ThrowableWeapon(const ThrowableWeaponConfig& config, mono::IEntityManager* entity_manager, mono::SystemContext* system_context)
    : m_config(config)
{ }

WeaponState ThrowableWeapon::Fire(const math::Vector& position, float direction, uint32_t timestamp)
{
    return m_state;
}

void ThrowableWeapon::Reload(uint32_t timestamp)
{}

int ThrowableWeapon::AmmunitionLeft() const
{
    return m_ammunition;
}

int ThrowableWeapon::MagazineSize() const
{
    return 10;
}

WeaponState ThrowableWeapon::GetState() const
{
    return m_state;
}
