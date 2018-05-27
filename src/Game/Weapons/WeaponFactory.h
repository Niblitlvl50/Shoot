
#pragma once

#include "IWeaponFactory.h"
#include "MonoFwd.h"

namespace game
{
    class WeaponFactory : public IWeaponFactory
    {
    public:
        WeaponFactory(mono::EventHandler& event_handler);
        
        std::unique_ptr<IWeaponSystem> CreateWeapon(WeaponType weapon, WeaponFaction faction) override;
        std::unique_ptr<IWeaponSystem> CreateWeapon(
            WeaponType weapon, WeaponFaction faction, mono::ParticlePool* pool) override;

    private:
        mono::EventHandler& m_event_handler;
    };
}
