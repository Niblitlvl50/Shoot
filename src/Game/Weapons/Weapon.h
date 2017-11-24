
#pragma once

#include "IWeaponSystem.h"
#include "WeaponConfiguration.h"

#include "MonoPtrFwd.h"
#include "Math/MathFwd.h"

namespace game
{
    class Weapon : public IWeaponSystem
    {
    public:

        Weapon(const WeaponConfiguration& config, mono::EventHandler& eventHandler);

        WeaponFireResult Fire(const math::Vector& position, float direction) override;
        int AmmunitionLeft() const override;
        void Reload() override;

    private:

        const WeaponConfiguration m_weaponConfig;
        mono::EventHandler& m_eventHandler;
        unsigned int m_lastFireTimestamp;
        float m_currentFireRate;
        int m_ammunition;

        mono::ISoundPtr m_fireSound;
        mono::ISoundPtr m_ooa_sound;
    };
}
