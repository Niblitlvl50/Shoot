
#pragma once

#include "IBulletWeapon.h"
#include "WeaponConfiguration.h"

#include "MonoPtrFwd.h"

namespace game
{
    class ThrowableWeapon : public IWeapon
    {
    public:

        ThrowableWeapon(const ThrowableWeaponConfig& config, mono::IEntityManager* entity_manager, mono::SystemContext* system_context);

        WeaponState Fire(const math::Vector& position, float direction, uint32_t timestamp) override;
        void Reload(uint32_t timestamp) override;
        int AmmunitionLeft() const override;
        int MagazineSize() const override;
        WeaponState GetState() const override;


    private:

        const ThrowableWeaponConfig m_config;
        WeaponState m_state;
        int m_ammunition;
    };
}
